//
// Created by liu on 2016/12/3.
//

#include "SRLBiLSTMModel.h"

using namespace std;

unsigned PRETRAINED_DIM = 200u;

unsigned SRLBiLSTMModel::loadPreTrainedWordEmb(std::string path) {
  preWordEmd[kUNK] = vector<float>(PRETRAINED_DIM, 0);
  ifstream in(path);
  string line;
  getline(in, line); // skip first line
  vector<float> v(PRETRAINED_DIM, 0);
  string word;
  unsigned counter = 0;
  while (getline(in, line)) {
    counter++;
    istringstream iss(line);
    iss >> word;
    for (unsigned i = 0; i < PRETRAINED_DIM; ++i)
      iss >> v[i];
    unsigned id = lookUpTable.getOrPushString(SRLLookUpTable::word, word);
    preWordEmd[id] = v;
  }
  return counter;
}

SRLBiLSTMModel::SRLBiLSTMModel(
  SRLBiLSTMParameter _parameter,
  string modelPath,
  string preTrainedWordEmbPath,
  string indexDirPath):
    kUNK(0),
    parameter(_parameter),
    modelPath(modelPath),
    indexDirPath(indexDirPath),
    preTrainedWordEmbPath(preTrainedWordEmbPath)
{
}

// note 注意一下C++ 构造函数调用，函数中直接调用时新生成一个变量而已，不会赋给this
SRLBiLSTMModel::SRLBiLSTMModel(
        string modelPath,
        string preTrainedWordEmbPath,
        string indexDirPath
):SRLBiLSTMModel(SRLBiLSTMParameter(), modelPath, preTrainedWordEmbPath, indexDirPath) { }


inline void SRLBiLSTMModelTrainer::loadData(string filePath, vector<SRLSample> & sampleList, bool changeTable) {
  ifstream in(filePath);
  TrainStn trainStn;
  trainStn.read(in);
  in.close();
  if (changeTable) {
    // load table
    trainStn.registerSubInstanceFeatureVector("label", lookUpTable.getTable(SRLLookUpTable::Type::label));
    trainStn.registerSubInstanceFeature("word", lookUpTable.getTable(SRLLookUpTable::Type::word));
    trainStn.registerSubInstanceFeature("pos", lookUpTable.getTable(SRLLookUpTable::Type::pos));
    trainStn.registerSubInstanceFeature("position", lookUpTable.getTable(SRLLookUpTable::Type::position));
    trainStn.registerSubInstanceFeature("semtag", lookUpTable.getTable(SRLLookUpTable::Type::semtag));
  }
  trainStn.ExtractFeatures(sampleList, lookUpTable);
}

