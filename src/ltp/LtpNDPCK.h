//
// Created by liu on 2016/11/12.
//

#ifndef LTP_LANGUAGE_TECHNOLOGY_PLATFORM_LTPNDPCK_H
#define LTP_LANGUAGE_TECHNOLOGY_PLATFORM_LTPNDPCK_H

#include "LTPResource.h"
#include "xml4nlp/Xml4nlp.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cassert>
#include "utils/logging.hpp"
#include "Ltp.h"

#define MAX_SENTENCE_LEN 1024
#define MAX_WORDS_NUM 256

/**
 * class LTP_NDPCK (for LTP with No Dependent ChecK)
 * Singleton
 */
class LTP_NDPCK {
    static LTP_NDPCK * instancePtr;
    LTP_NDPCK() {};
    std::string segmentor_model_file;
    std::string segmentor_lexicon_file;
    std::string postagger_model_file;
    std::string postagger_lexicon_file;
    std::string ner_model_file;
    std::string parser_model_file;
    std::string srl_model_dir;
    LTPResource _resource;    /*< the ltp resources */
public:
    static LTP_NDPCK * getInstance() {
      if (!instancePtr) {
        instancePtr = new LTP_NDPCK();
      }
      return instancePtr;
    }
    static void free() {
      if (instancePtr) {
        delete instancePtr;
      }
      instancePtr = NULL;
      return;
    }
    bool initPaths(const std::string& segmentor_model_file,
              const std::string& segmentor_lexicon_file,
              const std::string& postagger_model_file,
              const std::string& postagger_lexicon_file,
              const std::string& ner_model_file,
              const std::string& parser_model_file,
              const std::string& srl_model_dir) {
      this->segmentor_model_file = segmentor_model_file;
      this->segmentor_lexicon_file = segmentor_lexicon_file;
      this->postagger_model_file = postagger_model_file;
      this->postagger_lexicon_file = postagger_lexicon_file;
      this->ner_model_file = ner_model_file;
      this->parser_model_file = parser_model_file;
      this->srl_model_dir = srl_model_dir;
      return true;
    }

    bool loadSegmentor() {
      int ret;
      if (segmentor_lexicon_file == "") {
        ret = _resource.LoadSegmentorResource(segmentor_model_file);
      } else {
        ret = _resource.LoadSegmentorResource(segmentor_model_file, segmentor_lexicon_file);
      }
      if (0 != ret) {
        ERROR_LOG("in LTP::wordseg, failed to load segmentor resource");
        return false;
      }
      return true;
    }
    bool loadPostagger() {
      int ret;
      if (postagger_lexicon_file == "") {
        ret = _resource.LoadPostaggerResource(postagger_model_file);
      } else {
        ret = _resource.LoadPostaggerResource(postagger_model_file, postagger_lexicon_file);
      }
      if (0 != ret) {
        ERROR_LOG("in LTP::wordseg, failed to load postagger resource");
        return false;
      }
      return true;
    }
    bool loadNer() {
      if (0 != _resource.LoadNEResource(ner_model_file)) {
        ERROR_LOG("in LTP::ner, failed to load ner resource");
        return false;
      }
      return true;
    }
    bool loadParser() {
      if (0 != _resource.LoadParserResource(parser_model_file)) {
        ERROR_LOG("in LTP::parser, failed to load parser resource");
        return false;
      }
      return true;
    }
    bool loadSrl() {
      if ( 0 != _resource.LoadSRLResource(srl_model_dir)) {
        ERROR_LOG("in LTP::srl, failed to load srl resource");
        return false;
      }
      return true;
    }
    int wordseg(XML4NLP & xml);
    int postag(XML4NLP & xml);
    int ner(XML4NLP & xml);
    int parser(XML4NLP & xml);
    int srl(XML4NLP & xml);
    int splitSentence_dummy(XML4NLP & xml);
};


#endif //LTP_LANGUAGE_TECHNOLOGY_PLATFORM_LTPNDPCK_H
