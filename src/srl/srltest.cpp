#include "string"
#include "iostream"
#include "vector"
#include "SRL_DLL.h"

int main(int argc, char * argv[]) {
  if (argc < 2) {
    return -1;
  }

  SRL_LoadResource(argv[1]);

  std::vector<std::string> words;
  std::vector<std::string> postags;
  std::vector<std::string> nes;
  std::vector<std::pair<int,std::string> > parse;
  std::vector< std::pair< int, std::vector< std::pair<std::string, std::pair< int, int > > > > > srl;
  words.push_back("我们"); postags.push_back("r"); parse.push_back(make_pair(6, "SBV")); nes.push_back("O");
  words.push_back("即将"); postags.push_back("d"); parse.push_back(make_pair(6, "ADV")); nes.push_back("O");
  words.push_back("以"); postags.push_back("p"); parse.push_back(make_pair(6, "ADV")); nes.push_back("O");
  words.push_back("昂扬"); postags.push_back("a"); parse.push_back(make_pair(5, "ATT")); nes.push_back("O");
  words.push_back("的"); postags.push_back("u"); parse.push_back(make_pair(3, "RAD")); nes.push_back("O");
  words.push_back("斗志"); postags.push_back("n"); parse.push_back(make_pair(2, "POB")); nes.push_back("O");
  words.push_back("迎来"); postags.push_back("v"); parse.push_back(make_pair(-1, "HED")); nes.push_back("O");
  words.push_back("新"); postags.push_back("a"); parse.push_back(make_pair(10, "ATT")); nes.push_back("O");
  words.push_back("的"); postags.push_back("u"); parse.push_back(make_pair(7, "RAD")); nes.push_back("O");
  words.push_back("一"); postags.push_back("m"); parse.push_back(make_pair(10, "ATT")); nes.push_back("O");
  words.push_back("年"); postags.push_back("q"); parse.push_back(make_pair(6, "VOB")); nes.push_back("O");
  words.push_back("。"); postags.push_back("wp"); parse.push_back(make_pair(6, "WP")); nes.push_back("O");

  DoSRL(words,postags,nes,parse,srl);

  for(int i = 0;i<srl.size();++i) {
    std::cout<<srl[i].first<<":"<<std::endl;
    for(int j = 0;j<srl[i].second.size();++j) {
      std::cout<<"\ttype = "<<srl[i].second[j].first
               <<" beg = "<<srl[i].second[j].second.first
               <<" end = "<<srl[i].second[j].second.second
               <<std::endl;
    }
  }

  SRL_ReleaseResource();
  return 0;
}