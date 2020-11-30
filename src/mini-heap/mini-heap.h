/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 09:58:54
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-16 17:56:48
 */ 
#ifndef MINI_HEAP_H
#define MINI_HEAP_H

#include <stdint.h>
#include <unistd.h>

//#define heap_size(heap) 	((heap)->size)
#define heap_parent(npos)	((int32_t)(((npos) - 1) / 2))
#define heap_left(npos)		(((npos) * 2) + 1)
#define heap_right(npos)	(((npos) * 2) + 2)

typedef int32_t (*heap_compare_t)(const void *, const void *);
typedef void (*heap_destroy_t)(void *);
typedef void (*heap_index_t)(void *, const uint32_t);

typedef struct {
	int32_t	size;
	heap_compare_t compare;
	heap_destroy_t destroy;
	heap_index_t index;
	void **tree;
}Heap;

void heap_init(void *, heap_compare_t, heap_destroy_t,heap_index_t);
void heap_destroy(void *);
int32_t heap_insert(void *, const void *);
int32_t heap_extract(void *, void **);
int32_t heap_size(void *);



#endif // HEAP_H