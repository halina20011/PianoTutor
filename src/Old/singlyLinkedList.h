// Copyright (C) 2023  halina20011
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SINGLY_LINKED_LIST
#define SINGLY_LINKED_LIST

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Node{
    void *val;
    struct Node *next;
};

struct List{
    struct Node *head, *tail;
    size_t size;
};

struct List *newSinglyLinkedList();

#define printSingleLinkedList(list, format, type) do{\
    struct Node *curr = list->head; \
    while((curr = curr->next) != NULL){ \
        printf(format " => ", *(type*)curr->val); \
    } \
    printf("NULL\n"); \
} while(0);

#define listPushConvert(list, type, val) do{\
    type *data = malloc(sizeof(type)); \
    *data = val; \
    listPush(list, data); \
}while(0);

void listPush(struct List *list, void *val);
void listInsert(struct List *list, void *val);

void *listPop(struct List *list);

void listJoin(struct List *to, struct List *from);

void singlyLinkedListClear(struct List *list, bool freeVal);
void freeSinglyLinkedList(struct List *list, bool freeVal);

#endif
