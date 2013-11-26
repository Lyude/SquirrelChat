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
 *  Copyright (C) 2013 Alex Iadicicco
 *
 *  Redistribution and use in source and binary forms are permitted provided
 *  that the above copyright notice and this paragraph are duplicated in all
 *  such forms and that any documentation, advertising materials, and any
 *  other materials related to such distribution and use acknowledge that
 *  the software was developed by Alexander Iadicicco. The name of Alexander
 *  Iadicicco may not be used to endorse or promote products derived from
 *  this software without specific prior written permission. THIS SOFTWARE
 *  IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 *  AND FITNESS FOR A PARTICULAR PURPOSE.
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

static sqchat_trie_e *sqchat_trie_e_new(sqchat_trie_e * up)
{
    sqchat_trie_e *e;
    int i;

    e = malloc(sizeof(*e));
    e->val = NULL;
    e->up = up;
    for (i=0; i<16; i++)
        e->n[i] = NULL;

    return e;
}

static void sqchat_trie_e_del(sqchat_trie_e *e)
{
    free(e);
}

sqchat_trie *sqchat_trie_new(void (*canonize)())
{
    sqchat_trie *sqchat_trie;
    int i;

    sqchat_trie = malloc(sizeof(*sqchat_trie));
    sqchat_trie->canonize = canonize ? canonize : __null_canonize;

    sqchat_trie->n.val = NULL;
    sqchat_trie->n.up = NULL;
    for (i=0; i<16; i++)
        sqchat_trie->n.n[i] = 0;

    return sqchat_trie;
}

static void free_real(sqchat_trie_e * e, void (*cb)(), void * priv)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (!e->n[i])
            continue;
        free_real(e->n[i], cb, priv);
        sqchat_trie_e_del(e->n[i]);
    }
    if (cb && e->val)
        cb(e->val, priv);
}

void sqchat_trie_free(sqchat_trie * sqchat_trie, void (*cb)(), void * priv)
{
    free_real(&sqchat_trie->n, cb, priv);
    free(sqchat_trie);
}

static char nibble(char * s, int i)
{
    return (i%2==0) ? s[i/2]>>4 : s[i/2]&0xf;
}

static sqchat_trie_e *retrieval(sqchat_trie * sqchat_trie, const char * tkey, int create)
{
    sqchat_trie_e *n;
    char key[U_TRIE_KEY_MAX];
    unsigned int c, nib = 0; /* even=high, odd=low */

    strncpy(key, tkey, U_TRIE_KEY_MAX);

    sqchat_trie->canonize(key);

    if (!key[0])
        return NULL;

    n = &sqchat_trie->n;

    for (; n && key[nib/2]; nib++) {
        c = nibble(key, nib);
        if (n->n[c] == NULL) {
            if (create)
                n->n[c] = sqchat_trie_e_new(n);
            else
                return NULL;
        }
        n = n->n[c];
    }

    return n;
}

void sqchat_trie_set(sqchat_trie * sqchat_trie, const char * key, void * val)
{
    sqchat_trie_e *n = retrieval(sqchat_trie, key, 1);
    n->val = val;
}

void *sqchat_trie_get(sqchat_trie * sqchat_trie, const char * key)
{
    sqchat_trie_e *n = retrieval(sqchat_trie, key, 0);
    return n ? n->val : NULL;
}

static void each(sqchat_trie_e *e, void (*cb)(), void *priv)
{
    int i;

    if (e->val != NULL)
        cb(e->val, priv);

    for (i=0; i<16; i++) {
        if (e->n[i] != NULL)
            each(e->n[i], cb, priv);
    }
}

void sqchat_trie_each(sqchat_trie *sqchat_trie, void(*cb)(), void * priv)
{
    each(&sqchat_trie->n, cb, priv);
}

void *sqchat_trie_del(sqchat_trie * sqchat_trie, const char * key)
{
    sqchat_trie_e *prev, *cur;
    void *val;
    int i, nempty;

    cur = retrieval(sqchat_trie, key, 0);

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
        if (nempty != 16 || cur->val || cur == &sqchat_trie->n)
            break;
        cur = cur->up;
        sqchat_trie_e_del(prev);
    }

    return val;
}

void sqchat_trie_strtolower(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = tolower(s[i]);
}

void sqchat_trie_strtoupper(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = toupper(s[i]);
}

void sqchat_trie_rfc1459_strtoupper(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = sqchat_rfc1459_toupper(s[i]);
}

void sqchat_trie_rfc1459_strtolower(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = sqchat_rfc1459_tolower(s[i]);
}
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
