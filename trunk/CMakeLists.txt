project(lximedia)
cmake_minimum_required(VERSION 2.8)
file(STRINGS "VERSION" VERSION)

if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -D_FILE_OFFSET_BITS=64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_FILE_OFFSET_BITS=64")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -static -D_FILE_OFFSET_BITS=64 -DWIN32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -static -D_FILE_OFFSET_BITS=64 -DWIN32 -DUNICODE")
endif()

#######################################
# Extract VLC
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(VLC_VERSION "2.1.5-win32")
    execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/vlc/vlc.sh ${CMAKE_SOURCE_DIR}/ext/vlc/ ./ext/vlc/ ${VLC_VERSION})

    include_directories(
        SYSTEM
        ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/include)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libvlc.dll COMMAND cp ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/*.dll ${CMAKE_BINARY_DIR})
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/plugins COMMAND cp -R ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/plugins ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_target(vlc ALL DEPENDS ${CMAKE_BINARY_DIR}/libvlc.dll ${CMAKE_BINARY_DIR}/plugins VERBATIM)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(VLC_VERSION "2.1.5-macx")
    execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/vlc/vlc.sh ${CMAKE_SOURCE_DIR}/ext/vlc/ ./ext/vlc/ ${VLC_VERSION})

    include_directories(
        SYSTEM
        ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/include)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/lib COMMAND cp -R ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/lib ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/plugins COMMAND cp -R ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/plugins ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_target(vlc ALL DEPENDS ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/plugins VERBATIM)
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
# Compile JPGE
include_directories(SYSTEM ext/)

aux_source_directory(ext/jpge/ JPGE_SRC_LIST)

set_source_files_properties(
    ${JPGE_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++03 -O3")

add_library(jpge STATIC ${JPGE_SRC_LIST})

#######################################
# Compile MiniZ
aux_source_directory(ext/miniz/ MINIZ_SRC_LIST)

set_source_files_properties(
    ${MINIZ_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=gnu99 -O3")

add_library(miniz STATIC ${MINIZ_SRC_LIST})

#######################################
# Compile SHA1
include_directories(SYSTEM ext/)

aux_source_directory(ext/sha1/ SHA1_SRC_LIST)

set_source_files_properties(
    ${SHA1_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++03 -O3")

add_library(sha1 STATIC ${SHA1_SRC_LIST})

#######################################
# lximediaserver
aux_source_directory(src/mpeg/ MPEG_SRC_LIST)
aux_source_directory(src/platform/ PLATFORM_SRC_LIST)
aux_source_directory(src/pupnp/ PUPNP_SRC_LIST)
aux_source_directory(src/vlc/ VLC_SRC_LIST)

set_source_files_properties(
    ${MPEG_SRC_LIST} ${PLATFORM_SRC_LIST} ${PUPNP_SRC_LIST} ${VLC_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DUPNP_STATIC_LIB")

aux_source_directory(src/server/ SERVER_SRC_LIST)
aux_source_directory(src/server/html/ SERVER_HTML_SRC_LIST)
aux_source_directory(src/resources/ RESOURCES_SRC_LIST)

set_source_files_properties(
    ${SERVER_SRC_LIST} ${SERVER_HTML_SRC_LIST} ${RESOURCES_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DVERSION=\\\"${VERSION}\\\"")

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    SET(SERVER_RESOURCES ${CMAKE_SOURCE_DIR}/deploy/win32/lximediaserver.rc)
else()
    SET(SERVER_RESOURCES)
endif()

add_executable(
    lximediaserver
    ${MPEG_SRC_LIST} ${PLATFORM_SRC_LIST} ${PUPNP_SRC_LIST} ${VLC_SRC_LIST}
    ${SERVER_SRC_LIST} ${SERVER_HTML_SRC_LIST} ${RESOURCES_SRC_LIST} ${SERVER_RESOURCES})

target_include_directories(
    lximediaserver PRIVATE
    ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(lximediaserver pupnp jpge miniz sha1)
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries(lximediaserver uuid vlc pthread)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set_target_properties(lximediaserver PROPERTIES LINK_FLAGS "-framework Carbon ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/lib/libvlc.dylib")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximediaserver ws2_32 iphlpapi rpcrt4 shell32)
    set_target_properties(lximediaserver PROPERTIES COMPILE_FLAGS "-mwindows")
    set_target_properties(lximediaserver PROPERTIES LINK_FLAGS "-mwindows ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
endif()

#######################################
# test
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    file(GLOB_RECURSE TEST_SRC_LIST test/*.cpp)
    set_source_files_properties(${TEST_SRC_LIST} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall")
    add_executable(lximediaserver_test ${TEST_SRC_LIST})

    target_include_directories(
        lximediaserver_test PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/test)

    target_link_libraries(lximediaserver_test pupnp)
    if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
        target_link_libraries(lximediaserver_test uuid vlc pthread)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        set_target_properties(lximediaserver_test PROPERTIES LINK_FLAGS "${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/lib/libvlc.dylib")
        add_dependencies(lximediaserver_test vlc)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
        target_link_libraries(lximediaserver_test ws2_32 iphlpapi rpcrt4)
        set_target_properties(lximediaserver_test PROPERTIES COMPILE_FLAGS "-mconsole")
        set_target_properties(lximediaserver_test PROPERTIES LINK_FLAGS "-mconsole ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
        add_dependencies(lximediaserver_test vlc)
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/testreport.txt COMMAND ${CMAKE_BINARY_DIR}/lximediaserver_test > testreport.txt DEPENDS lximediaserver_test VERBATIM)
        add_custom_target(testreport ALL DEPENDS testreport.txt VERBATIM)
    endif()
endif()

#######################################
# install
install(TARGETS lximediaserver RUNTIME DESTINATION bin)

