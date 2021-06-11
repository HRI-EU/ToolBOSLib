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


if [[ "${CIA}" != "TRUE" ]]
then
    # All these tests require BBDMs,... not present during Nightly Build
    source ${SIT}/Modules/BBDM/BBDMAll/1.7/BashSrc

    cd ${CWD}/JSON             && ls -lh && runTest ./TestSerializeToJSON.py
    cd ${CWD}/Overhead         && runTest ${MAKEFILE_PLATFORM}/TestOverhead
    cd ${CWD}/SerializeUtility && runTest ./TestSerializeUtility.py
fi


# Depending on $CIA the test programs are built with -lBBDM... or not.
# That's why we need to have them sourced above if built that way.


cd ${CWD}/General        && runTest ${MAKEFILE_PLATFORM}/TestSerialize
cd ${CWD}/HeaderV2_Write && runTest ${MAKEFILE_PLATFORM}/TestHeaderV2_Write # writes /tmp/SerializeTestV20.txt
cd ${CWD}/HeaderV2_Read  && runTest ${MAKEFILE_PLATFORM}/TestHeaderV2_Read  # reads  /tmp/SerializeTestV20.txt


# EOF
