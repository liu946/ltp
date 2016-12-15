//
// Created by liu on 2016/12/13.
//

#include "trainStn.h"
#include <vector>
#include "iostream"
using namespace std;



bool TrainStn::read(ifstream& in) {
  string line;
  vector<vector<string>> input;
  while (getline(in, line)) {
    if (line.empty()) {
      // 处理这个句子，生成多个sample
      insertInstance(new Stn(input));
      input.clear();
      continue;
    }
    vector<string> lineFeatures;
    split(lineFeatures, line, is_any_of("\t\n"));
    input.push_back(lineFeatures);
  }
  return true;
}

bool Stn::extractSample(vector<SRLSample>& SampleList, SRLLookUpTable& tables) {
  vector<int> predicateList = getFeature<vector<int>>("predicates");
  for (int k = 0; k < predicateList.size(); ++k) {
    SRLSample baseSample;
    for (int j = 0; j < word_num; ++j) {
      Word * word = getInstance(j);
      Sample * wordSample = new Sample();
      initSampleByWord(*wordSample, *word, tables, (unsigned int) k);
      baseSample.insertInstance(wordSample);
    }
    SRLSample sample = baseSample;
    sample.setFeature<unsigned>("predicate", (const unsigned int &) predicateList[k]);
    ExtractSrlSyntacticPathFeatures(predicateList[k], sample);
    ExtractSrlSyntacticPathFeaturesIndexes(sample, tables);
    SampleList.push_back(sample);
  }
  return true;
}

bool Stn::ExtractSrlSyntacticPathFeaturesIndexes(SRLSample& srlSample, SRLLookUpTable& tables) {

  for (int j = 0; j < srlSample.size(); ++j) {
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::word, "word", "arguementPath", "arguementPathIndex");
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::word, "word", "predicatePath", "predicatePathIndex");
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::pos, "pos", "arguementPath", "arguementGenPathIndex");
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::pos, "pos", "predicatePath", "predicateGenPathIndex");
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::semtag, "semtag", "arguementPath", "arguementSemtagPathIndex");
    injectVectorUIntByTable(j, srlSample, tables, SRLLookUpTable::Type::semtag, "semtag", "predicatePath", "predicateSemtagPathIndex");
  }
  return true;
}

bool Stn::ExtractSrlSyntacticPathFeatures(int predicate, SRLSample &sample) {
  sample.setFeature<int>("predicate", predicate);

  vector<SRLSyntacticFeature> srlSyntacticFeatureList;
  srlSyntacticFeatureList.clear();
  vector<int> parentList = getFeatureList<int>("parent");
  for (int word = 0; word < word_num; word++) {
    SRLSyntacticFeature syntacticFeature(parentList, predicate, word);
    srlSyntacticFeatureList.push_back(syntacticFeature);
  }
  vector<int> predicateToRootPath;
  vector<int> isOnPredicateToRootPath(word_num, -1); // -1 表示不在路径上，其他数字表示路径上的index
  srlSyntacticFeatureList[predicate].getPathToRoot(predicateToRootPath, isOnPredicateToRootPath);
  // init syntactic path
  for (int word = 0; word < word_num; word++) {
    srlSyntacticFeatureList[word].initSyntacticFeature(predicateToRootPath, isOnPredicateToRootPath);
    sample.setInstanceFeature<vector<int>>(word, "arguementPath", srlSyntacticFeatureList[word].getArgumentPath());
    sample.setInstanceFeature<vector<int>>(word, "predicatePath", srlSyntacticFeatureList[word].getPredicatePath());
  }
  return true;
}
