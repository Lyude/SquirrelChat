/*
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
 *
 * This file incorporates work covered by the following copyright and permission
 * notice:
 *
 * 	Copyright (C) 2013 Alex Iadicicco
 *
 *	Redistribution and use in source and binary forms are permitted provided
 *	that the above copyright notice and this paragraph are duplicated in all
 *	such forms and that any documentation, advertising materials, and any
 *	other materials related to such distribution and use acknowledge that
 *	the software was developed by Alexander Iadicicco. The name of Alexander
 *	Iadicicco may not be used to endorse or promote products derived from
 *	this software without specific prior written permission. THIS SOFTWARE
 *	IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 *	AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * The project the incorporated work originates from may be found here:
 * <https://github.com/aji/ircd-micro>
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"
#include "casemap.h"

static void __null_canonize(char * s) { }

static trie_e *trie_e_new(trie_e * up)
{
    trie_e *e;
    int i;

    e = malloc(sizeof(*e));
    e->val = NULL;
    e->up = up;
    for (i=0; i<16; i++)
        e->n[i] = NULL;

    return e;
}

static void trie_e_del(trie_e *e)
{
    free(e);
}

trie *trie_new(void (*canonize)())
{
    trie *trie;
    int i;

    trie = malloc(sizeof(*trie));
    trie->canonize = canonize ? canonize : __null_canonize;

    trie->n.val = NULL;
    trie->n.up = NULL;
    for (i=0; i<16; i++)
        trie->n.n[i] = 0;

    return trie;
}

static char nibble(char * s, int i)
{
    return (i%2==0) ? s[i/2]>>4 : s[i/2]&0xf;
}

static trie_e *retrieval(trie * trie, const char * tkey, int create)
{
    trie_e *n;
    char key[U_TRIE_KEY_MAX];
    unsigned int c, nib = 0; /* even=high, odd=low */

    strncpy(key, tkey, U_TRIE_KEY_MAX);

    trie->canonize(key);

    if (!key[0])
        return NULL;

    n = &trie->n;

    for (; n && key[nib/2]; nib++) {
        c = nibble(key, nib);
        if (n->n[c] == NULL) {
            if (create)
                n->n[c] = trie_e_new(n);
            else
                return NULL;
        }
        n = n->n[c];
    }

    return n;
}

void trie_set(trie * trie, const char * key, void * val)
{
    trie_e *n = retrieval(trie, key, 1);
    n->val = val;
}

void *trie_get(trie * trie, const char * key)
{
    trie_e *n = retrieval(trie, key, 0);
    return n ? n->val : NULL;
}

static void each(trie_e *e, void (*cb)(), void *priv)
{
    int i;

    if (e->val != NULL)
        cb(e->val, priv);

    for (i=0; i<16; i++) {
        if (e->n[i] != NULL)
            each(e->n[i], cb, priv);
    }
}

void trie_each(trie *trie, void(*cb)(), void * priv)
{
    each(&trie->n, cb, priv);
}

void *trie_del(trie * trie, const char * key)
{
    trie_e *prev, *cur;
    void *val;
    int i, nempty;

    cur = retrieval(trie, key, 0);

    if (cur == NULL)
        return NULL;

    val = cur->val;

    cur->val = NULL;
    prev = NULL;
    while (cur) {
        nempty = 0;
        for (i=0; i<16; i++) {
            if (cur->n[i] == prev)
                cur->n[i] = NULL;
            if (!cur->n[i])
                nempty++;
        }
        prev = cur;
        if (nempty != 16 || cur->val || cur == &trie->n)
            break;
        cur = cur->up;
        trie_e_del(prev);
    }

    return val;
}

void trie_strtolower(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = tolower(s[i]);
}

void trie_strtoupper(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = toupper(s[i]);
}

void trie_rfc1459_strtoupper(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = rfc1459_toupper(s[i]);
}

void trie_rfc1459_strtolower(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = rfc1459_tolower(s[i]);
}
