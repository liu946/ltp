//
// Created by liu on 2016/11/12.
//

#include "LtpNDPCK.h"
#include <ctime>
#include <map>
#include <string>

#include "xml4nlp/Xml4nlp.h"
#include "splitsnt/SplitSentence.h"
#include "segmentor/segment_dll.h"
#include "postagger/postag_dll.h"
#include "parser.n/parser_dll.h"
#include "ner/ner_dll.h"
#include "srl/SRL_DLL.h"
#include "utils/codecs.hpp"
#include "utils/logging.hpp"

#if _WIN32
#pragma warning(disable: 4786 4284)
#pragma comment(lib, "segmentor.lib")
#pragma comment(lib, "postagger.lib")
#pragma comment(lib, "parser.lib")
#pragma comment(lib, "ner.lib")
#pragma comment(lib, "srl.lib")
#endif

LTP_NDPCK * LTP_NDPCK::instancePtr = nullptr;

int LTP_NDPCK::parser(XML4NLP &xml)  {
  if ( xml.QueryNote(NOTE_PARSER) ) return 0;
  // no dependence check

  void * parser = _resource.GetParser();

  if (parser == NULL) {
    ERROR_LOG("in LTP::parser, failed to init a parser");
    return kParserError;
  }

  int stnsNum = xml.CountSentenceInDocument();
  if (stnsNum == 0) {
    ERROR_LOG("in LTP::parser, number of sentences equals 0");
    return kEmptyStringError;
  }

  for (int i = 0; i < stnsNum; ++i) {
    std::vector<std::string>  vecWord;
    std::vector<std::string>  vecPOS;
    std::vector<int>          vecHead;
    std::vector<std::string>  vecRel;

    if (xml.GetWordsFromSentence(vecWord, i) != 0) {
      ERROR_LOG("in LTP::parser, failed to get words from xml");
      return kReadXmlError;
    }

    if (xml.GetPOSsFromSentence(vecPOS, i) != 0) {
      ERROR_LOG("in LTP::parser, failed to get postags from xml");
      return kReadXmlError;
    }

    if (0 == vecWord.size()) {
      ERROR_LOG("Input sentence is empty.");
      return kEmptyStringError;
    }

    if (vecWord.size() > MAX_WORDS_NUM) {
      ERROR_LOG("Input sentence is too long.");
      return kSentenceTooLongError;
    }

    if (-1 == parser_parse(parser, vecWord, vecPOS, vecHead, vecRel)) {
      ERROR_LOG("in LTP::parser, failed to perform parse on sent. #%d", i+1);
      return kParserError;
    }

    if (0 != xml.SetParsesToSentence(vecHead, vecRel, i)) {
      ERROR_LOG("in LTP::parser, failed to write parse result to xml");
      return kWriteXmlError;
    }
  }

  xml.SetNote(NOTE_PARSER);

  return 0;
}

int LTP_NDPCK::postag(XML4NLP &xml) {
  if ( xml.QueryNote(NOTE_POS) ) {
    return 0;
  }

  // no dependency

  void * postagger = _resource.GetPostagger();
  if (0 == postagger) {
    ERROR_LOG("in LTP::postag, failed to init a postagger");
    return kPostagError;
  }

  int stnsNum = xml.CountSentenceInDocument();

  if (0 == stnsNum) {
    ERROR_LOG("in LTP::postag, number of sentence equals 0");
    return kEmptyStringError;
  }

  for (int i = 0; i < stnsNum; ++i) {
    vector<string> vecWord;
    vector<string> vecPOS;

    xml.GetWordsFromSentence(vecWord, i);

    if (0 == vecWord.size()) {
      ERROR_LOG("Input sentence is empty.");
      return kEmptyStringError;
    }

    if (vecWord.size() > MAX_WORDS_NUM) {
      ERROR_LOG("Input sentence is too long.");
      return kSentenceTooLongError;
    }

    if (0 == postagger_postag(postagger, vecWord, vecPOS)) {
      ERROR_LOG("in LTP::postag, failed to perform postag on sent. #%d", i+1);
      return kPostagError;
    }

    if (xml.SetPOSsToSentence(vecPOS, i) != 0) {
      ERROR_LOG("in LTP::postag, failed to write postag result to xml");
      return kWriteXmlError;
    }
  }

  xml.SetNote(NOTE_POS);

  return 0;
}

