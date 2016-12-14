include_directories (
    .
    ${util_DIR} 
    ${SOURCE_DIR}/utils
    ${SOURCE_DIR}/
    ${THIRDPARTY_DIR}/dynet/
    ${THIRDPARTY_DIR}/maxent)

set (srl_VERSION "0.0.1")

set (srl_SRC
    Configuration.cpp
    Configuration.h
    ConstVar.h
    options.h
    DataPreProcess.cpp
    DataPreProcess.h
    DataStruct.h
    DepSRL.cpp
    DepSRL.h
    FeatureExtractor.cpp
    FeatureExtractor.h
    MyStruct.h
    MyTree.cpp
    MyTree.h
    Sentence.cpp
    Sentence.h
    SRLBaseline.cpp
    SRLBaselineExt.cpp
    SRLBaselineExt.h
    SRLBaseline.h
    SRLSyntacticFeature.cpp
    SRLSyntacticFeature.h
        SRLBiLSTMModel.cpp
        SRLBiLSTMModel.h
    SRL_DLL.cpp
    SRL_DLL.h
    SRL_DLL_x.cpp
    tree.hh)

add_library (srl_static_lib STATIC ${srl_SRC})

set_target_properties (srl_static_lib   PROPERTIES
    OUTPUT_NAME     srl)

add_library (srl_shared_lib SHARED ${srl_SRC})

set_target_properties (srl_shared_lib   PROPERTIES
    VERSION         ${srl_VERSION}
    OUTPUT_NAME     srl)

target_link_libraries (srl_static_lib maxent_static_lib)
target_link_libraries (srl_shared_lib maxent_shared_lib)

#configure_file (
#    SRL_DLL.h
#    ${INCLUDE_OUTPUT_PATH}/ltp/SRL_DLL.h)

link_directories (${LIBRARY_OUTPUT_PATH})

# look for Boost
if(DEFINED ENV{BOOST_ROOT})
    set(Boost_NO_SYSTEM_PATHS ON)
endif()
set(Boost_REALPATH ON)
find_package(Boost COMPONENTS program_options serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
set(LIBS ${LIBS} ${Boost_LIBRARIES})


include_directories(~/soft/dynet) # -I
link_directories(~/soft/dynet/build/dynet) # -L
INCLUDE_DIRECTORIES(~/soft/eigen) # ???
set(testsrl_SRC testMain.cpp)
add_executable(testsrl ${testsrl_SRC})
target_link_libraries (testsrl dynet ${Boost_LIBRARIES} maxent srl_static_lib)

include_directories(~/soft/dynet) # -I
link_directories(~/soft/dynet/build/dynet) # -L
INCLUDE_DIRECTORIES(~/soft/eigen) # ???
set(trainSrlTest_SRC TrainSRLTest.cpp SRLBiLSTMModel.cpp SRLLookUpTable.cpp trainStn.cpp)
add_executable(trainSrlTest ${trainSrlTest_SRC})
target_link_libraries (trainSrlTest dynet ${Boost_LIBRARIES} )

# Training suite is not supported in windows
if (NOT WIN32)
set (lgsrl_SRC lgsrl.cpp Corpus.cpp GetInstance.cpp)
add_executable (lgsrl ${lgsrl_SRC})
target_link_libraries (lgsrl maxent srl_static_lib)
set_target_properties (lgsrl
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TOOLS_DIR}/train/)
endif()
