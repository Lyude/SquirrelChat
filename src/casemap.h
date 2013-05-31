/* Provides case comparison and conversion functions for rfc1459 casemapping
 * Copyright (C) 2013 Stephen Chandler Paul
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CASEMAP_H__
#define __CASEMAP_H__

extern char rfc1459_tolower(const char c);
extern char rfc1459_toupper(const char c);
extern int rfc1459_strcasecmp(const char * s1, const char * s2);

#endif // __CASEMAP_H__
