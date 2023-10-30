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


set -euxo pipefail

if [[ "$#" -eq 0 ]] # no argument supplied, using default
then
    TOOLBOSCORE_VERSION="4.2"
else
    TOOLBOSCORE_VERSION="$1"
fi

# shellcheck source=/hri/sit/latest/DevelopmentTools/ToolBOSCore/4.2/BashSrc
source "/hri/sit/latest/DevelopmentTools/ToolBOSCore/${TOOLBOSCORE_VERSION}/BashSrc"


test -f lib/windows-amd64-vs2017/libToolBOSLib-GPLv3.4.0.lib
test -f lib/windows-amd64-vs2017/ToolBOSLib-GPLv3.4.0.dll
test -f lib/windows-amd64-vs2017/ToolBOSLib-GPLv3.4.0.exp
test -f lib/windows-amd64-vs2017/ToolBOSLib-GPLv3.4.0.lib
test -f lib/windows-amd64-vs2017/ToolBOSLib-GPLv3.def

test -L lib/windows-amd64-vs2017/libToolBOSLib-GPLv3.lib
test -L lib/windows-amd64-vs2017/ToolBOSLib-GPLv3.lib

test -f test/AnyString/windows-amd64-vs2017/TestAnyString.exe
test -f test/BasicFunctions/windows-amd64-vs2017/TestBasicFunctions.exe
test -f test/ESC/windows-amd64-vs2017/TestESC.exe
test -f test/IOChannelLifecycle/windows-amd64-vs2017/TestIOChannelLifecycle.exe
test -f test/IOChannelMain/windows-amd64-vs2017/TestIOChannelMain.exe
test -f test/ListsAndQueues/windows-amd64-vs2017/TestListsAndQueues.exe
test -f test/Multithreading/windows-amd64-vs2017/TestMultithreading.exe
test -f test/SerializeHeaderRead/windows-amd64-vs2017/TestSerializeHeaderRead.exe
test -f test/SerializeHeaderWrite/windows-amd64-vs2017/TestSerializeHeaderWrite.exe
test -f test/WorkQueue/windows-amd64-vs2017/TestWorkQueue.exe


# EOF
