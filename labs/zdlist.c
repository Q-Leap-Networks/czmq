/*  =========================================================================
    zdlist - generic type-free doubly-linked cyclic list container

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/*
@header
    Provides a generic doubly-linked cyclic list container, in which
    all elements of the list are equipotent. This container provides
    hooks for duplicator and destructor functions. These tie into CZMQ
    and standard C semantics, so e.g. for string items you can use
    strdup and zstr_free. To store custom objects, define your own
    duplicator and use the standard object destructor.
@discuss
    A dlist consists of elements doubly linked into a cyclic list. Each
    elements contains a pointer to an item or NULL if it is a guard
    element.
    
    A dlist can have 0, 1 or multiple guard elements depending on its
    intended use. A guard element can act as head of the dlist,
    simulating an empty dlist but able to have a destructor and
    duplicator.

    A dlist can be iterated either until the next guard element, or
    until a specific element is reached (allowing for a full round
    trip) while guard elements are skipped.
@end
*/

#include "../include/czmq.h"
#include "zdlist.h"

//  ---------------------------------------------------------------------
//  Structure of our class

struct _zdlist_t {
    struct _zdlist_t *next;
    struct _zdlist_t *prev;
    void *item;
    czmq_destructor *destructor;
    czmq_duplicator *duplicator;
};


//  --------------------------------------------------------------------------
//  Create a new dlist container (a dlist is a doubly-linked cyclic list)

