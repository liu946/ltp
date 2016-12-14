//
// Created by liu on 2016/12/9.
//

#ifndef CNN_TRAINSTN_H
#define CNN_TRAINSTN_H

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/variant.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "SRLLookUpTable.h"

using namespace std;
using namespace boost;

/**
 * 继承此类的类型可以有feature
 * 通过 getFeature 和 setFeature 方法添加
 */
class FeatureInstance {
protected:
  map<string, variant<int, unsigned, string, vector<string>, vector<int>, vector<unsigned>>> featureMap;
public:
  FeatureInstance(const FeatureInstance& that):featureMap(that.featureMap) {}
  FeatureInstance() {
    featureMap.clear();
  }

  template <class T = string>
  const T& getFeature(string featureName) {
    return *get<T>(&(featureMap[featureName]));
  }

  template <class T = string>
  const T& setFeature(string featureName, const T& feature) {
    featureMap[featureName] = feature;
    return feature;
  }

  bool isHasFeature(string featureName) const {
    return (bool)featureMap.count(featureName);
  }

  void registerFeature(string featureName, LookUpTable& table) {
    table.getOrPushString(getFeature(featureName));
  }

  void registerFeatureVector(string featureName, LookUpTable& table) {
    vector<string> featureList = getFeature<vector<string>>(featureName);
    for (auto i:featureList) {
      table.getOrPushString(i);
    }
  }
};

template <class InstanceClass>
class FeatureInstanceList {
protected:
  vector<InstanceClass *> instanceList;
public:
  FeatureInstanceList () {
    clear();
  }
  unsigned size() {
    return instanceList.size();
  }

  void destory() {
    for (int i = 0; i < instanceList.size(); ++i) {
      delete instanceList[i];
    }
  }

  void clear() { instanceList.clear(); }

  template <class T = string>
  const vector<T> getFeatureList(string featureName) {
    vector<T> res;
    for (int i = 0; i < instanceList.size(); ++i) {
      res.push_back(getInstanceFeature<T>(i, featureName));
    }
    return res;
  }

  template <class T = string>
  const vector<T>& setFeatureList(string featureName, vector<T> featureList) {
    if (featureList.size() != instanceList.size()) throw "长度不匹配 error"; // todo 类型化
    for (int i = 0; i < instanceList.size(); ++i) {
      instanceList[i]->template setFeature<T>(featureName, featureList[i]);
    }
    return instanceList;
  }

  template <class T = string>
  const vector<T>& setFeatureList(string featureName, T commonFeature) {
    for (int i = 0; i < instanceList.size(); ++i) {
      instanceList[i]->template setFeature<T>(featureName, commonFeature);
    }
    return instanceList;
  }

  const InstanceClass* insertInstance(InstanceClass * instance) {
    instanceList.push_back(instance);
    return instance;
  }

  const InstanceClass* setInstance(int index, InstanceClass * instance) {
    return instanceList[index] = instance;
  }

  template <class T = string>
  const T setInstanceFeature(int index, string featureName, T feature) {
    return instanceList[index]->template setFeature<T>(featureName, feature);
  }

  InstanceClass* getInstance(int index) {
    return (InstanceClass*)instanceList[index];
  }

  template <class T = string>
  const T& getInstanceFeature(int index, string featureName) {
    return instanceList[index]->template getFeature<T>(featureName);
  }

  void registerInstanceFeature(string featureName, LookUpTable& table) {
    int num = (int)this->instanceList.size();
    for (int j = 0; j < num; ++j) {
      getInstance(j)->registerFeature(featureName, table);
    }
  }
  void registerInstanceFeatureVector(string featureName, LookUpTable& table) {
    int num = (int)this->instanceList.size();
    for (int j = 0; j < num; ++j) {
      getInstance(j)->registerFeatureVector(featureName,table);
    }
  }
};

