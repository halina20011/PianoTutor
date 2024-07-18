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

#include "singlyLinkedList.h"

struct List *newSinglyLinkedList(){
    struct List *list = malloc(sizeof(struct List));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

// if both are null: then return null
// if tail is null: get last node from head
// if head->next null: set it to made node
void listPush(struct List *list, void *val){
    struct Node *newNode = malloc(sizeof(struct Node));
    if(newNode == NULL){
        return;
    }

    newNode->val = val;
    newNode->next = NULL;
    
    if(list->head == NULL){
        list->head = newNode;
    }
    else{
        list->tail->next = newNode;
    }

    list->tail = newNode;

    list->size++;
}

void listInsert(struct List *list, void *val){
    struct Node *newNode = malloc(sizeof(struct Node));
    if(newNode == NULL){
        return;
    }

    newNode->next = list->head;
    newNode->val = val;

    if(!list->head){
        list->tail = newNode;
    }

    list->head = newNode;
    list->size++;
}

void *listPop(struct List *list){
    if(!list || !list->head){
        return NULL;
    }
    
    struct Node *temp = list->head;

    if(list->head == list->tail){
        list->head = NULL;
        list->tail = NULL;
    }
    else{
        list->head = list->head->next;
    }

    void *val = temp->val;
    free(temp);

    list->size--;
    return val;
}

void listJoin(struct List *to, struct List *from){
    if(!from->head){
        return;
    }

    if(!to->head){
        to->head = from->head;
        to->tail = from->tail;
    }
    else{
        to->tail->next = from->head;
        to->tail = from->tail;
    }

    to->size += from->size;
    free(from);
}

void singlyLinkedListClear(struct List *list, bool freeVal){
    struct Node *curr = list->head, *temp = NULL;

    while(curr){
        temp = curr;
        curr = curr->next;
        if(freeVal){
            free(temp->val);
        }

        free(temp);
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void freeSinglyLinkedList(struct List *list, bool freeVal){
    singlyLinkedListClear(list, freeVal);
    free(list);
}
