/* Copyright 2013-2017 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_COLOR_H
#define UTILS_COLOR_H

#define ANSI_BLACK    "\033[0;30m"
#define ANSI_RED      "\033[0;31m"
#define ANSI_GREEN    "\033[0;32m"
#define ANSI_YELLOW   "\033[0;33m"
#define ANSI_BLUE     "\033[0;34m"
#define ANSI_MAGENTA  "\033[0;35m"
#define ANSI_CYAN     "\033[0;36m"
#define ANSI_GREY     "\033[0;37m"
#define ANSI_DARKGREY "\033[01;30m"
#define ANSI_BRED     "\033[01;31m"
#define ANSI_BGREEN   "\033[01;32m"
#define ANSI_BYELLOW  "\033[01;33m"
#define ANSI_BBLUE    "\033[01;34m"
#define ANSI_BMAGENTA "\033[01;35m"
#define ANSI_BCYAN    "\033[01;36m"
#define ANSI_WHITE    "\033[01;37m"
#define ANSI_NORMAL   "\033[0m"

// erase whole screen
#define ANSI_CLEAR "\033[2J"

// erase current line
#define ANSI_CLEAR_LINE "\033[2K"

/*
\033[y;xH   move cursor to column x row y
\033[H      move cursor to left top corner
\033[xA     move cursor x rows up
\033[xB     move cursor x rows down
\033[xD     move cursor x columns left
\033[xC     move cursor x columns right
\033[s      save cursor position
\033[u      restore cursor position
\0337       save cursor position and text attributes
\0338       restore cursor position and text attributes
*/

#define COL_TIME ANSI_CYAN
#define COL_VERBOSE ANSI_BYELLOW

#define COL_SEMA ANSI_RED

// for AST printing
#define COL_DECL  ANSI_BGREEN
#define COL_STMT  ANSI_BMAGENTA
#define COL_EXPR  ANSI_BMAGENTA
#define COL_TYPE  ANSI_GREEN
#define COL_CANON ANSI_YELLOW
#define COL_VALUE ANSI_BCYAN
#define COL_ATTR  ANSI_BLUE
#define COL_ATTRIBUTES ANSI_YELLOW
#define COL_NORM  ANSI_NORMAL

#endif

