/*  =========================================================================
    zspeedtest - test speed of hash and dlist operations as needed for ztimeout

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/* Memory requirements:
 * MAX_SIZE (1 << 21)
 * zhash:  369m
 * zdlist: 353m
 *
 * Results: Intel(R) Atom(TM) CPU  330   @ 1.60GHz, gcc (Debian 4.8.3-1) 4.8.3
 * zhash:  1048576 insertions in 3.004000 seconds [349.059920 k/second]
 * zhash:  2097152 insertions in 6.231000 seconds [336.567485 k/second]
 *
 * zhash:  1048576 deletions in 1.101000 seconds [952.385104 k/second]
 * zhash:  2097152 deletions in 2.137000 seconds [981.353299 k/second]
 *
 * zhash:   1048576 insert/deletions in 3.376000 seconds [310.597156 k/second]
 * zhash:   2097152 insert/deletions in 6.758000 seconds [310.321397 k/second]
 * zhash:   4194304 insert/deletions in 13.529000 seconds [310.023209 k/second]
 * zhash:   8388608 insert/deletions in 27.109000 seconds [309.439965 k/second]
 * zhash:  16777216 insert/deletions in 54.223000 seconds [309.411431 k/second]
 *
 * zdlist:  1048576 insertions in 0.770000 seconds [1361.787013 k/second]
 * zdlist:  2097152 insertions in 1.535000 seconds [1366.222801 k/second]
 *
 * zdlist:  1048576 deletions in 0.448000 seconds [2340.571429 k/second]
 * zdlist:  2097152 deletions in 0.897000 seconds [2337.962096 k/second]
 *
 * zdlist:   1048576 moves in 0.847000 seconds [1237.988194 k/second]
 * zdlist:   2097152 moves in 1.680000 seconds [1248.304762 k/second]
 * zdlist:   4194304 moves in 3.346000 seconds [1253.527794 k/second]
 * zdlist:   8388608 moves in 6.696000 seconds [1252.778973 k/second]
 * zdlist:  16777216 moves in 13.354000 seconds [1256.343867 k/second]
 */

#include "../include/czmq.h"
#include "zdlist.h"

#define MAX_SIZE (1 << 21)
#define REPEAT_COUNT 8

// key helper functions
typedef struct {
    char hex [17];
} hex_id_t;

static hex_id_t *
s_hex_id_new (int64_t id)
{
    static const char hex[] = "0123456789ABCDEF";
    hex_id_t *self = (hex_id_t *) zmalloc (sizeof (hex_id_t));
    if (self) {
        int index;
        byte *ptr = (byte *) &id;
        for (index = 0; index < 8; ++index) {
            self->hex [index * 2] = hex [ptr [index] / 16];
            self->hex [index * 2 + 1] = hex [ptr [index] % 16];
        }
        self->hex [16] = 0;
    }
    return self;
}

static void
s_hex_id_destroy (hex_id_t **self_p)
{
    assert (self_p);
    hex_id_t *self = *self_p;
    free (self);
    *self_p = NULL;
}

static hex_id_t *
s_hex_id_dup (const hex_id_t *self) {
    assert (self);
    hex_id_t *copy = s_hex_id_new (0);
    if (copy)
        memcpy (copy->hex, self->hex, 17);
    return copy;
}

// value helper functions
typedef struct {
    char data[64];
} value_t;

static value_t *
s_value_new ()
{
    return (value_t *) zmalloc (sizeof (value_t));
}

static void
s_value_destroy (value_t **self_p)
{
    assert (self_p);
    value_t *self = *self_p;
    free (self);
    *self_p = NULL;
}

static value_t *
s_value_dup (value_t *self)
{
    value_t *copy = s_value_new ();
    if (copy)
        memcpy (copy, self, sizeof (value_t));
    return copy;
}

