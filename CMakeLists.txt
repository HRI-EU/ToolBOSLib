#
#  Copyright (c) Honda Research Institute Europe GmbH
#
#  This file is part of ToolBOSLib.
#
#  ToolBOSLib is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ToolBOSLib is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with ToolBOSLib. If not, see <http://www.gnu.org/licenses/>.
#
#


#----------------------------------------------------------------------------
# Standard header + common routines
#----------------------------------------------------------------------------


cmake_minimum_required(VERSION 2.8)

project(ToolBOSLib)

find_package(BuildSystemTools)

set(BST_INSTALL_CATEGORY Libraries)


#----------------------------------------------------------------------------
# Dependencies
#----------------------------------------------------------------------------


bst_find_package(External/msinttypes/1.0)
bst_find_package(External/pthreads4w/1.0)

# do not include cutest here, otherwise the library will link against it
# although not needed


#----------------------------------------------------------------------------
# Build specification
#----------------------------------------------------------------------------


# The package binutils 2.34 (e.g. in Ubuntu 20.04) received an API change,
# see TBCORE-2160 for details.

if("$ENV{MAKEFILE_PLATFORM}" STREQUAL "focal64")
    add_definitions(-DNEW_BFD_API)
endif()


add_definitions(-DTOOLBOS_MAJVERSION=${TARGET_VERSION_MAJOR}
                -DTOOLBOS_MINVERSION=${TARGET_VERSION_MINOR}
                -DHAVE_BFD_DEMANGLE
                -D_DEFAULT_SOURCE)

# non-standard location for ToolBOS.conf.h
include_directories($ENV{TOOLBOSCORE_ROOT}/etc)


# the C90 standard recommends that C++ does not have INT_MAX etc. by default
# but we use them to define e.g. BASEI32_MAX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS")


file(GLOB SRC_FILES src/*.c)


if(UNIX OR "$ENV{MAKEFILE_PLATFORM}" MATCHES "^mingw")
    add_definitions(-D_POSIX_C_SOURCE=199506L -D__USE_MISC
                    -D__USE_XOPEN -D__USE_GNU -D_GNU_SOURCE -D_XOPEN_SOURCE=600)

    if("$ENV{MAKEFILE_PLATFORM}" MATCHES "^mingw" )
        list(APPEND BST_LIBRARIES_SHARED ws2_32.lib wsock32.lib shlwapi.lib winmm.lib psapi.lib)
    else()
        list(APPEND BST_LIBRARIES_SHARED bfd dl iberty m pthread rt z)
    endif()

else()
    list(APPEND SRC_FILES ${PTHREADS4W_SRCDIR}/pthread.c)

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /D_WIN32_WINNT=0x400 /DHAVE_CONFIG_H /D__CLEANUP_C /DWIN32 /DNDEBUG /D_WINDOWS /D_USRDLL /D__PTW32_STATIC_LIB /DPTW32_STATIC_LIB -I${TOOLBOSCORE_PTHREADS_DIRECTORY}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINNT=0x400 /DHAVE_CONFIG_H /D__CLEANUP_C /DWIN32 /DNDEBUG /D_WINDOWS /D_USRDLL /D__PTW32_STATIC_LIB /DPTW32_STATIC_LIB -I${TOOLBOSCORE_PTHREADS_DIRECTORY}")

    list(APPEND BST_LIBRARIES_SHARED ws2_32.lib wsock32.lib shlwapi.lib winmm.lib psapi.lib)

endif()

bst_build_libraries("${SRC_FILES}" "${PROJECT_NAME}" "${BST_LIBRARIES_SHARED}")


file(GLOB EXE_FILES examples/*.c examples/*.cpp)

# TBCORE-2269  examples/AnyLogCPP.cpp is known not to compile for Windows
if("$ENV{MAKEFILE_PLATFORM}" STREQUAL "windows-amd64-vs2017")
    LIST(REMOVE_ITEM EXE_FILES "${CMAKE_HOME_DIRECTORY}/examples/AnyLogCPP.cpp")
endif()

bst_build_executables("${EXE_FILES}" "${BST_LIBRARIES_SHARED}")


if(EXISTS "$ENV{SIT}/External/cutest/1.5/lib/$ENV{MAKEFILE_PLATFORM}")

    bst_find_package(External/cutest/1.5)

    message("building tests for platform=$ENV{MAKEFILE_PLATFORM}")

    set(EXE_FILES ${CMAKE_HOME_DIRECTORY}/test/AnyString/TestAnyString.c
                  ${CMAKE_HOME_DIRECTORY}/test/BasicFunctions/TestBasicFunctions.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/ESC/TestESC.c
                  ${CMAKE_HOME_DIRECTORY}/test/IOChannelLifecycle/TestIOChannelLifecycle.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/IOChannelMain/TestIOChannelMain.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/ListsAndQueues/TestListsAndQueues.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/Multithreading/TestMultithreading.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/SerializeHeaderRead/TestSerializeHeaderRead.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/SerializeHeaderWrite/TestSerializeHeaderWrite.cpp
                  ${CMAKE_HOME_DIRECTORY}/test/WorkQueue/TestWorkQueue.cpp)

    bst_build_executables("${EXE_FILES}" "${BST_LIBRARIES_SHARED}")

else()

    message("skipping tests (CuTest not found for $ENV{MAKEFILE_PLATFORM})")

endif()


# EOF