zdlist_t *
zdlist_new (void *item)
{
    zdlist_t *self = (zdlist_t *) zmalloc (sizeof (zdlist_t));
    if (self) {
        self->prev = self;
        self->next = self;
        self->item = item;
    }
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy a dlist container

void
zdlist_destroy (zdlist_t **self_p)
{
    assert (self_p);
    while (*self_p) {
        zdlist_t *node = zdlist_detach (self_p);
        if (node->destructor)
            (node->destructor) ((void **) &node);
        free (node);
    }
}


//  --------------------------------------------------------------------------
//  Insert an item after an element, return new element if OK, else
//  NULL. Creates a new dlist if self is NULL.
zdlist_t *
zdlist_insert_after (zdlist_t *self, void *item)
{
    // Duplicate item if duplicator is set
    if (self && self->duplicator && item) {
        item = (self->duplicator) (item);
        if (!item)
            return NULL; // duplication failure, abort
    }

    // Create new dlist element
    zdlist_t *node = zdlist_new (item);
    if (!node) {
        // allocation failure, check if we need to destroy a duplicated item
        if (self->duplicator && self->destructor)
            (self->destructor) (&item);
        return NULL;
    }

    // copy destructor and duplicator and move node into dlist
    if (self) {
        node->destructor = self->destructor;
        node->duplicator = self->duplicator;
        zdlist_move_after (node, self);
    }

    return node;
}


//  --------------------------------------------------------------------------
//  Insert an item before an element, return new element if OK, else
//  NULL. Creates a new dlist if self is NULL.
zdlist_t *
zdlist_insert_before (zdlist_t *self, void *item)
{
    if (self)
        self = self->prev;
    return zdlist_insert_after (self, item);
}


//  --------------------------------------------------------------------------
//  Detach first element from the dlist and return it. Advances self_p to
//  the next element in the dlist or NULL if none left.
zdlist_t *
zdlist_detach (zdlist_t **self_p)
{
    assert (self_p);
    zdlist_t *node = *self_p;
    
    if (node) {
        // advance self_p
        if (node->next == node)
            *self_p = NULL;
        else
            *self_p = node->next;

        // split linkage of dlist
        node->next->prev = node->prev;
        node->prev->next = node->next;
        node->next = node;
        node->prev = node;
    }

    return node;
}


//  --------------------------------------------------------------------------
//  Remove first element from the dlist and return its item. Advances
//  self_p to the next element in the dlist or NULL if none left.
void *
zdlist_remove (zdlist_t **self_p)
{
    assert (self_p);
    zdlist_t *node = zdlist_detach (self_p);
    void *item = NULL;

    if (node) {
        item = node->item;
        free (node);
    }

    return item;
}


//  --------------------------------------------------------------------------
//  Remove first element from the dlist and destroy it. Advances
//  self_p to the next element in the dlist or NULL if none left.
void
zdlist_destroy_one (zdlist_t **self_p)
{
    zdlist_t *self = zdlist_detach (self_p);
    zdlist_destroy (&self);
}


//  --------------------------------------------------------------------------
//  Move first element from the dlist to after the destination. Destination may
//  be another dlist.
void
zdlist_move_after (zdlist_t *self, zdlist_t *destination)
{
    assert (self);
    assert (destination);
    // remove self from old dlist
    self->prev->next = self->next;
    self->next->prev = self->prev;
    // insert into new dlist
    self->prev = destination;
    self->next = destination->next;
    self->prev->next = self;
    self->next->prev = self;
}


//  --------------------------------------------------------------------------
//  Move first element from the dlist to before the destination. Destination
//  may be another dlist.
void
zdlist_move_before (zdlist_t *self, zdlist_t *destination)
{
    assert (destination);
    zdlist_move_after (self, destination->prev);
}


//  --------------------------------------------------------------------------
//  Return first element with item from the dlist. If self is NULL
//  return NULL. If terminator is NULL return NULL when there is no
//  element before the next guard. Otherwise return NULL if there is
//  no element that is not a guard before the terminator.
zdlist_t *
zdlist_first (zdlist_t *self, zdlist_t *terminator)
{
    if (!self)
        return NULL;
    if (self->item)
        return self;
    return zdlist_next (self, terminator);
}


//  --------------------------------------------------------------------------
//  Return next element with item from the dlist. If terminator is NULL
//  return NULL when the next element is a guard. Otherwise return
//  NULL if the next element that is no a guard is the terminator.
zdlist_t *
zdlist_next (zdlist_t *last, zdlist_t *terminator)
{
    assert (last);
    zdlist_t *node = last->next;
    if (node == terminator)
        return NULL;
    if (node->item)
        return node;
    if (terminator)
        return zdlist_next (node, terminator);
    return NULL;
}


//  --------------------------------------------------------------------------
//  Return the item of an element of the dlist. If self is NULL return
//  NULL.
void *
zdlist_item (zdlist_t *self)
{
    if (self)
        return self->item;
    else
        return NULL;
}


//  --------------------------------------------------------------------------
//  Set a user-defined deallocator for dlist items; by default items are not
//  freed when the dlist is destroyed. This is an O(n) operation.

czmq_destructor *
zdlist_set_destructor (zdlist_t *self, czmq_destructor destructor)
{
    assert (self);
    czmq_destructor *old = self->destructor;
    self->destructor = destructor;
    zdlist_t *node = self->next;
    while (node != self) {
        node->destructor = destructor;
        node = node->next;
    }
    return old;
}


//  --------------------------------------------------------------------------
//  Set a user-defined duplicator for dlist items; by default items are not
//  copied when the dlist is duplicated. This is an O(n) operation.

czmq_duplicator *
zdlist_set_duplicator (zdlist_t *self, czmq_duplicator duplicator)
{
    assert (self);
    czmq_duplicator *old = self->duplicator;
    self->duplicator = duplicator;
    zdlist_t *node = self->next;
    while (node != self) {
        node->duplicator = duplicator;
        node = node->next;
    }
    return old;
}


//  --------------------------------------------------------------------------
//  Runs selftest of class

void
zdlist_test (int verbose)
{
    printf (" * zdlist: ");

    //  @selftest
    //  Three items we'll use as test data
    //  Dlist items are void *, not particularly stdlists
    char *cheese = "boursin";
    char *bread = "baguette";
    char *wine = "bordeaux";

    // test insertion
    zdlist_t *dlist = zdlist_insert_after (NULL, NULL);
    assert (dlist);

    zdlist_t *node = zdlist_insert_after (dlist, cheese);
    assert (zdlist_item (node) == cheese);

    node = zdlist_insert_after (dlist, bread);
    assert (node);
    assert (zdlist_item (node) == bread);

    node = zdlist_insert_before (dlist, NULL);
    assert (node);

    node = zdlist_insert_before (dlist, wine);
    assert (node);
    assert (zdlist_item (node) == wine);

    // test iteration till next guard
    node = zdlist_first (dlist, NULL);
    assert (node);
    assert (zdlist_item (node) == bread);

    node = zdlist_next (node, NULL);
    assert (node);
    assert (zdlist_item (node) == cheese);
    
    node = zdlist_next (node, NULL);
    assert (!node);

    // test iteration all the way around
    node = zdlist_first (dlist, dlist);
    assert (node);
    assert (zdlist_item (node) == bread);

    node = zdlist_next (node, dlist);
    assert (node);
    assert (zdlist_item (node) == cheese);
    
    node = zdlist_next (node, dlist);
    assert (node);
    assert (zdlist_item (node) == wine);
    
    node = zdlist_next (node, NULL);
    assert (!node);

    // test detach
    node = zdlist_detach (&dlist);
    assert (zdlist_item (node) == NULL);
    assert (zdlist_item (dlist) == bread);
    zdlist_destroy (&node);
    assert (!node);

    node = zdlist_detach (&dlist);
    assert (zdlist_item (node) == bread);
    assert (zdlist_item (dlist) == cheese);
    zdlist_destroy (&node);
    assert (!node);

    node = zdlist_detach (&dlist);
    assert (zdlist_item (node) == cheese);
    assert (zdlist_item (dlist) == NULL);
    zdlist_destroy (&node);
    assert (!node);

    // destroy leftovers
    zdlist_destroy (&dlist);
    assert (!dlist);
    //  @end

    printf ("OK\n");
}
