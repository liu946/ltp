//
// Created by liu on 2016/12/3.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLBILSTMPREDICTOR_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLBILSTMPREDICTOR_H

#include <cstdlib>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <set>
#include <map>

#include <unordered_map>
#include <unordered_set>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "dynet/nodes.h"
#include "dynet/dynet.h"
#include "dynet/training.h"
#include "dynet/timing.h"
#include "dynet/rnn.h"
#include "dynet/gru.h"
#include "dynet/lstm.h"
#include "dynet/dict.h"
#include "dynet/expr.h"
#include <dynet/model.h>

#include "strutils.h"
#include "trainStn.h"
#include "SRLLookUpTable.h"

using namespace std;
using namespace dynet;
using namespace dynet::expr;

//unsigned LAYERS = 2;
//unsigned INPUT_DIM = 50;
//unsigned HIDDEN_DIM = 100;
//unsigned POS_DIM = 20;

//unsigned LSTM_INPUT_DIM = 20;
//unsigned LSTM_HIDDEN_DIM = 100;
//unsigned GENTAG_DIM = 20;
//unsigned SEMTAG_DIM = 20;
//unsigned POSITION_DIM = 5;
//
//bool USE_SENT = false;
//bool USE_GENPATH = false;
//bool USE_SEMPATH = false;
//// bool USE_WORDPATH = false;
//
//bool USE_DROPOUT = true;
//float DROPOUT_RATE = 0.5;
//
//unsigned VOCAB_SIZE = 0;
//unsigned POS_SIZE = 0;
//unsigned GENTAG_SIZE = 0;
//unsigned SEMTAG_SIZE = 0;
//unsigned POSITION_SIZE = 0;
//unsigned LABEL_SIZE = 0;
//
//unsigned MAX_ITERATION = 50000; // #batches
//string ACTIVATE = "rectify";
//string LANG = "EN";
//string NIL = "NULL";

// todo remove-end

typedef vector<pair<string, double>> Prediction;

struct SRLBiLSTMParameter {
  unsigned layers,
      input_dim,
      pos_dim,
      semtag_dim,
      hidden_dim,
      position_dim,
      lstm_hidden_dim,
      lstm_input_dim,
      pretrained_dim;
  bool use_dropout,
      use_genpath,
      use_sempath,
      use_sent;
  float dropout_rate;
  string activate;
  SRLBiLSTMParameter(
    unsigned _layers = 1,
    unsigned _input_dim = 200,
    unsigned _pos_dim = 25,
    unsigned _semtag_dim = 50,
    unsigned _hidden_dim = 200,
    unsigned _position_dim = 5,
    unsigned _lstm_input_dim = 100,
    unsigned _lstm_hidden_dim = 100
  ) {
    layers = _layers;
    input_dim = _input_dim;
    pos_dim = _pos_dim;
    semtag_dim = _semtag_dim;
    hidden_dim = _hidden_dim;
    position_dim = _position_dim;
    lstm_hidden_dim = _lstm_hidden_dim;
    lstm_input_dim = _lstm_input_dim;
    pretrained_dim = 200;
    use_dropout = false;
    dropout_rate = 0.5;
    activate = "rectify";
    use_genpath = true;
    use_sempath = true;
    use_sent = true;
  }
};
struct TrainNum {
  unsigned words_num;
  unsigned pos_num;
  unsigned semtag_num;
  unsigned position_num;
  unsigned label_num;
public:
  TrainNum(unsigned int words_num, unsigned int pos_num, unsigned int semtag_num, unsigned int position_num, unsigned label_num)
          : words_num(words_num), pos_num(pos_num), semtag_num(semtag_num), position_num(position_num), label_num(label_num) {}
  TrainNum() {}
};

struct Performance {
  double precision = 0;
  double recall = 0;
  double fscore = 0;
  double tp = 0;
  double n_arg = 0;
  double n_parg = 0;
};


struct SRLBuilder {
  SRLBiLSTMParameter lstmParameter;
  LookupParameter p_word; // word embeddings
  LookupParameter p_preword; // pretrained word embeddings (not updated)
  LookupParameter p_pos; // pos tag embeddings
  // LookupParameter p_gen_tags; // generic path tag embeddings
  LookupParameter p_sem_tags; // semantic relation tag embeddings
  LookupParameter p_positions; // positions = before/after
  Parameter p_Arg;    // Argument representation to hidden layer
  Parameter p_Pred;   // Predicate representation to hidden layer
  Parameter p_GenPath;   // Generic path representation to hidden layer
  Parameter p_SemPath;   // Semantic path representation to hidden layer
  // Parameter p_WordPath;  // Generic path of words to hidden layer
  Parameter p_Position;
  Parameter p_pbias;  // hidden bias
  Parameter p_p2l;    // label
  Parameter p_lbias;  // label bias

