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


source ${TOOLBOSCORE_ROOT}/include/Unittest.bash


CWD=$(pwd)

# if [[ "${CIA}" != "TRUE" ]]
# then
#     # All these tests require BBDMs,... not present during Nightly Build
#     #
#     source ${SIT}/Libraries/BPLBase/7.1/BashSrc
#     source ${SIT}/Modules/BBDM/BBDMArrayBlockF32/1.7/BashSrc
#     source ${SIT}/Modules/BBDM/BBDMBaseI32/1.7/BashSrc
#     source ${SIT}/Modules/BBDM/BBDMBlockF32/1.7/BashSrc
#     source ${SIT}/Modules/BBDM/BBDMBaseF32/1.7/BashSrc
#     source ${SIT}/Modules/BBDM/BBDMMemI8/1.7/BashSrc
#
#     cd ${CWD}/General        && runTest ${MAKEFILE_PLATFORM}/TestCoreLibrary
# fi


cd ${CWD}/IOChannel      && runTest ${MAKEFILE_PLATFORM}/TestIOChannel
cd ${CWD}/ListsAndQueues && runTest ${MAKEFILE_PLATFORM}/TestListsAndQueues
cd ${CWD}/Multithreading && runTest ${MAKEFILE_PLATFORM}/TestMultithreading


# EOF
