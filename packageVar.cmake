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
# Dependencies
#----------------------------------------------------------------------------


# please include here the packageVar.cmake files of the packages this one
# depends on, e.g:
# bst_find_package(Libraries/ToolBOSLib/3.0)


#----------------------------------------------------------------------------
# Build specification
#----------------------------------------------------------------------------


# locations of headerfiles, e.g.:
# include_directories($ENV{SIT}/Libraries/ToolBOSLib/3.0/include)

include_directories($ENV{SIT}/Libraries/ToolBOSLib-GPLv3/4.0/include)
include_directories($ENV{SIT}/Libraries/ToolBOSLib-GPLv3/4.0/include/$ENV{MAKEFILE_PLATFORM})


# locations of libraries, e.g.:
# link_directories($ENV{SIT}/Libraries/ToolBOSLib/3.0/lib/$ENV{MAKEFILE_PLATFORM})

link_directories($ENV{SIT}/Libraries/ToolBOSLib-GPLv3/4.0/lib/$ENV{MAKEFILE_PLATFORM})


# libraries to link (shared libs without "lib" prefix and filename extension), e.g.:
# list(APPEND BST_LIBRARIES_SHARED ToolBOSCore)
# list(APPEND BST_LIBRARIES_STATIC $ENV{SIT}/path/to/libFoo.a)

if(UNIX)
    list(APPEND BST_LIBRARIES_SHARED pthread)

    # must occur before libToolBOSCore.a to get correct order for static linking,
    # note that the list finally will be reversed
    list(APPEND BST_LIBRARIES_STATIC -ldl -lrt -lpthread -lbfd)
endif()


list(APPEND BST_LIBRARIES_SHARED ToolBOSLib-GPLv3)
list(APPEND BST_LIBRARIES_STATIC $ENV{SIT}/Libraries/ToolBOSLib-GPLv3/4.0/lib/$ENV{MAKEFILE_PLATFORM}/libToolBOSLib-GPLv3${CMAKE_STATIC_LIBRARY_SUFFIX})

if(WIN32)
    list(APPEND BST_LIBRARIES_SHARED ws2_32.lib wsock32.lib shlwapi.lib
                                     winmm.lib psapi.lib)

    list(APPEND BST_LIBRARIES_STATIC ws2_32.lib wsock32.lib shlwapi.lib
                                     winmm.lib psapi.lib)
endif()

add_definitions(-D_POSIX_C_SOURCE=199506L -D__USE_MISC -DHAVE_BFD_DEMANGLE
                -D__USE_XOPEN -D__USE_GNU -D_GNU_SOURCE -D_XOPEN_SOURCE=600)

# the C90 standard recommends that C++ does not have INT_MAX etc. by default
# but we use them to define e.g. BASEI32_MAX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS")


# EOF

