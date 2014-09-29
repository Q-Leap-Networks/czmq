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

#ifndef __ZDLIST_H_INCLUDED__
#define __ZDLIST_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Type of a dlist
typedef struct _zdlist_t zdlist_t;

//  Create a new dlist container (a dlist is a doubly-linked cyclic
//  list). Produce a guard element if item is NULL. See zdlist_first/next.
CZMQ_EXPORT zdlist_t *
    zdlist_new (void *item);

//  Destroy a dlist container
CZMQ_EXPORT void
    zdlist_destroy (zdlist_t **self_p);

//  Insert an item after an element, return new element if OK, else
//  NULL. Creates a new dlist if self is NULL. Insert a guard element
//  if item is NULL. See zdlist_first/next.
CZMQ_EXPORT zdlist_t *
    zdlist_insert_after (zdlist_t *self, void *item);

//  Insert an item before an element, return new element if OK, else
//  NULL. Creates a new dlist if self is NULL. Insert a guard element
//  if item is NULL. See zdlist_first/next.
CZMQ_EXPORT zdlist_t *
    zdlist_insert_before (zdlist_t *self, void *item);

//  Detach first element from the dlist and return it. Advances self_p to
//  the next element in the dlist or NULL if none left.
CZMQ_EXPORT zdlist_t *
    zdlist_detach (zdlist_t **self_p);

//  Remove first element from the dlist and return its item. Advances
//  self_p to the next element in the dlist or NULL if none left.
CZMQ_EXPORT void *
    zdlist_remove (zdlist_t **self_p);

//  Remove first element from the dlist and destroy it. Advances
//  self_p to the next element in the dlist or NULL if none left.
CZMQ_EXPORT void
    zdlist_destroy_one (zdlist_t **self_p);

//  Move first element from the dlist to after the destination.
//  Destination may be another dlist.
CZMQ_EXPORT void
    zdlist_move_after (zdlist_t *self, zdlist_t *destination);

//  Move first element from the dlist to before the destination. Destination
//  may be another dlist.
CZMQ_EXPORT void
    zdlist_move_before (zdlist_t *self, zdlist_t *destination);

//  Return first element with item from the dlist. If self is NULL
//  return NULL. If terminator is NULL return NULL when there is no
//  element before the next guard. Otherwise return NULL if there is
//  no element that is not a guard before the terminator.
CZMQ_EXPORT zdlist_t *
    zdlist_first (zdlist_t *self, zdlist_t *terminator);

//  Return next element with item from the dlist. If terminator is NULL
//  return NULL when the next element is a guard. Otherwise return
//  NULL if the next element that is no a guard is the terminator.
CZMQ_EXPORT zdlist_t *
    zdlist_next (zdlist_t *last, zdlist_t *terminator);

//  Return the item of an element of the dlist. If self is NULL return
//  NULL. Returns NULL on guard elements.
CZMQ_EXPORT void *
    zdlist_item (zdlist_t *self);

//  Set a user-defined deallocator for dlist items; by default items are not
//  freed when the dlist is destroyed. This is an O(n) operation.
CZMQ_EXPORT czmq_destructor *
    zdlist_set_destructor (zdlist_t *self, czmq_destructor destructor);

//  Set a user-defined duplicator for dlist items; by default items are not
//  copied when inserted into the dlist. This is an O(n) operation.
CZMQ_EXPORT czmq_duplicator *
    zdlist_set_duplicator (zdlist_t *self, czmq_duplicator duplicator);

//  Self test of this class
CZMQ_EXPORT void
    zdlist_test (int verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
