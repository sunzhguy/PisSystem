/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 09:58:54
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 14:31:47
 */ 
#ifndef MINI_HEAP_H
#define MINI_HEAP_H

#include <stdint.h>
#include <unistd.h>

//#define heap_size(heap) 	((heap)->size)
#define HEAP_PARENT(npos)	((int32_t)(((npos) - 1) / 2))
#define HEAP_LEFT(npos)		(((npos) * 2) + 1)
#define HEAP_RIGHT(npos)	(((npos) * 2) + 2)

typedef int32_t (*PF_HEAP_COMPARE)(const void *, const void *);
typedef void (*PF_HEAP_DESTROY)(void *);
typedef void (*PF_HEAP_INDEX)(void *, const uint32_t);

typedef struct {
	int32_t	iSize;
	PF_HEAP_COMPARE pfCompare;
	PF_HEAP_DESTROY pfDestory;
	PF_HEAP_INDEX   pfIndex;
	void **tree;
}T_HEAP;

void MiniHeap_Init(void *, PF_HEAP_COMPARE, PF_HEAP_DESTROY,PF_HEAP_INDEX);
void MiniHeap_Destroy(void *);
int32_t MiniHeap_Insert(void *, const void *);
int32_t MiniHeap_Extract(void *, void **);
int32_t MiniHeap_Size(void *);



#endif // HEAP_H