  LSTMBuilder up_gen_path_lstm;
  LSTMBuilder dw_gen_path_lstm;
  LSTMBuilder up_sem_path_lstm;
  LSTMBuilder dw_sem_path_lstm;

  // sentence representation
  Parameter p_start_of_sent;
  Parameter p_end_of_sent;
  LSTMBuilder fw_sent_lstm;
  LSTMBuilder bw_sent_lstm;
  Parameter p_w2lstm;
  Parameter p_p2lstm;
  Parameter p_t2lstm;
  Parameter p_ibias;

  Parameter p_SentPred;
  Parameter p_SentArg;
  // LSTMBuilder up_word_path_lstm;
  // LSTMBuilder dw_word_path_lstm;

  struct cmp_outcome {
    bool operator()(const pair<string, double>& lpr,
                    const pair<string, double>& rpr) const {
      return lpr.second > rpr.second;
    }
  };

  // 增加一个默认构造函数，避免使用时必须的初始化。
  SRLBuilder() {}

  SRLBuilder(Model *model,
             const unordered_map<unsigned, vector<float>> &pretrained,
             SRLBiLSTMParameter lstmParameter,
             const TrainNum & train_num
          ):
          p_word(model->add_lookup_parameters(train_num.words_num, {lstmParameter.input_dim})),
          p_pos(model->add_lookup_parameters(train_num.pos_num, {lstmParameter.pos_dim})),
          p_positions(model->add_lookup_parameters(train_num.position_num, {lstmParameter.position_dim})),
          p_pbias(model->add_parameters({lstmParameter.hidden_dim})),
          p_p2l(model->add_parameters({train_num.label_num, lstmParameter.hidden_dim})),
          p_lbias(model->add_parameters({train_num.label_num})),
          up_gen_path_lstm(lstmParameter.layers, lstmParameter.pos_dim + lstmParameter.input_dim, lstmParameter.lstm_hidden_dim, model),
          dw_gen_path_lstm(lstmParameter.layers, lstmParameter.pos_dim + lstmParameter.input_dim, lstmParameter.lstm_hidden_dim, model),
          up_sem_path_lstm(lstmParameter.layers, lstmParameter.semtag_dim, lstmParameter.lstm_hidden_dim, model),
          dw_sem_path_lstm(lstmParameter.layers, lstmParameter.semtag_dim, lstmParameter.lstm_hidden_dim, model),
          p_start_of_sent(model->add_parameters({lstmParameter.lstm_hidden_dim})),
          p_end_of_sent(model->add_parameters({lstmParameter.lstm_hidden_dim})),
          fw_sent_lstm(lstmParameter.layers, lstmParameter.lstm_input_dim, lstmParameter.lstm_hidden_dim, model),
          bw_sent_lstm(lstmParameter.layers, lstmParameter.lstm_input_dim, lstmParameter.lstm_hidden_dim, model),
          p_w2lstm(model->add_parameters({lstmParameter.lstm_hidden_dim, lstmParameter.input_dim})),
          p_p2lstm(model->add_parameters({lstmParameter.lstm_hidden_dim, lstmParameter.pos_dim})),
          p_ibias(model->add_parameters({lstmParameter.lstm_hidden_dim})) {
    unsigned ARG_DIM = 2*lstmParameter.input_dim + lstmParameter.pos_dim; // lemma+pos
    unsigned PRED_DIM = 2*lstmParameter.input_dim + lstmParameter.pos_dim;

    if (pretrained.size() > 0) {
      for (auto it : pretrained) {
        p_word.initialize(it.first, it.second);
      }
      ARG_DIM += lstmParameter.pretrained_dim;
      PRED_DIM += lstmParameter.pretrained_dim;
    }

    p_Arg = model->add_parameters({lstmParameter.hidden_dim, ARG_DIM});
    p_Pred = model->add_parameters({lstmParameter.hidden_dim, PRED_DIM});
    p_GenPath = model->add_parameters({lstmParameter.hidden_dim, 2 * lstmParameter.lstm_hidden_dim});
    p_sem_tags = model->add_lookup_parameters(train_num.semtag_num, {lstmParameter.semtag_dim});
    p_SemPath = model->add_parameters({lstmParameter.hidden_dim, 2 * lstmParameter.lstm_hidden_dim});
    p_Position = model->add_parameters({lstmParameter.hidden_dim, lstmParameter.position_dim});
    p_SentPred = model->add_parameters({lstmParameter.hidden_dim, 2 * lstmParameter.lstm_hidden_dim});
    p_SentArg = model->add_parameters({lstmParameter.hidden_dim, 2 * lstmParameter.lstm_hidden_dim});
    this->lstmParameter = lstmParameter;
  }