bool SRLBiLSTMModelTrainer::train(std::string trainDataPath, std::string devDataPath, int max_iter) {
  vector<SRLSample> trainSampleList, devSampleList;
  loadData(trainDataPath, trainSampleList, true); // set table
  lookUpTable.freeze();
  loadData(devDataPath, devSampleList, false); // todo 检查是否冻结
  preWordEmd_num = (loadPreTrainedWordEmb(preTrainedWordEmbPath));
  reNewNumber();
  srl = (loadModel(modelPath));

  // end init todo mv to constructor
  SimpleSGDTrainer sgd(&model);
  sgd.eta_decay = 0.08;
  cerr << "Training started." << "\n";
  Performance best_dev_perf;
  bool global_use_drop_out = parameter.use_dropout;
  unsigned trainSetSize = (unsigned)trainSampleList.size();
  vector<unsigned> order(trainSetSize);
  unsigned status_every_i_iterations = min(100u, trainSetSize);
  unsigned si = trainSetSize;
  bool first = true;
  for (unsigned i = 0; i < trainSampleList.size(); ++i)
    order[i] = i;

  double llh = 0;unsigned smps = 0;double tot_seen = 0;
  for (int turn_iter = 0; turn_iter < max_iter; turn_iter++) {
    Performance perf;
    for (unsigned sii = 0; sii < status_every_i_iterations; ++sii) {
      if (si >= trainSetSize) {
        si = 0;
        if (first) {
          first = false;
        } else {
          sgd.update_epoch();
        }
        cerr << "**SHUFFLE\n";
        random_shuffle(order.begin(), order.end());
      }
      ComputationGraph hg;
      SRLSample &sample = trainSampleList[order[si]];
      vector<Prediction> results;
      Expression err = srl.nn_srlabeler(&hg, sample, lookUpTable, perf, results);
      double lp = as_scalar(hg.incremental_forward(err));
      assert (lp >= 0.0);
      hg.backward(err);
      sgd.update(1.0);
      llh += lp;
      smps += sample.size();
      tot_seen += 1;
      ++si;
    }
    sgd.status();
    cerr << "update #" << turn_iter << " (epoch " << (tot_seen / trainSampleList.size())
         << ")\tllh: " << llh << " ppl: " << exp(llh / smps)
         << " P: " << perf.precision
         << " R: " << perf.recall
         << " F: " << perf.fscore << endl;

    smps = 0;llh = 0;
    if (turn_iter % 100 == 0) {
      unsigned dev_size = (unsigned)devSampleList.size();
      auto t_start = std::chrono::high_resolution_clock::now();
      Performance dev_perf;
      parameter.use_dropout = false;
      for (unsigned j = 0; j < devSampleList.size(); ++j) {
        ComputationGraph hg;
        vector<Prediction> results;
        srl.nn_srlabeler(&hg, devSampleList[j], lookUpTable, dev_perf, results);
      }
      auto t_end = std::chrono::high_resolution_clock::now();
      cerr << "  **dev (iter=" << turn_iter << " epoch=" << (tot_seen / trainSampleList.size())
           << ") P: " << dev_perf.precision
           << "  R: " << dev_perf.recall
           << "  F: " << dev_perf.fscore
           << "\t[" << dev_size << " samples in "
           << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " ms"
           << endl;
      if (dev_perf.fscore > best_dev_perf.fscore) {
        best_dev_perf = dev_perf;
        ofstream out(modelPath);
        boost::archive::text_oarchive oa(out);
        oa << model;
        out.close();
      }
      parameter.use_dropout = global_use_drop_out;
    }
  }
  return true;
}

