cmake_minimum_required(VERSION 3.8)
project(lib)

#1 set
set(PROJECTNAME lib)
set(CMAKE_CXX_STANDARD 11)
#set(3RDPARTY_PATH /usr/src/3rdparty/)
#set(PRO_PATH /usr/src/src/lib/)

#2 sorce file path
#AUX_SOURCE_DIRECTORY(${PRO_PATH}/src/ SRC_LIST)
AUX_SOURCE_DIRECTORY(. SRC_LIST)

#3 out path
#SET(LIBRARY_OUTPUT_PATH ./)
#SET(EXECUTABLE_OUTPUT_PATH  ${PRO_PATH}/bin)

#4 definitions
#ADD_DEFINITIONS( -g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
#        -DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
#        -DAC_HAS_DEBUG -DLINUX_DAEMON -DLINUX2 -DHAS_SQLITE)

#5 include and lib file path
#INCLUDE_DIRECTORIES(./ ${3RDPARTY_PATH}/include/acl-3.2.2/ ${3RDPARTY_PATH}/include/curl-7.50.1/)
#LINK_DIRECTORIES(./ ${3RDPARTY_PATH}/lib/acl-3.2.2/ ${3RDPARTY_PATH}/lib/curl-7.50.1/)

#6 add_xxxx
ADD_EXECUTABLE(${PROJECTNAME} ${SRC_LIST})

#7 target_link_librareies
#TARGET_LINK_LIBRARIES(${PROJECTNAME} lib_fiber_cpp.a lib_fiber.a lib_acl.a libcurl.a sqlite3 pthread dl rt z)
