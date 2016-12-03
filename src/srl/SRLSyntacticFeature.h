//
// Created by liu on 2016/11/28.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLSYNTACTICFEATURE_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLSYNTACTICFEATURE_H

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

class SRLSyntacticFeature {
private:
    vector<int> argument_path;
    vector<int> predicate_path;
    vector<string> argument_lemma_path;
    vector<string> argument_syntactic_label_path;
    vector<string> predicate_lemma_path;
    vector<string> predicate_syntactic_label_path;
    int predicate;
    int word;
    unsigned long sentenceLength;
    const LTPData & ltpData;
    /**
     * 展开所有的path
     */
    void initSyntacticPathFeature4();
public:
    SRLSyntacticFeature(const LTPData & _ltpData, const int  _predicate, const int _word):
            ltpData(_ltpData),
            predicate(_predicate),
            word(_word),
            sentenceLength(_ltpData.vecParent.size()){}
    /**
     * 初始化两个句法路径
     * @param predicateToRootPath 入参，谓词的句法路径，由谓词的getPathToRoot生成。
     * @param isOnPredicateToRootPath 入参，谓词句法路径上的词，由谓词的getPathToRoot生成。
     */
    void initSyntacticFeature(vector<int>& predicateToRootPath, vector<int>& isOnPredicateToRootPath);
    /**
     * 此词到句法树ROOT的路径
     * @param ToRootPath 出参，谓词的句法路径
     * @param isOnToRootPath 出参，谓词句法路径上的词
     */
    void getPathToRoot(vector<int>& ToRootPath, vector<int>& isOnToRootPath);
    const vector<int>& getArgumentPath() const { return argument_path; }
    const vector<int>& getPredicatePath() const { return predicate_path; }
    const vector<string> &getArgument_lemma_path() const {
      return argument_lemma_path;
    }
    const vector<string> &getArgument_syntactic_label_path() const {
      return argument_syntactic_label_path;
    }
    const vector<string> &getPredicate_lemma_path() const {
      return predicate_lemma_path;
    }
    const vector<string> &getPredicate_syntactic_label_path() const {
      return predicate_syntactic_label_path;
    }

};


#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_SRLSYNTACTICFEATURE_H
