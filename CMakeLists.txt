project(lximedia)
cmake_minimum_required(VERSION 2.8)
file(STRINGS "VERSION" VERSION)

if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -D_FILE_OFFSET_BITS=64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_FILE_OFFSET_BITS=64")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/LXiMediaServer.app/Contents/MacOS)
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/LXiMediaServer.app/Contents/Resources)
    file(COPY deploy/macx/lximedia.icns DESTINATION ${CMAKE_BINARY_DIR}/LXiMediaServer.app/Contents/Resources)
    file(COPY deploy/macx/Info.plist DESTINATION ${CMAKE_BINARY_DIR}/LXiMediaServer.app/Contents)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -static -D_FILE_OFFSET_BITS=64 -DWIN32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -static -D_FILE_OFFSET_BITS=64 -DWIN32 -DUNICODE")
endif()

#######################################
# Extract VLC
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    include_directories(SYSTEM ext/vlc/include)

    file(COPY ext/vlc/bin.win32/libvlc.dll DESTINATION .)
    file(COPY ext/vlc/bin.win32/libvlccore.dll DESTINATION .)
    file(COPY ext/vlc/bin.win32/plugins DESTINATION .)
    add_custom_target(
        vlc ALL DEPENDS
        ${CMAKE_BINARY_DIR}/libvlc.dll
        ${CMAKE_BINARY_DIR}/libvlccore.dll
        ${CMAKE_BINARY_DIR}/plugins
        VERBATIM)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    include_directories(SYSTEM ext/vlc/include)

    file(COPY ext/vlc/bin.macx/lib DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.dylib
        COMMAND ln -sf libvlc.5.dylib ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.dylib
        DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.5.dylib
        VERBATIM)

    file(COPY ext/vlc/bin.macx/plugins DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlccore.dylib
        COMMAND ln -sf libvlccore.8.dylib ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlccore.dylib
        DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlccore.8.dylib
        VERBATIM)

    add_custom_target(
        vlc ALL DEPENDS
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.dylib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlccore.dylib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins
        VERBATIM)
endif()

#######################################
# Compile PUPnP
include_directories(SYSTEM ext/pupnp/ixml/inc ext/pupnp/upnp/inc)

# pupnp libraries
file(GLOB_RECURSE PUPNP_IXML_SRC_LIST ext/pupnp/ixml/src/*.c)
file(GLOB_RECURSE PUPNP_THREADUTIL_SRC_LIST ext/pupnp/threadutil/src/*.c)
file(GLOB_RECURSE PUPNP_UPNP_SRC_LIST ext/pupnp/upnp/src/*.c)

set_source_files_properties(
    ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=gnu99 -DUPNP_STATIC_LIB")

add_library(
    pupnp STATIC
    ${PUPNP_IXML_SRC_LIST}
    ${PUPNP_THREADUTIL_SRC_LIST}
    ${PUPNP_UPNP_SRC_LIST})

include_directories(
    PRIVATE
    ext/pupnp/
    ext/pupnp/ixml/src/inc
    ext/pupnp/threadutil/inc
    ext/pupnp/threadutil/src/inc
    ext/pupnp/upnp/src/inc)

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
file(GLOB_RECURSE SRC_LIST src/*.cpp)

set_source_files_properties(
    ${SRC_LIST}
    PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -DUPNP_STATIC_LIB -DVERSION=\\\"${VERSION}\\\" -DPLATFORM=\\\"${CMAKE_SYSTEM_NAME}\\\"")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set_property(SOURCE ${SRC_LIST} APPEND_STRING PROPERTY COMPILE_FLAGS " -Werror")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    SET(SERVER_RESOURCES ${CMAKE_SOURCE_DIR}/deploy/win32/lximediaserver.rc)
else()
    SET(SERVER_RESOURCES)
endif()

add_executable(lximediaserver ${SRC_LIST} ${SERVER_RESOURCES})

include_directories(
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(lximediaserver pupnp jpge miniz sha1)
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries(lximediaserver uuid vlc pthread)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set_target_properties(lximediaserver PROPERTIES LINK_FLAGS "-liconv -framework Carbon ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.dylib")
    add_dependencies(lximediaserver vlc)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximediaserver ws2_32 iphlpapi rpcrt4 shell32)
    set_target_properties(lximediaserver PROPERTIES COMPILE_FLAGS "-mwindows")
    set_target_properties(lximediaserver PROPERTIES LINK_FLAGS "-mwindows libvlc.dll")
    add_dependencies(lximediaserver vlc)
endif()

#######################################
# test
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    file(GLOB_RECURSE TEST_SRC_LIST test/*.cpp)

    set_source_files_properties(
        ${TEST_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Werror")

    add_executable(lximediaserver_test ${TEST_SRC_LIST})

    include_directories(
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/test)

    target_link_libraries(lximediaserver_test pupnp)
    if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
        target_link_libraries(lximediaserver_test uuid vlc pthread)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        set_target_properties(lximediaserver_test PROPERTIES LINK_FLAGS "-liconv ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/libvlc.dylib")
        add_dependencies(lximediaserver_test vlc)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
        target_link_libraries(lximediaserver_test ws2_32 iphlpapi rpcrt4)
        set_target_properties(lximediaserver_test PROPERTIES COMPILE_FLAGS "-mconsole")
        set_target_properties(lximediaserver_test PROPERTIES LINK_FLAGS "-mconsole libvlc.dll")
        add_dependencies(lximediaserver_test vlc)
    endif()
endif()

#######################################
# install
install(TARGETS lximediaserver RUNTIME DESTINATION bin)

