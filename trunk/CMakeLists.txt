project(lximedia)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-g -std=c++11 -pthread -Iext/pupnp/libupnp-1.6.19/upnp/inc/ -Iext/pupnp/libupnp-1.6.19/ixml/inc/")
link_directories(ext/pupnp/libupnp-1.6.19/upnp/.libs ext/pupnp/libupnp-1.6.19/ixml/.libs ext/pupnp/libupnp-1.6.19/threadutil/.libs)

aux_source_directory(src/lximcbackend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST})
target_link_libraries(lximcbackend upnp ixml threadutil)
