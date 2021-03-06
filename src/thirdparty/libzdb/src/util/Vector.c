/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.
 */


#include "Config.h"

#include <stdio.h>

#include "Vector.h"


/**
 * Implementation of the Vector interface. 
 *
 * @file
 */


/* ----------------------------------------------------------- Definitions */


#define T Vector_T
struct T {
        int length;
        int capacity;
        void **array;
	uint32_t timestamp;
};


/* ------------------------------------------------------- Private methods */


static inline void _ensureCapacity(T V) {
        if (V->length >= V->capacity) {
                V->capacity = 2 * V->length;
		RESIZE(V->array, V->capacity * sizeof (void *));
        }
}


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T Vector_new(int hint) {
	T V;
        Assertion_assert(hint >= 0);
	NEW(V);
        if (hint == 0)
                hint = 16;
        V->capacity = hint;
        V->array = CALLOC(V->capacity, sizeof (void *));
        return V;
}


void Vector_free(T *V) {
	Assertion_assert(V && *V);
	FREE((*V)->array);
	FREE(*V);
}


void Vector_insert(T V, int i, void *e) {
	Assertion_assert(V);
	Assertion_assert(i >= 0 && i <= V->length);
	V->timestamp++;
        _ensureCapacity(V);
        for (int j = V->length++; j > i; j--)
                V->array[j] = V->array[j-1];
        V->array[i] = e;
}


void *Vector_set(T V, int i, void *e) {
	Assertion_assert(V);
	Assertion_assert(i >= 0 && i < V->length);
	V->timestamp++;
	void *prev = V->array[i];
        V->array[i] = e;
	return prev;
}


void *Vector_get(T V, int i) {
	Assertion_assert(V);
	Assertion_assert(i >= 0 && i < V->length);
        return V->array[i];
}


void *Vector_remove(T V, int i) {
	Assertion_assert(V);
	Assertion_assert(i >= 0 && i < V->length);
	V->timestamp++;
 	void *x = V->array[i];
        V->length--;
        for (int j = i; j < V->length; j++)
                V->array[j] = V->array[j+1];
        return x;
}


void Vector_push(T V, void *e) {
        Assertion_assert(V);
	V->timestamp++;
        _ensureCapacity(V);
        V->array[V->length++] = e;
}


void *Vector_pop(T V) {
        Assertion_assert(V);
 	Assertion_assert(V->length>0);
	V->timestamp++;
        return V->array[--V->length];
}


int Vector_isEmpty(T V) {
        Assertion_assert(V);
        return (V->length == 0);
}


int Vector_size(T V) {
        Assertion_assert(V);
        return V->length;
}


void Vector_map(T V, void apply(const void *element, void *ap), void *ap) {
	Assertion_assert(V);
	Assertion_assert(apply);
	uint32_t stamp = V->timestamp;
	for (int i = 0; i < V->length; i++) {
                apply(V->array[i], ap);
                Assertion_assert(V->timestamp == stamp);
        }
}


void **Vector_toArray(T V) {
        int i;
        Assertion_assert(V);
	void **array = ALLOC((V->length + 1) * sizeof (*array)); 
	for (i = 0; i < V->length; i++)
		array[i] = V->array[i];
	array[i] = NULL;
	return array;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