class Word: public FeatureInstance {
private:
  string word;

public:
  Word(string word):word(word) {
    setFeature("word", word);
    setFeature("lemma", word); // zh
  }
  Word(const vector<string>& lineFeatures):word(lineFeatures[1]) {
    int index = lexical_cast<int>(lineFeatures[0]) - 1;
    int parent = lexical_cast<int>(lineFeatures[8]) - 1;
    setFeature<int>("index", index);
    setFeature("word", word);
    setFeature("lemma", lineFeatures[2]);
    setFeature("pos", lineFeatures[4]);
    setFeature("predicate", lineFeatures[13]);
    setFeature<int>("parent", parent);
    setFeature("semtag", lineFeatures[10]);
    setFeature("position", (index <= parent ? "before" : "after"));
    vector<string> label;
    for (int j = 14 /*14 位置开始论元标号*/; j < lineFeatures.size(); ++j) {
      label.push_back(lineFeatures[j]);
    }
    setFeature<vector<string>>("label", label);
  }
};

class Sample:public FeatureInstance {
public:
  Sample() {}
  Sample(const FeatureInstance& that):FeatureInstance(that) {}
};

class SRLSample: public FeatureInstanceList<Sample>, public FeatureInstance {
public:
  SRLSample() {
  }

  ~SRLSample(){ /* 内存泄露? */ }

  const Sample* insertSubSample() {
    return insertInstance(new Sample);
  }

  const Sample* getSample(int index) { return getInstance(index); }

//  SRLSample& copy(const SRLSample& that) {
//
//  }

};

class SRLSyntacticFeature {
private:
  vector<int> argument_path;
  vector<int> predicate_path;
  int predicate;
  int word;
  unsigned long sentenceLength;
  const vector<int> & parentList;
public:
  SRLSyntacticFeature(const vector<int> & parentList, const int  predicate, const int word):
          parentList(parentList),
          predicate(predicate),
          word(word),
          sentenceLength(parentList.size()){}
  /**
   * 初始化两个句法路径
   * @param predicateToRootPath 入参，谓词的句法路径，由谓词的getPathToRoot生成。
   * @param isOnPredicateToRootPath 入参，谓词句法路径上的词，由谓词的getPathToRoot生成。
   */
  void initSyntacticFeature(vector<int>& predicateToRootPath, vector<int>& isOnPredicateToRootPath) {
    predicate_path.clear();
    argument_path.clear();

    // 沿句法树向上寻找
    for (int thisNode = word; thisNode != -1; thisNode = parentList[thisNode]) {
      if (isOnPredicateToRootPath[thisNode] == -1) { // 当前节点没在谓词到根的语法路径上
        argument_path.push_back(thisNode);
      } else {// 找到在谓词的句法路径上
        int nca = thisNode;
        argument_path.push_back(thisNode);
        unsigned i;
        for (i = 0; predicateToRootPath[i] != nca; i++) {
          predicate_path.push_back(predicateToRootPath[i]);
        }
        predicate_path.push_back(predicateToRootPath[i]);
        break;
      }
    }
  }
  /**
   * 此词到句法树ROOT的路径
   * @param ToRootPath 出参，谓词的句法路径
   * @param isOnToRootPath 出参，谓词句法路径上的词
   */
  void getPathToRoot(vector<int>& ToRootPath, vector<int>& isOnToRootPath) {
    ToRootPath.resize(0);
    isOnToRootPath.resize(sentenceLength, -1);
    for (int thisNode = word, index = 0; thisNode != -1; thisNode = parentList[thisNode], index++) {
      ToRootPath.push_back(thisNode);
      isOnToRootPath[thisNode] = index;
    }
  }
  const vector<int>& getArgumentPath() const { return argument_path; }
  const vector<int>& getPredicatePath() const { return predicate_path; }
};

class Stn: public FeatureInstance, public FeatureInstanceList<Word> {
  unsigned word_num;
  /**
   * 未完成
   * @param predicateList
   * @return
   */
  vector<int> eraseNoArgPredicate(vector<int> &predicateList) {
    vector<int> predicateListRes;
    for (int j = 0; j < predicateList.size(); ++j) {
      int argCounter = 0;
      for (int k = 0; k < instanceList.size(); ++k) {
        if (getInstanceFeature<string>(j, "label") != "_") {
          argCounter ++;
          break;
        }
      }
    }
    return predicateListRes;
  }

