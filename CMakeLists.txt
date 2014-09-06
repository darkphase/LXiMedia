project(lximedia)
cmake_minimum_required(VERSION 2.8)
file(STRINGS "VERSION" VERSION)

#######################################
# Extract VLC
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(VLC_VERSION "2.1.5-win32")
    execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/vlc/vlc.sh ${CMAKE_SOURCE_DIR}/ext/vlc/ ./ext/vlc/ ${VLC_VERSION})

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libvlc.dll COMMAND cp ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libvlccore.dll COMMAND cp ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlccore.dll ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/plugins COMMAND cp -R ${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/plugins ${CMAKE_BINARY_DIR} VERBATIM)
    add_custom_target(vlc ALL DEPENDS ${CMAKE_BINARY_DIR}/libvlc.dll ${CMAKE_BINARY_DIR}/libvlccore.dll ${CMAKE_BINARY_DIR}/plugins VERBATIM)
endif()

#######################################
# Extract PUPnP
set(PUPNP_VERSION "1.6.19")
execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/ext/pupnp/pupnp.sh ${CMAKE_SOURCE_DIR}/ext/pupnp/ ./ext/pupnp/ ${PUPNP_VERSION})

# pupnp libraries
file(GLOB_RECURSE PUPNP_IXML_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/*.c)
file(GLOB_RECURSE PUPNP_THREADUTIL_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/*.c)
file(GLOB_RECURSE PUPNP_UPNP_SRC_LIST ${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/*.c)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set_source_files_properties(
        ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-pthread -DWIN32 -DUPNP_STATIC_LIB -static -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION} -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/inc")
else()
    set_source_files_properties(
        ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-pthread -DUPNP_STATIC_LIB -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION} -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/src/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/threadutil/src/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/src/inc")
endif()

add_library(pupnp STATIC ${PUPNP_IXML_SRC_LIST} ${PUPNP_THREADUTIL_SRC_LIST} ${PUPNP_UPNP_SRC_LIST})

#######################################
# lximediacenter
aux_source_directory(src/lximcbackend/platform/ LXIMCBACKEND_PLATFORM_SRC_LIST)
aux_source_directory(src/lximcbackend/pupnp/ LXIMCBACKEND_PUPNP_SRC_LIST)
aux_source_directory(src/lximcbackend/vlc/ LXIMCBACKEND_VLC_SRC_LIST)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set_source_files_properties(
        ${LXIMCBACKEND_PLATFORM_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DWIN32 -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/")

    set_source_files_properties(
        ${LXIMCBACKEND_PUPNP_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DWIN32 -DUPNP_STATIC_LIB -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc")

    set_source_files_properties(
        ${LXIMCBACKEND_VLC_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DWIN32 -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/include/")
else()
    set_source_files_properties(
        ${LXIMCBACKEND_PLATFORM_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -I${CMAKE_SOURCE_DIR}/src/lximcbackend/")

    set_source_files_properties(
        ${LXIMCBACKEND_PUPNP_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DUPNP_STATIC_LIB -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/ixml/inc -I${CMAKE_BINARY_DIR}/ext/pupnp/libupnp-${PUPNP_VERSION}/upnp/inc")

    set_source_files_properties(
        ${LXIMCBACKEND_VLC_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -I${CMAKE_SOURCE_DIR}/src/lximcbackend/")
endif()

add_library(lximediacenter STATIC ${LXIMCBACKEND_PLATFORM_SRC_LIST} ${LXIMCBACKEND_PUPNP_SRC_LIST} ${LXIMCBACKEND_VLC_SRC_LIST})

#######################################
# lximcbackend
aux_source_directory(src/lximcbackend/backend/ LXIMCBACKEND_SRC_LIST)
aux_source_directory(src/lximcbackend/backend/html/ LXIMCBACKEND_HTML_SRC_LIST)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set_source_files_properties(
        ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_HTML_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DVERSION=\\\"${VERSION}\\\" -DWIN32 -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_SOURCE_DIR}/src/lximcbackend/backend/")
else()
    set_source_files_properties(
        ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_HTML_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DVERSION=\\\"${VERSION}\\\" -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_SOURCE_DIR}/src/lximcbackend/backend/")
endif()

add_executable(lximcbackend ${LXIMCBACKEND_SRC_LIST} ${LXIMCBACKEND_HTML_SRC_LIST})

target_link_libraries(lximcbackend lximediacenter pupnp pthread)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend ws2_32 iphlpapi rpcrt4)
    set_target_properties(lximcbackend PROPERTIES LINK_FLAGS "${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
else()
    target_link_libraries(lximcbackend uuid vlc)
endif()

# lximcbackend_test
aux_source_directory(test/lximcbackend/ LXIMCBACKEND_TEST_SRC_LIST)
aux_source_directory(test/lximcbackend/vlc/ LXIMCBACKEND_VLC_TEST_SRC_LIST)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set_source_files_properties(
        ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DWIN32 -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_SOURCE_DIR}/src/lximcbackend/backend/ -I${CMAKE_SOURCE_DIR}/test/lximcbackend/ -I${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/include/")
else()
    set_source_files_properties(
        ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -I${CMAKE_SOURCE_DIR}/src/lximcbackend/ -I${CMAKE_SOURCE_DIR}/src/lximcbackend/backend/ -I${CMAKE_SOURCE_DIR}/test/lximcbackend/")
endif()

add_executable(lximcbackend_test ${LXIMCBACKEND_TEST_SRC_LIST} ${LXIMCBACKEND_VLC_TEST_SRC_LIST})

target_link_libraries(lximcbackend_test pupnp pthread)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximcbackend_test ws2_32 iphlpapi rpcrt4)
    set_target_properties(lximcbackend_test PROPERTIES LINK_FLAGS "${CMAKE_BINARY_DIR}/ext/vlc/vlc-${VLC_VERSION}/libvlc.dll")
else()
    target_link_libraries(lximcbackend_test uuid vlc)
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
#include_directories(src/lximcbackend/launcher/)
aux_source_directory(src/lximcbackend/launcher/ LXIMCLAUNCHER_SRC_LIST)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set_source_files_properties(
        ${LXIMCLAUNCHER_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DVERSION=\\\"${VERSION}\\\" -DWIN32 -static -I${CMAKE_SOURCE_DIR}/src/lximcbackend/")
else()
    set_source_files_properties(
        ${LXIMCLAUNCHER_SRC_LIST}
        PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pthread -DVERSION=\\\"${VERSION}\\\" -I${CMAKE_SOURCE_DIR}/src/lximcbackend/")
endif()

add_executable(lximclauncher ${LXIMCLAUNCHER_SRC_LIST})

target_link_libraries(lximclauncher lximediacenter pupnp pthread)
if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(lximclauncher ws2_32 iphlpapi)
else()
    target_link_libraries(lximclauncher)
endif()

#######################################
# install
install(TARGETS lximcbackend lximclauncher
        RUNTIME DESTINATION bin)
