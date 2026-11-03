#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

mymemory_t* mymemory_init (size_t size){
	mymemory_t *memory = malloc(sizeof(mymemory_t));

	if(memory == NULL){
		return NULL;
	}
	
	memory -> pool = malloc(size);

	if(memory -> pool == NULL){
		free(memory);
		return NULL;
	}

	memory -> total_size = size;

	memory -> head = NULL;

	return memory;
}

void mymemory_cleanup(mymemory_t *memory){
	if(memory == NULL){
		return;
	}

	allocation_t *current = memory->head;

	while(current->next != NULL){
		allocation_t *next = current -> next;			
		free(current);
		current = next;
	}

	free(memory->pool);
	free(memory);
}

void* mymemory_alloc(mymemory_t *memory, size_t size){

	if(memory == NULL)return NULL;
	if(memory->pool == NULL)return NULL;

	if(size > memory->total_size || size == 0){
		return NULL;
	}

	if(memory->head == NULL){
		allocation_t* alloc = malloc(sizeof(allocation_t));

		if(alloc == NULL){
			return NULL;
		}

		alloc->start = memory->pool;
		alloc->size = size;
		alloc->next = NULL;

		memory->head = alloc;
		return alloc->start;
	}

	allocation_t* alloc = malloc(sizeof(allocation_t));

	if(alloc == NULL){
		return NULL;
	}

	alloc->size = size;

	allocation_t* current = memory->head;
	allocation_t* previous = memory->head;

	size_t iBlock = (char*)memory->head->start - (char*)memory->pool; 

	if(iBlock >= size){
		alloc->start = memory->pool;
		alloc->next = memory->head;
		memory->head = alloc;

		return alloc->start;
	}

	while(current->next != NULL){
		current = current -> next;			
		
		size_t fBlock = (char*)current->start - ((char*)previous->start + previous->size);

		if(fBlock >= size){
			previous -> next = alloc;
			alloc -> next = current;
			return alloc->start = previous->start + previous->size;
		}else{
			previous = current;
		}
	}

	size_t rBlock = ((char*)memory->pool + memory->total_size) - ((char*)current->start + current->size);

	if (rBlock < size) {		
		free(alloc);
		return NULL; 
	}else{
		current -> next = alloc;
		alloc -> next = NULL;
		return alloc->start = current -> start + current -> size;

	}
}


void mymemory_free(mymemory_t *memory, void *ptr){
	if(memory == NULL || ptr == NULL){
		return;
	}

	allocation_t *current = memory->head;
	allocation_t *previous = NULL;

	while(current != NULL && current->start != ptr){
		previous = current;
		current = current -> next;
	}

	if(current == NULL){
		return;
	}

	if(previous == NULL){
		memory->head = current->next;
	}else{
		previous->next = current->next;
	}

	free(current);
}

