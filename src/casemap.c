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
#include "casemap.h"

char sqchat_rfc1459_tolower(const char c) {
    return c >= 'A' && c <= '^' ? c + 32 : c;
}

char sqchat_rfc1459_toupper(const char c) {
    return c >= 'a' && c <= '~' ? c - 32 : c;
}

int sqchat_rfc1459_strcasecmp(const char * s1, const char * s2) {
    const char * c1 = s1;
    const char * c2 = s2;
    int result;

    if (c1 == c2)
        return 0;

    while ((result = sqchat_rfc1459_tolower(*c1) - sqchat_rfc1459_tolower(*c2++)) == 0)
        if (*c1++ == '\0')
            break;

    return result;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
