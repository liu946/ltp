
# ltp
LOCAL_PATH := $(call my-dir)/src
include $(CLEAR_VARS)

LOCAL_CPPFLAGS := -frtti # typeid 支持
LOCAL_CPPFLAGS += -fexceptions # try catch 支持
LOCAL_LDLIBS:=-L$(SYSROOT)/usr/lib -llog

LOCAL_MODULE    := ltp
LOCAL_CFLAGS    := -std=gnu++11
LOCAL_C_INCLUDES =  $(LOCAL_PATH) \
                    $(LOCAL_PATH)/utils \
                    $(LOCAL_PATH)/../thirdparty \
                    $(LOCAL_PATH)/../thirdparty/boost/include \
                    $(LOCAL_PATH)/../thirdparty/eigen-3.2.4 \
                    $(LOCAL_PATH)/../thirdparty/maxent \
                    $(LOCAL_PATH)/../thirdparty/tinyxml

LOCAL_SRC_FILES := ltp/ltp-jni.cpp ltp/Ltp.cpp ltp/LTPResource.cpp ltp/LtpNDPCK.cpp


# xml4nlp
MF_LIB_FILES    :=$(wildcard $(LOCAL_PATH)/xml4nlp/*.cpp)
MF_LIB_FILES    :=$(MF_LIB_FILES:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES +=$(MF_LIB_FILES)

# ner
LOCAL_SRC_FILES += ner/decoder.cpp
LOCAL_SRC_FILES += ner/extractor.cpp
LOCAL_SRC_FILES += ner/ner.cpp
LOCAL_SRC_FILES += ner/ner_dll.cpp

# parser.n
MF_PARSER_DIR   :=parser.n
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/classifier.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/instance.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/io.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/options.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/parser.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/system.cpp
LOCAL_SRC_FILES += $(MF_PARSER_DIR)/parser_dll.cpp

# postagger

LOCAL_SRC_FILES += postagger/decoder.cpp
LOCAL_SRC_FILES += postagger/extractor.cpp
LOCAL_SRC_FILES += postagger/postagger.cpp
LOCAL_SRC_FILES += postagger/postag_dll.cpp

# segmentor

LOCAL_SRC_FILES += segmentor/decoder.cpp
LOCAL_SRC_FILES += segmentor/preprocessor.cpp
LOCAL_SRC_FILES += segmentor/model.cpp
LOCAL_SRC_FILES += segmentor/extractor.cpp
LOCAL_SRC_FILES += segmentor/segmentor.cpp
LOCAL_SRC_FILES += segmentor/segment_dll.cpp

# splitsnt

LOCAL_SRC_FILES += splitsnt/SplitSentence.cpp

# srl
LOCAL_SRC_FILES += srl/Configuration.cpp
LOCAL_SRC_FILES += srl/DataPreProcess.cpp
LOCAL_SRC_FILES += srl/DepSRL.cpp
LOCAL_SRC_FILES += srl/FeatureExtractor.cpp
LOCAL_SRC_FILES += srl/MyTree.cpp
LOCAL_SRC_FILES += srl/Sentence.cpp
LOCAL_SRC_FILES += srl/SRLBaseline.cpp
LOCAL_SRC_FILES += srl/SRLBaselineExt.cpp
LOCAL_SRC_FILES += srl/SRL_DLL.cpp
LOCAL_SRC_FILES += srl/SRL_DLL_x.cpp

# third party
MY_THRID_PATH  :=../thirdparty

# boost
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/cmdline.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/config_file.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/convert.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/options_description.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/parsers.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/positional_options.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/split.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/utf8_codecvt_facet.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/value_semantic.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/variables_map.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/program_options/src/winmain.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/c_regex_traits.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/cpp_regex_traits.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/cregex.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/fileiter.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/icu.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/instances.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/posix_api.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/regex.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/regex_debug.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/regex_raw_buffer.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/regex_traits_defaults.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/static_mutex.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/w32_regex_traits.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/wc_regex_traits.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/wide_posix_api.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/winstances.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/boost/libs/regex/src/usinstances.cpp

# maxent
LOCAL_SRC_FILES += $(MY_THRID_PATH)/maxent/maxent.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/maxent/sgd.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/maxent/owlqn.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/maxent/lbfgs.cpp

#tinyxml
LOCAL_SRC_FILES += $(MY_THRID_PATH)/tinyxml/tinystr.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/tinyxml/tinyxml.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/tinyxml/tinyxmlerror.cpp
LOCAL_SRC_FILES += $(MY_THRID_PATH)/tinyxml/tinyxmlparser.cpp

include $(BUILD_SHARED_LIBRARY)