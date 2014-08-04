project(lximedia)
cmake_minimum_required(VERSION 2.8)

# Extract and configure PUPnP
set(PUPNP_VERSION "1.6.19")
execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/pupnp/pupnp.sh ${CMAKE_SOURCE_DIR}/ext/pupnp/ ./pupnp/ ${PUPNP_VERSION})

# VLC memout plugin.
set(CMAKE_C_FLAGS "-std=gnu99 -pthread -D__PLUGIN__ -D_FILE_OFFSET_BITS=64 -D__USE_UNIX98 -D_REENTRANT -D_THREAD_SAFE")
include_directories(/usr/include/vlc/plugins)
add_library(access_output_lximedia_memout_plugin MODULE src/lximcbackend/vlc/modules/lximedia_memout.c)
target_link_libraries(access_output_lximedia_memout_plugin vlccore)

# VLC transcode plugin; fixes https://forum.videolan.org/viewtopic.php?f=13&t=115390
aux_source_directory(src/lximcbackend/vlc/modules/lximedia_transcode VLC_TRANSCODE_SRC_LIST)
add_library(stream_out_lximedia_transcode_plugin MODULE ${VLC_TRANSCODE_SRC_LIST})
target_link_libraries(stream_out_lximedia_transcode_plugin vlccore)

# lximcbackend
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread")
include_directories(
    ${CMAKE_BINARY_DIR}/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc/
    ${CMAKE_BINARY_DIR}/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc)

get_filename_component(UPNP_LIBS ${CMAKE_BINARY_DIR}/pupnp/libupnp-${PUPNP_VERSION}/upnp/.libs ABSOLUTE)
get_filename_component(IXML_LIBS ${CMAKE_BINARY_DIR}/pupnp/libupnp-${PUPNP_VERSION}/ixml/.libs ABSOLUTE)
get_filename_component(THREADUTIL_LIBS ${CMAKE_BINARY_DIR}/pupnp/libupnp-${PUPNP_VERSION}/threadutil/.libs ABSOLUTE)
link_directories(${UPNP_LIBS} ${IXML_LIBS} ${THREADUTIL_LIBS})

aux_source_directory(src/lximcbackend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)
add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})
target_link_libraries(lximcbackend uuid vlc upnp ixml threadutil)
