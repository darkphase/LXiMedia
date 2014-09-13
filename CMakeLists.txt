project(lximedia)
cmake_minimum_required(VERSION 2.8)
file(STRINGS "VERSION" VERSION)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -static -DWIN32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -static -DWIN32")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

#######################################
# Extract VLC
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(VLC_VERSION "2.1.5-win32")
    execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/vlc/vlc.sh ${CMAKE_SOURCE_DIR}/ext/vlc/ ./ext/vlc/ ${VLC_VERSION})

    include_directories(
        SYSTEM
        ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/include)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libvlc.dll COMMAND cp ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libvlccore.dll COMMAND cp ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlccore.dll ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/plugins COMMAND cp -R ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/plugins ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_target(vlc ALL DEPENDS ${CMAKE_BINARY_DIR}/libvlc.dll ${CMAKE_BINARY_DIR}/libvlccore.dll ${CMAKE_BINARY_DIR}/plugins VERBATIM)
endif()

#######################################
# Extract PUPnP
set(PUPNP_VERSION "1.6.19")
execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/pupnp/pupnp.sh ${CMAKE_SOURCE_DIR}/ext/pupnp/ ./ext/pupnp/ ${PUPNP_VERSION})

include_directories(
    SYSTEM
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc)

# pupnp libraries
file(GLOB_RECURSE PUPNP_IXML_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/*.c)
file(GLOB_RECURSE PUPNP_THREADUTIL_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/*.c)
file(GLOB_RECURSE PUPNP_UPNP_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/*.c)

set_source_files_properties(
    ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=gnu99 -DUPNP_STATIC_LIB")

add_library(pupnp STATIC ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST})

target_include_directories(
    pupnp PRIVATE
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/inc
    ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/inc)

#######################################
# lximediacenter
aux_source_directory(src/lximcbackend/platform/ LXIMCBACKEND_PLATFORM_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)

set_source_files_properties(
    ${LXIMCBACKEND_PLATFORM_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DUPNP_STATIC_LIB")

add_library(lximediacenter STATIC ${LXIMCBACKEND_PLATFORM_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})

target_include_directories(
    lximediacenter PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lximcbackend)

#######################################
# lximcbackend
aux_source_directory(src/lximcbackend/backend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/backend/html/ LXIMCBACKEND_HTML_SRC_LIST)

set_source_files_properties(
    ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_HTML_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DVERSION=\\\"${VERSION}\\\"")

add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_HTML_SRC_LIST})

target_include_directories(
    lximcbackend PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lximcbackend
    ${CMAKE_SOURCE_DIR}/src/lximcbackend/backend)

target_link_libraries(lximcbackend lximediacenter pupnp)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend ws2_32 iphlpapi rpcrt4)
    set_target_properties(lximcbackend PROPERTIES LINK_FLAGS "${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
else()
    target_link_libraries(lximcbackend uuid vlc pthread)
endif()

# lximcbackend_test
aux_source_directory(test/lximcbackend/ LXIMCBACKEND_TEST_SRC_LIST)
aux_source_directory(test/lximcbackend/vlc/ LXIMCBACKEND_VLC_TEST_SRC_LIST)

set_source_files_properties(
    ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall")

add_executable(lximcbackend_test ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST})

target_include_directories(
    lximcbackend_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lximcbackend
    ${CMAKE_SOURCE_DIR}/src/lximcbackend/backend
    ${CMAKE_SOURCE_DIR}/test/lximcbackend)

target_link_libraries(lximcbackend_test pupnp)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend_test ws2_32 iphlpapi rpcrt4)
    set_target_properties(lximcbackend_test PROPERTIES LINK_FLAGS "${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
else()
    target_link_libraries(lximcbackend_test uuid vlc pthread)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
else()
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/testreport.txt COMMAND lximcbackend_test > testreport.txt DEPENDS lximcbackend_test VERBATIM)
        add_custom_target(testreport ALL DEPENDS testreport.txt VERBATIM)
    endif()
endif()

#######################################
# lximclauncher
aux_source_directory(src/lximcbackend/launcher/ LXIMCLAUNCHER_SRC_LIST)

set_source_files_properties(
    ${LXIMCLAUNCHER_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DVERSION=\\\"${VERSION}\\\"")

add_executable(lximclauncher ${LXIMCLAUNCHER_SRC_LIST})

target_include_directories(
    lximclauncher PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lximcbackend)

target_link_libraries(lximclauncher lximediacenter pupnp)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximclauncher ws2_32 iphlpapi)
else()
    target_link_libraries(lximclauncher pthread)
endif()

#######################################
# install
install(TARGETS lximcbackend lximclauncher RUNTIME DESTINATION bin)