Expression SRLBuilder::nn_srlabeler(ComputationGraph* hg,
                        SRLSample & srlSample,
                        SRLLookUpTable& lookUpTable,
                        Performance& perf, vector<Prediction> & results) {
  // vector<Prediction> results; out with parameter
  vector<Expression> log_probs;
  up_gen_path_lstm.new_graph(*hg);
  dw_gen_path_lstm.new_graph(*hg);
  up_sem_path_lstm.new_graph(*hg);
  dw_sem_path_lstm.new_graph(*hg);
  fw_sent_lstm.new_graph(*hg);
  bw_sent_lstm.new_graph(*hg);
  // up_word_path_lstm.new_graph(*hg);
  // dw_word_path_lstm.new_graph(*hg);

  if (lstmParameter.use_dropout) {
    up_gen_path_lstm.set_dropout(lstmParameter.dropout_rate);
    dw_gen_path_lstm.set_dropout(lstmParameter.dropout_rate);
    up_sem_path_lstm.set_dropout(lstmParameter.dropout_rate);
    dw_sem_path_lstm.set_dropout(lstmParameter.dropout_rate);
    fw_sent_lstm.set_dropout(lstmParameter.dropout_rate);
    bw_sent_lstm.set_dropout(lstmParameter.dropout_rate);
  } else {
    up_gen_path_lstm.disable_dropout();
    dw_gen_path_lstm.disable_dropout();
    up_sem_path_lstm.disable_dropout();
    dw_sem_path_lstm.disable_dropout();
    fw_sent_lstm.disable_dropout();
    bw_sent_lstm.disable_dropout();
  }

  Expression pbias = parameter(*hg, p_pbias);
  Expression A = parameter(*hg, p_Arg);
  Expression P = parameter(*hg, p_Pred);
  Expression PT = parameter(*hg, p_Position);
  Expression GP; GP = parameter(*hg, p_GenPath);
  Expression SP; SP = parameter(*hg, p_SemPath);
  Expression StP = parameter(*hg, p_SentPred); // @TEST
  Expression StA = parameter(*hg, p_SentArg);
  // Expression WP; if (USE_WORDPATH) WP = parameter(*hg, p_WordPath);
  Expression p2l = parameter(*hg, p_p2l);
  Expression lbias = parameter(*hg, p_lbias);

  Expression w2lstm = parameter(*hg, p_w2lstm);
  Expression p2lstm = parameter(*hg, p_p2lstm);
  Expression ib = parameter(*hg, p_ibias);

  Expression t2lstm;
//      if (p_t2lstm) t2lstm = parameter(*hg, p_t2lstm);

  // precision = tp / (#parg)
  // recall = tp / (#arg)
  // double tp = 0, n_arg = 0, n_parg = 0;

  // get sentence representations
  Expression sent_start = parameter(*hg, p_start_of_sent);
  Expression sent_end = parameter(*hg, p_end_of_sent);
  // save all hidden representations
  fw_sent_lstm.start_new_sequence(); fw_sent_lstm.add_input(sent_start);
  bw_sent_lstm.start_new_sequence(); bw_sent_lstm.add_input(sent_end);
  unsigned snt_len = srlSample.size();
  vector<Expression> fw_sent_exprs(snt_len);
  vector<Expression> bw_sent_exprs(snt_len);
  for (unsigned i = 0; i < snt_len; ++i) {
    {
      Expression wf = lookup(*hg, p_word, srlSample.getInstanceFeature<unsigned>(i, "word"));
      Expression pf = lookup(*hg, p_pos, srlSample.getInstanceFeature<unsigned>(i, "pos"));
      Expression i_i = affine_transform({ib, w2lstm, wf, p2lstm, pf});
//          没有使用拼接的办法，只是把pretrain的word-embedding作了初始化
//          if (p_t2lstm && pretrained.count(snt.words[i])) {
//            Expression tf = const_lookup(*hg, p_preword, snt.words[i]);
//            i_i = affine_transform({i_i, t2lstm, tf});
//          }
      fw_sent_exprs[i] = fw_sent_lstm.add_input(rectify(i_i));
    }
    {
      Expression wb = lookup(*hg, p_word, srlSample.getInstanceFeature<unsigned>(snt_len-i-1, "word"));
      Expression pb = lookup(*hg, p_pos, srlSample.getInstanceFeature<unsigned>(snt_len-i-1, "pos"));
      Expression i_i = affine_transform({ib, w2lstm, wb, p2lstm, pb});
//          if (p_t2lstm && pretrained.count(snt.words[snt_len-i-1])) {
//            Expression tb = const_lookup(*hg, p_preword, snt.words[snt_len-i-1]);
//            i_i = affine_transform({i_i, t2lstm, tb});
//          }
      bw_sent_exprs[snt_len-i-1] = bw_sent_lstm.add_input(rectify(i_i));
    }
  }
  fw_sent_lstm.add_input(sent_end);
  bw_sent_lstm.add_input(sent_start);
  int pred_id = srlSample.getFeature<int>("predicate");
  vector<Expression> fb_sent_pred_expr = {fw_sent_exprs[pred_id],
                                          bw_sent_exprs[pred_id]};
  Expression sent_pred_expr = concatenate(fb_sent_pred_expr);
  for (unsigned i = 0; i < snt_len; ++i) {

    Prediction prediction;

    up_gen_path_lstm.start_new_sequence();
    dw_gen_path_lstm.start_new_sequence();
    up_sem_path_lstm.start_new_sequence();
    dw_sem_path_lstm.start_new_sequence();

    Expression pt_expr = lookup(*hg, p_positions, srlSample.getInstanceFeature<unsigned>(i, "position"));

    const vector<unsigned>& up_gen_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "arguementGenPathIndex");
    const vector<unsigned>& dw_gen_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "predicateGenPathIndex");
    const vector<unsigned>& up_word_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "arguementPathIndex");
    const vector<unsigned>& dw_word_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "predicatePathIndex");
    const vector<unsigned>& up_sem_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "arguementSemtagPathIndex");
    const vector<unsigned>& dw_sem_path = srlSample.getInstanceFeature<vector<unsigned>>(i, "predicateSemtagPathIndex");
    if (lstmParameter.use_genpath) {
      unsigned up_siz = (unsigned)up_gen_path.size();
      for (unsigned i = 0; i < up_siz; ++i) {
        // cerr << "t = " << t << endl;
        unsigned t = up_gen_path[i];
        unsigned w = up_word_path[i];
        Expression gen_t = lookup(*hg, p_pos, t);
        Expression gen_w = lookup(*hg, p_word, w);
        vector<Expression> gen_unit_expr = {gen_w, gen_t};
        up_gen_path_lstm.add_input(concatenate(gen_unit_expr));
      }
      unsigned dw_siz = (unsigned)dw_gen_path.size();
      assert (dw_siz > 0);
      for (unsigned i = 0; i < dw_siz; ++i) {
        unsigned t = dw_gen_path[dw_siz-i-1];
        unsigned w = dw_word_path[dw_siz-i-1];
        Expression gen_t = lookup(*hg, p_pos, t);
        Expression gen_w = lookup(*hg, p_word, w);
        vector<Expression> gen_unit_expr = {gen_w, gen_t};
        dw_gen_path_lstm.add_input(concatenate(gen_unit_expr));
      }
    }

    // hg->incremental_forward();
    if (lstmParameter.use_sempath) {
      for (auto& t : up_sem_path) {
        Expression sem_t = lookup(*hg, p_sem_tags, t);
        up_sem_path_lstm.add_input(sem_t);
      }
      unsigned dw_siz = (unsigned)dw_sem_path.size();
      assert (dw_siz > 0);
      for (unsigned i = 0; i < dw_siz; ++i) {
        Expression sem_t = lookup(*hg, p_sem_tags, dw_sem_path[dw_siz-i-1]);
        dw_sem_path_lstm.add_input(sem_t);
      }
    }
    vector<Expression> sent_arg_expr = {fw_sent_exprs[i], bw_sent_exprs[i]};
    Expression hidden = affine_transform({pbias, PT, pt_expr});
    if (lstmParameter.use_sent) {
      hidden = affine_transform({hidden, StA, concatenate(sent_arg_expr), StP, sent_pred_expr});
    }
    if (lstmParameter.use_genpath) {
      vector<Expression> gen_path_expr{up_gen_path_lstm.back(), dw_gen_path_lstm.back()};
      hidden = affine_transform({hidden, GP, concatenate(gen_path_expr)});
    }
    if (lstmParameter.use_sempath) {
      vector<Expression> sem_path_expr{up_sem_path_lstm.back(), dw_sem_path_lstm.back()};
      hidden = affine_transform({hidden, SP, concatenate(sem_path_expr)});
    }
    Expression nl_hidden;
    if (lstmParameter.activate == "tanh") nl_hidden = tanh(hidden);
    else if (lstmParameter.activate == "cube") nl_hidden = cube(hidden);
    else nl_hidden = rectify(hidden);
    if (lstmParameter.use_dropout) nl_hidden = dropout(nl_hidden, lstmParameter.dropout_rate);

    Expression output = affine_transform({lbias, p2l, nl_hidden});
    Expression adiste = softmax(output);

    vector<float> probs = as_vector(hg->incremental_forward(adiste));
    for (unsigned i = 0; i < probs.size(); ++i) {
      prediction.push_back(make_pair<string, double>(lookUpTable.getStringByIndex(SRLLookUpTable::label,i), probs[i]));
    }
    sort(prediction.begin(), prediction.end(), cmp_outcome());

    adiste = log(adiste);

    // hg->PrintGraphviz();

    // evaluate 概率最大标签概率 max_prob 和 概率最大标签 max_idx
    vector<float> adist = as_vector(hg->incremental_forward(adiste));
    double max_prob = adist[0];
    unsigned max_idx = 0;
    for (unsigned i = 1; i < adist.size(); ++i) {
      if (adist[i] > max_prob) {
        max_prob = adist[i];
        max_idx = i;
      }
    }
    unsigned label = srlSample.getInstanceFeature<unsigned>(i, "label");
    if (lookUpTable.getStringByIndex(SRLLookUpTable::Type::label, label) != "_") perf.n_arg += 1;
    if (lookUpTable.getStringByIndex(SRLLookUpTable::Type::label, max_idx) != "_") {
      perf.n_parg += 1;
      if (max_idx == label)
        perf.tp += 1;
    }
    if (up_gen_path.size() == 1 && dw_gen_path.size() == 1) {// predicate
      /*
      perf.tp += 1;
      perf.n_arg += 1;
      perf.n_parg += 1;
      */
    }

    // end evaluate

    log_probs.push_back(pick(adiste, label));
    // results.push_back(max_idx);
    results.push_back(prediction);
  }
  perf.precision = perf.tp / perf.n_parg;
  perf.recall = perf.tp / perf.n_arg;
  perf.fscore = 2 * perf.precision * perf.recall / (perf.precision + perf.recall);

  Expression tot_neglogprob = -sum(log_probs);
  assert(tot_neglogprob.pg != nullptr);

  return tot_neglogprob;
}
