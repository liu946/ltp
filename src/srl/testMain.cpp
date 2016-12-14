//
// Created by liu on 16/10/27.
//

#include <iostream>
#include <vector>
#include <dynet/init.h>

#include "SRL_DLL.h"

int main(int argc, char * argv[]) {
    dynet::initialize(argc, argv);
    SRL_LoadResource("./ltp_data/srl");

    std::vector<std::string> words;
    std::vector<std::string> postags;
    std::vector<std::string> nes;
    std::vector<std::pair<int,std::string> > parse;
    std::vector< std::pair< int, std::vector< std::pair<std::string, std::pair< int, int > > > > > srl;
    /**
     * note : srl is [(v.pos, [(label.type, (start.pos, end.pos))])]
     */
    words.push_back("我们"); postags.push_back("r"); nes.push_back("0"); parse.push_back(make_pair(6, "SBV"));
    words.push_back("即将"); postags.push_back("d"); nes.push_back("0"); parse.push_back(make_pair(6, "ADV"));
    words.push_back("以"); postags.push_back("p"); nes.push_back("0"); parse.push_back(make_pair(6, "ADV"));
    words.push_back("昂扬"); postags.push_back("a"); nes.push_back("0"); parse.push_back(make_pair(5, "ATT"));
    words.push_back("的"); postags.push_back("u"); nes.push_back("0"); parse.push_back(make_pair(3, "RAD"));
    words.push_back("斗志"); postags.push_back("n"); nes.push_back("0"); parse.push_back(make_pair(2, "POB"));
    words.push_back("迎来"); postags.push_back("v"); nes.push_back("0"); parse.push_back(make_pair(-1, "HED"));
    words.push_back("新"); postags.push_back("a"); nes.push_back("0"); parse.push_back(make_pair(10, "ATT"));
    words.push_back("的"); postags.push_back("u"); nes.push_back("0"); parse.push_back(make_pair(7, "RAD"));
    words.push_back("一"); postags.push_back("m"); nes.push_back("0"); parse.push_back(make_pair(10, "ATT"));
    words.push_back("年"); postags.push_back("q"); nes.push_back("0"); parse.push_back(make_pair(6, "VOB"));
    words.push_back("。"); postags.push_back("wp"); nes.push_back("0"); parse.push_back(make_pair(6, "WP"));
//    words.push_back("一把手");  postags.push_back("n"); nes.push_back("O"); parse.push_back(make_pair(2,"SBV"));
//    words.push_back("亲自");    postags.push_back("d"); nes.push_back("O"); parse.push_back(make_pair(2,"ADV"));
//    words.push_back("过问");    postags.push_back("v"); nes.push_back("O"); parse.push_back(make_pair(-1,"HED"));
//    words.push_back("。");      postags.push_back("wp");nes.push_back("O"); parse.push_back(make_pair(2,"WP"));

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

