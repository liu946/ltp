//
// Created by liu on 2016/12/12.
//

#include "SRLBiLSTMModel.h"
#include <dynet/init.h>

int main(int argc, char * argv[]) {
  dynet::initialize(argc, argv);

  SRLBiLSTMModelTrainer srl("ltp_data/srl/srl.bilstm.model", "ltp_data/srl/w2v-d200.txt", "ltp_data/srl/index/");
  srl.train("ltp_data/srl/devel.txt", "ltp_data/srl/devel.txt");

  return 0;
}