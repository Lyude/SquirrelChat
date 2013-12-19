/* Limits use of __attribute__ to supporting compilers
 * (Included in every file for SquirrelChat by default)
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
#ifndef __SQ_ATTRIBUTES_H__

#ifdef __GNUC__
#define _attr_nonnull(...)              __attribute__((nonnull(__VA_ARGS__)))
#define _attr_format(type, str, start)	__attribute__((format(type, str, start)))
#define _attr_malloc			__attribute__((malloc()))
#else
#define _attr_nonnull(...)
#define _attr_format(type, str, start)
#define _attr_malloc
#endif

#endif // __SQ_ATTRIBUTES_H__
