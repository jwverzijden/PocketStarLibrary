/*
PS_FontDefinitions.h
Last modified 02 Dec 2018

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

#ifndef PS_FontDefs_h
#define PS_FontDefs_h

typedef struct {
    const uint8_t       width;
    const uint16_t      offset;
} PSCharInfo;

typedef struct {
    const uint8_t       height;
    const char          startChar;
    const char          endChar;
    const PSCharInfo    *charInfo;
    const uint8_t       *bitmap;
} PSFont;

#endif