int LTP_NDPCK::ner(XML4NLP &xml) {
  if ( xml.QueryNote(NOTE_NE) ) {
    return 0;
  }

  // no dependency

  void * ner = _resource.GetNER();

  if (NULL == ner) {
    ERROR_LOG("in LTP::ner, failed to init a ner.");
    return kNERError;
  }

  int stnsNum = xml.CountSentenceInDocument();

  if (stnsNum == 0) {
    ERROR_LOG("in LTP::ner, number of sentence equals 0");
    return kEmptyStringError;
  }

  for (int i = 0; i < stnsNum; ++ i) {
    vector<string> vecWord;
    vector<string> vecPOS;
    vector<string> vecNETag;

    if (xml.GetWordsFromSentence(vecWord, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get words from xml");
      return kReadXmlError;
    }

    if (xml.GetPOSsFromSentence(vecPOS, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get postags from xml");
      return kNERError;
    }

    if (0 == vecWord.size()) {
      ERROR_LOG("Input sentence is empty.");
      return kEmptyStringError;
    }

    if (vecWord.size() > MAX_WORDS_NUM) {
      ERROR_LOG("Input sentence is too long.");
      return kSentenceTooLongError;
    }

    if (0 == ner_recognize(ner, vecWord, vecPOS, vecNETag)) {
      ERROR_LOG("in LTP::ner, failed to perform ner on sent. #%d", i+1);
      return kNERError;
    }

    xml.SetNEsToSentence(vecNETag, i);
  }

  xml.SetNote(NOTE_NE);
  return 0;
}

int LTP_NDPCK::wordseg(XML4NLP &xml) {
  if (xml.QueryNote(NOTE_WORD)) {
    return 0;
  }

  // no dependent

  // get the segmentor pointer
  void * segmentor = _resource.GetSegmentor();
  if (0 == segmentor) {
    ERROR_LOG("in LTP::wordseg, failed to init a segmentor");
    return kWordsegError;
  }

  int stnsNum = xml.CountSentenceInDocument();

  if (0 == stnsNum) {
    ERROR_LOG("in LTP::wordseg, number of sentence equals 0");
    return kEmptyStringError;
  }

  for (int i = 0; i < stnsNum; ++ i) {
    std::string strStn = xml.GetSentence(i);
    std::vector<std::string> vctWords;

    if (ltp::strutils::codecs::length(strStn) > MAX_SENTENCE_LEN) {
      ERROR_LOG("in LTP::wordseg, input sentence is too long");
      return kSentenceTooLongError;
    }

    if (0 == segmentor_segment(segmentor, strStn, vctWords)) {
      ERROR_LOG("in LTP::wordseg, failed to perform word segment on \"%s\"",
                strStn.c_str());
      return kWordsegError;
    }

    if (0 != xml.SetWordsToSentence(vctWords, i)) {
      ERROR_LOG("in LTP::wordseg, failed to write segment result to xml");
      return kWriteXmlError;
    }
  }

  xml.SetNote(NOTE_WORD);
  return 0;
}

int LTP_NDPCK::srl(XML4NLP &xml) {
  if ( xml.QueryNote(NOTE_SRL) ) return 0;

  // no dependency

  int stnsNum = xml.CountSentenceInDocument();
  if (stnsNum == 0) {
    ERROR_LOG("in LTP::srl, number of sentence equals 0");
    return kEmptyStringError;
  }

  for (int i = 0; i < stnsNum; ++i) {
    vector<string>              vecWord;
    vector<string>              vecPOS;
    vector<string>              vecNE;
    vector< pair<int, string> > vecParse;
    vector< pair<int, vector< pair<const char *, pair< int, int > > > > > vecSRLResult;

    if (xml.GetWordsFromSentence(vecWord, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get words from xml");
      return kReadXmlError;
    }

    if (xml.GetPOSsFromSentence(vecPOS, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get postags from xml");
      return kReadXmlError;
    }

    if (xml.GetNEsFromSentence(vecNE, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get ner result from xml");
      return kReadXmlError;
    }

    if (xml.GetParsesFromSentence(vecParse, i) != 0) {
      ERROR_LOG("in LTP::ner, failed to get parsing result from xml");
      return kReadXmlError;
    }

    if (0 != SRL(vecWord, vecPOS, vecNE, vecParse, vecSRLResult)) {
      ERROR_LOG("in LTP::srl, failed to perform srl on sent. #%d", i+1);
      return kSRLError;
    }

    int j = 0;
    for (; j < vecSRLResult.size(); ++j) {
      vector<string>        vecType;
      vector< pair<int, int> >  vecBegEnd;
      int k = 0;

      for (; k < vecSRLResult[j].second.size(); ++k) {
        vecType.push_back(vecSRLResult[j].second[k].first);
        vecBegEnd.push_back(vecSRLResult[j].second[k].second);
      }

      if (0 != xml.SetPredArgToWord(i, vecSRLResult[j].first, vecType, vecBegEnd)) {
        return kWriteXmlError;
      }
    }
  }

  xml.SetNote(NOTE_SRL);
  return 0;
}

int LTP_NDPCK::splitSentence_dummy(XML4NLP &xml) {
  if ( xml.QueryNote(NOTE_SENT) ) {
    return 0;
  }

  int paraNum = xml.CountParagraphInDocument();

  if (paraNum == 0) {
    ERROR_LOG("in LTP::splitsent, There is no paragraph in doc,");
    ERROR_LOG("you may have loaded a blank file or have not loaded a file yet.");
    return kEmptyStringError;
  }

  for (int i = 0; i < paraNum; ++i) {
    vector<string> vecSentences;
    string para;
    xml.GetParagraph(i, para);

    if (0 == SplitSentence( para, vecSentences )) {
      ERROR_LOG("in LTP::splitsent, failed to split sentence");
      return kSplitSentenceError;
    }

    // dummy
    // vecSentences.push_back(para);
    if (0 != xml.SetSentencesToParagraph(vecSentences, i)) {
      ERROR_LOG("in LTP::splitsent, failed to write sentence to xml");
      return kWriteXmlError;
    }
  }

  xml.SetNote(NOTE_SENT);
  return 0;
}
