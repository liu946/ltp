
#include <jni.h>
#include "Ltp.h"
#include "LtpNDPCK.h"
#include <string>


#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_initLTP(JNIEnv* env, jobject thiz, jstring path);
jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_freeLTP(JNIEnv* env, jobject thiz);
jstring Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPAnalize(JNIEnv* env, jobject thiz, jstring sentence);
jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_initLTPNDPCK(JNIEnv* env, jobject thiz, jstring path);
jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPNDPCKloadParser(JNIEnv* env, jobject thiz);
jstring Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPNDPCKParser(JNIEnv* env, jobject thiz, jstring xmlstring);

#ifdef __cplusplus
}
#endif

LTP * ltpPointer = NULL;

jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_initLTP(JNIEnv* env, jobject thiz, jstring path) {
  const char *utf8 = env->GetStringUTFChars(path, NULL);
  std::string basepath = std::string(utf8);
  std::string last_stage = "all";
  std::string segmentor_model = basepath + "ltp_data/cws.model";
  std::string segmentor_lexicon = "";
  std::string postagger_model = basepath + "ltp_data/pos.model";
  std::string postagger_lexcion = "";
  std::string ner_model = basepath + "ltp_data/ner.model";
  std::string parser_model = basepath + "ltp_data/parser.model";
  std::string srl_data= basepath + "ltp_data/srl/";

  ltpPointer = new LTP(last_stage, segmentor_model, segmentor_lexicon, postagger_model,
                         postagger_lexcion, ner_model, parser_model, srl_data);
  return JNI_TRUE;
}

jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_freeLTP(JNIEnv* env, jobject thiz) {
  if (ltpPointer) {
    delete ltpPointer;
    return JNI_TRUE;
  }
  return JNI_FALSE;
}

jstring Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPAnalize(JNIEnv* env, jobject thiz, jstring sentence) {

  const char *utf8 = env->GetStringUTFChars(sentence, NULL);
  assert(NULL != utf8);

  XML4NLP xml4nlp;
  xml4nlp.CreateDOMFromString(std::string(utf8));

  ltpPointer->srl(xml4nlp);

  std::string result;
  xml4nlp.SaveDOM(result);
  xml4nlp.ClearDOM();

  return env->NewStringUTF(result.c_str());
}

jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_initLTPNDPCK(JNIEnv* env, jobject thiz, jstring path) {
  const char *utf8 = env->GetStringUTFChars(path, NULL);
  std::string basepath = std::string(utf8);
  std::string segmentor_model = basepath + "ltp_data/cws.model";
  std::string segmentor_lexicon = "";
  std::string postagger_model = basepath + "ltp_data/pos.model";
  std::string postagger_lexcion = "";
  std::string ner_model = basepath + "ltp_data/ner.model";
  std::string parser_model = basepath + "ltp_data/parser.model";
  std::string srl_data= basepath + "ltp_data/srl/";
  LTP_NDPCK::getInstance()->initPaths(segmentor_model, segmentor_lexicon, postagger_model,
                                        postagger_lexcion, ner_model, parser_model, srl_data);
  return JNI_TRUE;
}

jboolean Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPNDPCKloadParser(JNIEnv* env, jobject thiz) {
  LTP_NDPCK::getInstance()->loadParser();
  return JNI_TRUE;
}

jstring Java_cn_edu_hit_ir_ltpmobel_MainActivity_LTPNDPCKParser(JNIEnv* env, jobject thiz, jstring xmlstring) {
    const char *utf8 = env->GetStringUTFChars(xmlstring, NULL);
    assert(NULL != utf8);
    XML4NLP xml;
    xml.LoadXMLFromString(std::string(utf8));

    LTP_NDPCK::getInstance()->parser(xml);

    std::string res;
    xml.SaveDOM(res);
    return env->NewStringUTF(res.c_str());
}