  Expression nn_srlabeler(ComputationGraph* hg,
                          SRLSample & srlSample,
                          SRLLookUpTable& lookUpTable,
                          Performance& perf, vector<Prediction> & results);
};



class SRLBiLSTMModel {

protected:
//  typedef std::vector<std::pair<std::string, double> > Prediction;
  unordered_map<unsigned, vector<float>> preWordEmd;
  unsigned preWordEmd_num;
  SRLBiLSTMParameter parameter;
  Model model;
  unsigned kUNK;
  SRLBuilder srl;
  SRLLookUpTable lookUpTable;
  TrainNum train_num;
  string modelPath;
  string preTrainedWordEmbPath;
  string indexDirPath;

  SRLBuilder loadModel(std::string path) {
    return SRLBuilder(&model, preWordEmd, parameter, train_num);
  }

  unsigned int loadPreTrainedWordEmb (std::string path);

  string WordsIndexFileName = "words.index";
  string PosIndexFileName = "pos.index";
  string SemtagIndexFileName = "semtag.index";
  string PositionIndexFileName = "position.index";
  string LabelIndexFileName = "label.index";
public:
  SRLBiLSTMModel(SRLBiLSTMParameter parameter, string modelPath, string preTrainedWordEmbPath, string indexDirPath);
  SRLBiLSTMModel(string modelPath, string preTrainedWordEmbPath, string indexDirPath);


//  bool predict(const Stns& snt, vector<Prediction>& predictions) {
//    ComputationGraph hg;
//    Performance perf; // 这是一个用于记录准确值的 todo 删除
//    srl.nn_srlabeler(&hg, snt, corpus.labels, perf, predictions);
//    return true;
//  }
};

class SRLBiLSTMModelTrainer: public SRLBiLSTMModel {
private:
  inline void reNewNumber() {
    train_num = TrainNum(
            lookUpTable.getTable(SRLLookUpTable::Type::word).size(),
            lookUpTable.getTable(SRLLookUpTable::Type::pos).size(),
            lookUpTable.getTable(SRLLookUpTable::Type::semtag).size(),
            lookUpTable.getTable(SRLLookUpTable::Type::position).size(),
            lookUpTable.getTable(SRLLookUpTable::Type::label).size()
    );
  }
  inline void loadData(string filePath, vector<SRLSample> & sampleList, bool changeTable = false);
public:
  SRLBiLSTMModelTrainer(string modelPath, string preTrainedWordEmbPath, string indexDirPath):
          SRLBiLSTMModel(modelPath, preTrainedWordEmbPath, indexDirPath)
  {

  }
  bool train(std::string trainDataPath, std::string devDataPath, int max_iter = 5000);
};

class SRLBiLSTMModelPredicter: public SRLBiLSTMModel {
public:
  SRLBiLSTMModelPredicter(string modelPath, string preTrainedWordEmbPath, string indexDirPath):
          SRLBiLSTMModel(modelPath, preTrainedWordEmbPath, indexDirPath)
  {
    train_num = TrainNum(
            lookUpTable.getTable(SRLLookUpTable::Type::word).loadIndex(indexDirPath + WordsIndexFileName),
            lookUpTable.getTable(SRLLookUpTable::Type::pos).loadIndex(indexDirPath + PosIndexFileName),
            lookUpTable.getTable(SRLLookUpTable::Type::semtag).loadIndex(indexDirPath + SemtagIndexFileName),
            lookUpTable.getTable(SRLLookUpTable::Type::position).loadIndex(indexDirPath + PositionIndexFileName),
            lookUpTable.getTable(SRLLookUpTable::Type::label).loadIndex(indexDirPath + LabelIndexFileName)
    );
    lookUpTable.freeze();
    preWordEmd_num = (loadPreTrainedWordEmb(preTrainedWordEmbPath));
    srl = (loadModel(modelPath));
  }

};

#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLBILSTMPREDICTOR_H
