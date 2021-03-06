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

name             = 'ToolBOSLib-GPLv3'

version          = '4.0'

category         = 'Libraries'

install          = [ 'external' ]

delete = []

for platform in getPlatformNames():
    delete.append( 'test/ESC/'                        + platform )
    delete.append( 'test/General/'                    + platform )
    delete.append( 'test/IOChannel/'                  + platform )
    delete.append( 'test/ListsAndQueues/'             + platform )
    delete.append( 'test/Multithreading/'             + platform )
    delete.append( 'test/Serialize/General/'          + platform )
    delete.append( 'test/Serialize/HeaderV2_Read/'    + platform )
    delete.append( 'test/Serialize/HeaderV2_Write/'   + platform )
    delete.append( 'test/Serialize/Overhead/'         + platform )
    delete.append( 'test/WorkQueue/'                  + platform )

usePatchlevels   = True

patchlevel       = 10

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
