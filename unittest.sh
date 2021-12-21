#!/bin/bash
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
# Setup
#----------------------------------------------------------------------------


# shellcheck source=../ToolBOSCore/include/Unittest.bash
source "${TOOLBOSCORE_ROOT}/include/Unittest.bash"

source ${SIT}/External/cutest/1.5/BashSrc

CWD=$(pwd)

export LD_LIBRARY_PATH=${CWD}/lib/${MAKEFILE_PLATFORM}:${LD_LIBRARY_PATH}


#----------------------------------------------------------------------------
# Unittests
#----------------------------------------------------------------------------


cd "${CWD}/test/BasicFunctions"       && runTest "${MAKEFILE_PLATFORM}/TestBasicFunctions"
cd "${CWD}/test/ESC"                  && runTest "${MAKEFILE_PLATFORM}/TestESC"
cd "${CWD}/test/IOChannelLifecycle"   && runTest "${MAKEFILE_PLATFORM}/TestIOChannelLifecycle"
cd "${CWD}/test/IOChannelMain"        && runTest "${MAKEFILE_PLATFORM}/TestIOChannelMain"
cd "${CWD}/test/ListsAndQueues"       && runTest "${MAKEFILE_PLATFORM}/TestListsAndQueues"
cd "${CWD}/test/Multithreading"       && runTest "${MAKEFILE_PLATFORM}/TestMultithreading"
cd "${CWD}/test/SerializeHeaderWrite" && runTest "${MAKEFILE_PLATFORM}/TestSerializeHeaderWrite"

# needs restructuring into proper unittest
# cd "${CWD}/test/AnyString"            && runTest "${MAKEFILE_PLATFORM}/TestAnyString"

# aux. file SerializeTestV20.txt missing in repo
# cd "${CWD}/test/SerializeHeaderRead"  && runTest "${MAKEFILE_PLATFORM}/TestSerializeHeaderRead"

# stops with: TestWorkQueue.cpp:167 ANY_REQUIRE( istatus ) failed!
# cd "${CWD}/test/WorkQueue"            && runTest "${MAKEFILE_PLATFORM}/TestWorkQueue"


# we managed to get here --> success
exit 0


# EOF
