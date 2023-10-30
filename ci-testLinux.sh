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


# BashSrc files need to be ready first, see TBCORE-2267
# set -euxo pipefail

if [[ "$#" -eq 0 ]] # no argument supplied, using default
then
    TOOLBOSCORE_VERSION="4.2"
else
    TOOLBOSCORE_VERSION="$1"
fi

# shellcheck source=/hri/sit/latest/DevelopmentTools/ToolBOSCore/4.2/BashSrc
source "/hri/sit/latest/DevelopmentTools/ToolBOSCore/${TOOLBOSCORE_VERSION}/BashSrc"
source "${SIT}/External/anaconda3/envs/common/3.9/BashSrc"

BST.py --test


# EOF
