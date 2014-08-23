project(lximedia)
cmake_minimum_required(VERSION 2.8)

# Extract PUPnP
set(PUPNP_VERSION "1.6.19")
execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/pupnp/pupnp.sh ${CMAKE_SOURCE_DIR}/ext/pupnp/ ./ext/pupnp/ ${PUPNP_VERSION})

# pupnp libraries
set(CMAKE_C_FLAGS "-pthread")
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32 -DUPNP_STATIC_LIB -static")
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/inc)

file(GLOB_RECURSE PUPNP_IXML_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-1.6.19/ixml/src/*.c)
file(GLOB_RECURSE PUPNP_THREADUTIL_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-1.6.19/threadutil/src/*.c)
file(GLOB_RECURSE PUPNP_UPNP_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-1.6.19/upnp/src/*.c)
add_library(pupnp STATIC ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST})

# lximcbackend
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread")
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32 -DUPNP_STATIC_LIB -static")
    include_directories(ext/vlc/include/)
    get_filename_component(VLC_LIBS ext/vlc/bin.win32 ABSOLUTE)
    link_directories(${VLC_LIBS})
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc/
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc/)

aux_source_directory(src/lximcbackend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)
add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})
target_link_libraries(lximcbackend pupnp)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend ws2_32 iphlpapi rpcrt4)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SOURCE_DIR}/ext/vlc/bin.win32/libvlc.dll")
else()
    target_link_libraries(lximcbackend uuid vlc)
endif()

# lximcbackend_test
include_directories(src/ test/lximcbackend/)
aux_source_directory(test/lximcbackend/ LXIMCBACKEND_TEST_SRC_LIST)
aux_source_directory(test/lximcbackend/vlc/ LXIMCBACKEND_VLC_TEST_SRC_LIST)
add_executable(lximcbackend_test ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST})
target_link_libraries(lximcbackend_test pupnp)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend_test ws2_32 iphlpapi rpcrt4)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SOURCE_DIR}/ext/vlc/bin.win32/libvlc.dll")
else()
    target_link_libraries(lximcbackend_test uuid vlc)
endif()

#add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/testreport.txt COMMAND lximcbackend_test > testreport.txt DEPENDS lximcbackend_test VERBATIM)
#add_custom_target(testreport ALL DEPENDS testreport.txt VERBATIM)