  vector<int> extractPredicates() {
    vector<int> predicateList;
    for (int i = 0; i < word_num; ++i) {
      if (getInstanceFeature(i, "predicate") != "_") {
        predicateList.push_back(i);
      }
    }
//    todo 去掉没有论元的谓词？注意论元顺序的改变
//    predicateList = eraseNoArgPredicate(predicateList);
    return predicateList;
  }
public:
  Stn(vector<vector<string>>& wordLines) {
    word_num = (unsigned)wordLines.size();
    for (vector<vector<string>>::iterator wordLine = wordLines.begin(); wordLine != wordLines.end(); wordLine++) {
      insertInstance(new Word(*wordLine));
    }
    setFeature<vector<int>>("predicates", extractPredicates());
  }
  ~Stn() { destory(); }

  /**
   * to extract simple features
   * @param sample
   * @param word
   * @param tables
   */
  inline void initSampleByWord(Sample& sample, Word& word, SRLLookUpTable& tables, unsigned predicateIndex) {
    sample.setFeature<unsigned>("word", tables.getOrPushString(SRLLookUpTable::Type::word, word.getFeature("word")));
    sample.setFeature<unsigned>("position", tables.getOrPushString(SRLLookUpTable::Type::position, word.getFeature("position")));
    sample.setFeature<unsigned>("pos", tables.getOrPushString(SRLLookUpTable::Type::pos, word.getFeature("pos")));
    sample.setFeature<unsigned>("semtag", tables.getOrPushString(SRLLookUpTable::Type::semtag, word.getFeature("semtag")));
    // label
    sample.setFeature<unsigned>("label", tables.getOrPushString(SRLLookUpTable::Type::label, word.getFeature<vector<string>>("label")[predicateIndex]));
  }

  bool extractSample(vector<SRLSample>& SampleList, SRLLookUpTable& tables);


  inline bool injectVectorUIntByTable(int index,
                                      SRLSample& srlSample,
                                      SRLLookUpTable& tables,
                                      SRLLookUpTable::Type type,
                                      string featureName,
                                      string originName,
                                      string targetName) {
    vector<int> argPath = srlSample.getInstanceFeature<vector<int>>(index, originName);
    vector<unsigned> argPathWordIndex;
    for (int i = 0; i < argPath.size(); i++) {

      argPathWordIndex.push_back(srlSample.getInstanceFeature<unsigned>(argPath[i], featureName));
    }
    srlSample.setInstanceFeature<vector<unsigned>>(index, targetName, argPathWordIndex);
    return true;
  }

  bool ExtractSrlSyntacticPathFeaturesIndexes(SRLSample& srlSample, SRLLookUpTable& tables) {
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

  bool ExtractSrlSyntacticPathFeatures(int predicate, SRLSample &sample);

};

class TrainStn: public FeatureInstanceList<Stn> {

public:
  ~TrainStn() { destory(); }
  /**
   *
   * @param in 文件输入流，文件格式相关
   * @return
   */
  bool read(ifstream& in);

  bool ExtractFeatures(vector<SRLSample> & sampleList, SRLLookUpTable& tables) {
    for (int i = 0; i < instanceList.size(); i++) {
      ((Stn*)instanceList[i])->extractSample(sampleList, tables);
    }
    return true;
  }

  void registerSubInstanceFeature(string featureName, LookUpTable& table) {
    int num = (int)this->instanceList.size();
    for (int j = 0; j < num; ++j) {
      getInstance(j)->registerInstanceFeature(featureName, table);
    }
  }
  void registerSubInstanceFeatureVector(string featureName, LookUpTable& table) {
    int num = (int)this->instanceList.size();
    for (int j = 0; j < num; ++j) {
      getInstance(j)->registerInstanceFeatureVector(featureName,table);
    }
  }

};


#endif //CNN_TRAINSTN_H