// test speed
void
zspeedtest_test (int verbose)
{
    int rc;
    printf(" * zspeedtest ...\n");

    // test speed of zhash
    // -------------------
    
    // create hash table
    zhash_t *zhash = zhash_new ();
    assert (zhash);
    zhash_set_key_destructor (zhash, (czmq_destructor *) s_hex_id_destroy);
    zhash_set_key_duplicator (zhash, (czmq_duplicator *) s_hex_id_dup);
    zhash_set_destructor (zhash, (czmq_destructor *) s_value_destroy);
    zhash_set_duplicator (zhash, (czmq_duplicator *) s_value_dup);

    // time insertions
    int64_t start = zclock_mono ();
    int count = 0;
    int next_clock = 1024;
    int64_t key = 1;
    value_t value;
    while (count < MAX_SIZE) {
        hex_id_t *hex_id = s_hex_id_new (key++);
        assert (hex_id);
        rc = zhash_insert (zhash, (const void *) hex_id, (void *) &value);
        assert (rc == 0);
        s_hex_id_destroy (&hex_id);
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zhash: %8d insertions in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // time deletions
    start = zclock_mono ();
    key = 1;
    next_clock = 1024;
    count = 0;
    while (count < MAX_SIZE) {
        hex_id_t *hex_id = s_hex_id_new (key++);
        assert (hex_id);
        zhash_delete (zhash, (const void *) hex_id);
        s_hex_id_destroy (&hex_id);
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zhash: %8d deletions in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // time insert/deletions
    int64_t keys [MAX_SIZE] = { 0 };
    for (count = 0; count < MAX_SIZE; ++count) {
        hex_id_t *hex_id = s_hex_id_new (key++);
        assert (hex_id);
        rc = zhash_insert (zhash, (const void *) hex_id, (void *) &value);
        assert (rc == 0);
        s_hex_id_destroy (&hex_id);
        keys [count] = key;
    }
    start = zclock_mono ();
    next_clock = 1024;
    count = 0;
    while (count < MAX_SIZE * REPEAT_COUNT) {
        int index = random() % MAX_SIZE;
        hex_id_t *hex_id = s_hex_id_new (keys [index]);
        assert (hex_id);
        zhash_delete (zhash, (const void *) hex_id);
        s_hex_id_destroy (&hex_id);
        hex_id = s_hex_id_new (key++);
        assert (hex_id);
        rc = zhash_insert (zhash, (const void *) hex_id, (void *) &value);
        assert (rc == 0);
        s_hex_id_destroy (&hex_id);
        keys [index] = key;
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zhash: %9d insert/deletions in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // cleanup
    zhash_destroy (&zhash);

    // test speed of zdlist
    // --------------------
    
    // create zdlist
    zdlist_t *zdlist = zdlist_new (NULL);
    assert (zdlist);
    zdlist_set_destructor (zdlist, (czmq_destructor *) s_value_destroy);
    zdlist_set_duplicator (zdlist, (czmq_duplicator *) s_value_dup);

    // time insertions
    start = zclock_mono ();
    count = 0;
    next_clock = 1024;
    while (count < MAX_SIZE) {
        zdlist_insert_before (zdlist, (void *) &value);
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zdlist: %8d insertions in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // time deletions
    start = zclock_mono ();
    next_clock = 1024;
    count = 0;
    zdlist_t *iterator = zdlist_first (zdlist, NULL);
    while (iterator) {
        zdlist_destroy_one (&iterator);
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zdlist: %8d deletions in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // time move
    zdlist_t *items [MAX_SIZE] = { 0 };
    for (count = 0; count < MAX_SIZE; ++count) {
        items [count] = zdlist_insert_before (zdlist, (void *) &value);
    }
    start = zclock_mono ();
    next_clock = 1024;
    count = 0;
    while (count < MAX_SIZE * REPEAT_COUNT) {
        int index = random() % MAX_SIZE;
        zdlist_move_after (items [index], zdlist);
        count++;
        if (count >= next_clock) {
            int64_t now = zclock_mono ();
            next_clock *= 2;
            if (now == start)
                continue;
            double time = (now - start) / 1000.0;
            printf("   zdlist: %9d moves in %f seconds [%f k/second]\n",
                   count, time, count / time / 1000.0);
        }
    }

    // cleanup
    zdlist_destroy (&zdlist);

    printf(" * zspeedtest: OK\n");
}
