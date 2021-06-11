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

CWD=$(pwd)

export LD_LIBRARY_PATH=${CWD}/lib/${MAKEFILE_PLATFORM}:${LD_LIBRARY_PATH}


#----------------------------------------------------------------------------
# Unittests
#----------------------------------------------------------------------------


# ensure package had been built
BST.py -b -j32

cd "${CWD}/test"           && runTest "./TestCoreLibrary.sh"
cd "${CWD}/IOChannel"      && runTest "${MAKEFILE_PLATFORM}/TestIOChannel"
cd "${CWD}/ListsAndQueues" && runTest "${MAKEFILE_PLATFORM}/TestListsAndQueues"
cd "${CWD}/Multithreading" && runTest "${MAKEFILE_PLATFORM}/TestMultithreading"
cd "${CWD}/test/Serialize" && runTest "./TestSerialize.sh"


# we managed to get here --> success
exit 0


# EOF
