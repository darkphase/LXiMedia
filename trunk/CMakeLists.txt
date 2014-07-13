project(lximedia)
cmake_minimum_required(VERSION 2.8)

# VLC memout plugin.
set(CMAKE_C_FLAGS "-g -std=gnu99 -pthread -D__PLUGIN__ -D_FILE_OFFSET_BITS=64 -D__USE_UNIX98 -D_REENTRANT -D_THREAD_SAFE")
include_directories(/usr/include/vlc/plugins)
add_library(access_output_lximedia_memout_plugin MODULE src/lximcbackend/vlc/modules/lximedia_memout.c)
target_link_libraries(access_output_lximedia_memout_plugin vlccore)

# VLC transcode plugin; fixes https://forum.videolan.org/viewtopic.php?f=13&t=115390
aux_source_directory(src/lximcbackend/vlc/modules/lximedia_transcode VLC_TRANSCODE_SRC_LIST)
add_library(stream_out_lximedia_transcode_plugin MODULE ${VLC_TRANSCODE_SRC_LIST})
target_link_libraries(stream_out_lximedia_transcode_plugin vlccore)

# lximcbackend
set(CMAKE_CXX_FLAGS "-g -std=c++11 -pthread")
include_directories(/development/lximedia/build-trunk-cmake/ext/pupnp/libupnp-1.6.19/upnp/inc/ /development/lximedia/build-trunk-cmake/ext/pupnp/libupnp-1.6.19/ixml/inc/)
link_directories(ext/pupnp/libupnp-1.6.19/upnp/.libs ext/pupnp/libupnp-1.6.19/ixml/.libs ext/pupnp/libupnp-1.6.19/threadutil/.libs)

aux_source_directory(src/lximcbackend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)
add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})
target_link_libraries(lximcbackend uuid vlc upnp ixml threadutil)
