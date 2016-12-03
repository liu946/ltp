//
// Created by liu on 2016/11/28.
//

#include "SRLSyntacticFeature.h"
#include "MyStruct.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <tree.hh>
#include <set>
#include <map>

using namespace std;


void SRLSyntacticFeature::initSyntacticFeature(vector<int> &predicateToRootPath, vector<int> &isOnPredicateToRootPath) {
  predicate_path.resize(0);
  argument_path.resize(0);

  // 沿句法树向上寻找
  for (int thisNode = word; thisNode != -1; thisNode = ltpData.vecParent[thisNode]) {
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
  initSyntacticPathFeature4();
}

void SRLSyntacticFeature::getPathToRoot(vector<int>& ToRootPath, vector<int>& isOnToRootPath) {
  ToRootPath.resize(0);
  isOnToRootPath.resize(sentenceLength, -1);
  for (int thisNode = word, index = 0; thisNode != -1; thisNode = ltpData.vecParent[thisNode], index++) {
    ToRootPath.push_back(thisNode);
    isOnToRootPath[thisNode] = index;
  }
}

void SRLSyntacticFeature::initSyntacticPathFeature4() {
  for (vector<int>::iterator i = argument_path.begin(); i != argument_path.end(); i++) {
    argument_lemma_path.push_back(ltpData.vecWord[*i]);
    argument_syntactic_label_path.push_back(ltpData.vecRelation[*i]);
  }
  for (vector<int>::iterator i = predicate_path.begin(); i != predicate_path.end(); i++) {
    predicate_lemma_path.push_back(ltpData.vecWord[*i]);
    predicate_syntactic_label_path.push_back(ltpData.vecRelation[*i]);
  }
}

