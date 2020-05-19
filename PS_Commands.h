/*
PS_Commands.h
Last modified 10 Jul 2018

This file is part of the PocketStar Library.

Copyright (C) 2018  Florian Keller

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PS_Commands_h
#define PS_Commands_h

#define COMMAND_SET_COLOUMN     0x15
#define COMMAND_DRAW_LINE       0x21
#define COMMAND_DRAW_RECTANGLE  0x22
#define COMMAND_CLEAR_WINDOW    0x25
#define COMMAND_SET_FILL        0x26
#define COMMAND_SET_ROW         0x75
#define COMMAND_MASTER_CURRENT  0x87 // brightness
#define COMMAND_SET_REMAP       0xA0
#define COMMAND_DISPLAY_OFF     0xAE
#define COMMAND_DISPLAY_ON      0xAF

#endif