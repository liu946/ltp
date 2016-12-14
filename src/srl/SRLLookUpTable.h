//
// Created by liu on 2016/12/13.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_FREEZABLELOOKUPTABLE_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_FREEZABLELOOKUPTABLE_H

#include "vector"
#include "string"
#include "iostream"
#include <unordered_map>
#include <fstream>

using namespace std;

class LookUpTable {
public:
  unordered_map<string, unsigned> string_to_num;
  unordered_map<unsigned, string> num_to_string;

  virtual string getStringByIndex(unsigned index) {
    return num_to_string[index]; // todo throw error
  }

  inline void pushString(const string &s) {
    unsigned index = (unsigned)string_to_num.size();
    string_to_num[s] = index;
    num_to_string[index] = s;
  }

  virtual unsigned getOrPushString(const string &s) {
    if (string_to_num.find(s) == string_to_num.end()) {
      pushString(s);
    }
    return string_to_num[s];
  }

  virtual unsigned loadIndex(string path) {
    ifstream in(path);
    string word;
    unsigned num;
    in >> num;
    for (unsigned i = 0; i < num; i++) {
      in >> word;
      string_to_num[word] = i;
      num_to_string[i] = word;
    }
    in.close();
    return num;
  }

  virtual unsigned saveIndex(string path) {
    throw string("unfinished");
  }

  unsigned size() {
    return (unsigned)string_to_num.size();
  }


};

class FreezableLookUpTable: public LookUpTable {
public:
  bool freezed = false;
  string unk;

  void freeze(bool freezed = true) {
    this->freezed = freezed;
  }

  unsigned getOrPushString(const string &s) {
    if (freezed) {
      return unchangeGet(s);
    } else {
      return this->LookUpTable::getOrPushString(s);
    }
  }

  unsigned unchangeGet(const string &s) {
    if (string_to_num.find(s) == string_to_num.end()) {
      return string_to_num[unk];
    } else {
      return string_to_num[s];
    }
  }

  FreezableLookUpTable(string unk = "<UNK>"):unk(unk) {
    getOrPushString(unk);
  }
};

class SRLLookUpTable {
  vector<FreezableLookUpTable> tables;
public:
  enum Type {
    word = 0,
    pos,
    position,
    semtag,
    label,
    all // must at end of type
  };
  SRLLookUpTable():tables(Type::all) {

  }
  void freeze() {
    for (int i = 0; i < Type::all; i++) {
      tables[i].freeze();
    }
  }

  unsigned getOrPushString(Type type, const string &s) {
    return tables[type].getOrPushString(s);
  }

  inline FreezableLookUpTable & getTable(Type type) {
    return tables[type];
  }

  virtual string getStringByIndex(Type type, unsigned index) {
    return getTable(type).getStringByIndex(index);
  }

};


#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_FREEZABLELOOKUPTABLE_H
