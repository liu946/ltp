include_directories (.
        ${THIRDPARTY_DIR}/boost/include
        ${THIRDPARTY_DIR}/eigen-3.2.10/
        ${THIRDPARTY_DIR}/dynet
        )

# look for Boost
#if(DEFINED ENV{BOOST_ROOT})
#    set(Boost_NO_SYSTEM_PATHS ON)
#endif()
#set(Boost_REALPATH ON)
#find_package(Boost COMPONENTS program_options serialization REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})
#set(LIBS ${LIBS} ${Boost_LIBRARIES})

link_directories(${THIRDPARTY_DIR}/dynet/build/dynet) # -L

set(LIBS ${LIBS} ${Boost_LIBRARIES})

add_subdirectory(include)
include_directories(
        include
)

set(LIBS ${LIBS} base_static_lib)

set (srl_VERSION "0.0.1")

INCLUDE_DIRECTORIES(~/soft/eigen) # -I
include_directories(~/soft/dynet) # -I

include_directories(common)
add_subdirectory(common)

set(LIBS ${LIBS} common_static_lib)

add_subdirectory(srl)
set(LIBS ${LIBS} srl_static_lib)

add_subdirectory(pp)
set(LIBS ${LIBS} pp_static_lib)

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
    SRL_DLL.cpp
    SRL_DLL.h
    SRL_DLL_x.cpp
    tree.hh)

add_library (ppsrl_static_lib STATIC ${srl_SRC})

set_target_properties (ppsrl_static_lib   PROPERTIES
    OUTPUT_NAME     srl)

#add_library (ppsrl_shared_lib SHARED ${srl_SRC})
#
#set_target_properties (ppsrl_shared_lib   PROPERTIES
#    VERSION         ${srl_VERSION}
#    OUTPUT_NAME     srl)

target_link_libraries (ppsrl_static_lib dynet ${LIBS})
#target_link_libraries (ppsrl_shared_lib maxent_shared_lib)

configure_file (
    SRL_DLL.h
    ${INCLUDE_OUTPUT_PATH}/ltp/SRL_DLL.h)

link_directories (${LIBRARY_OUTPUT_PATH})

## Training suite is not supported in windows
#if (NOT WIN32)
#set (lgsrl_SRC lgsrl.cpp Corpus.cpp GetInstance.cpp)
#add_executable (lgsrl ${lgsrl_SRC})
#target_link_libraries (lgsrl maxent srl_static_lib)
#set_target_properties (lgsrl
#    PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${TOOLS_DIR}/train/)
#endif()

add_executable (testsrl srltest.cpp)
target_link_libraries (testsrl dynet ppsrl_static_lib)
