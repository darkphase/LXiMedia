project(lximedia)
cmake_minimum_required(VERSION 2.8)

# Extract and configure PUPnP
set(PUPNP_VERSION "1.6.19")
execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/pupnp/pupnp.sh ${CMAKE_SOURCE_DIR}/ext/pupnp/ ./ext/pupnp/ ${PUPNP_VERSION})

# lximcbackend
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread")
include_directories(
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc/
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc)

get_filename_component(UPNP_LIBS ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/.libs ABSOLUTE)
get_filename_component(IXML_LIBS ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/.libs ABSOLUTE)
get_filename_component(THREADUTIL_LIBS ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/.libs ABSOLUTE)
link_directories(${UPNP_LIBS} ${IXML_LIBS} ${THREADUTIL_LIBS})

aux_source_directory(src/lximcbackend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)
add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})
target_link_libraries(lximcbackend uuid vlc upnp ixml threadutil)

aux_source_directory(test/lximcbackend/ LXIMCBACKEND_TEST_SRC_LIST)
add_executable(lximcbackend_test ${LXIMCBACKEND_TEST_SRC_LIST})
target_link_libraries(lximcbackend_test uuid vlc upnp ixml threadutil)
