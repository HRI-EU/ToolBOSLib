# -*- coding: utf-8 -*-
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


from ToolBOSCore.Platforms.Platforms import getPlatformNames

name             = 'ToolBOSLib-GPLv3'            # also hardcoded in ToolBOSLib.h !

version          = '4.0'                         # also hardcoded in ToolBOSLib.h !

category         = 'Libraries'

install          = [ 'external' ]

delete = []

for platform in getPlatformNames():
    delete.append( 'test/AnyString/'            + platform )
    delete.append( 'test/BasicFunctions/'       + platform )
    delete.append( 'test/ESC/'                  + platform )
    delete.append( 'test/IOChannelLifecycle/'   + platform )
    delete.append( 'test/IOChannelMain/'        + platform )
    delete.append( 'test/ListsAndQueues/'       + platform )
    delete.append( 'test/Multithreading/'       + platform )
    delete.append( 'test/SerializeGeneral/'     + platform )
    delete.append( 'test/SerializeHeaderRead/'  + platform )
    delete.append( 'test/SerializeHeaderWrite/' + platform )
    delete.append( 'test/WorkQueue/'            + platform )

usePatchlevels   = True

patchlevel       = 7

sqLevel          = 'advanced'

sqComments       = { 'C03': 'many macro names are historic and cannot be changed without touching a lot of dependent packages\n\nsome macros are lowercase on purpose to replace non-existent functions on certain platforms' }

copyright        = [ 'Copyright (c) Honda Research Institute Europe GmbH',
                     'This file is part of ToolBOSLib.',
                     'ToolBOSLib is free software: you can redistribute it and/or modify',
                     'it under the terms of the GNU General Public License as published by',
                     'the Free Software Foundation, either version 3 of the License, or',
                     '(at your option) any later version.',
                     'ToolBOSLib is distributed in the hope that it will be useful,',
                     'but WITHOUT ANY WARRANTY; without even the implied warranty of',
                     'MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the',
                     'GNU General Public License for more details.',
                     'You should have received a copy of the GNU General Public License',
                     'along with ToolBOSLib. If not, see <http://www.gnu.org/licenses/>.' ]


# EOF
