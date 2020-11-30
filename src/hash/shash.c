/*
 * Copyright (C) 2014-2015  liangdong <liangdong01@baidu.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "shash.h"

struct shash *shash_new()
{
    struct shash *shash = calloc(1, sizeof(*shash));
    shash->num_buckets[0] = SHASH_INIT_NUM_BUCKETS;
    shash->buckets[0] = calloc(1ULL << SHASH_INIT_NUM_BUCKETS, sizeof(struct shash_node));
    return shash;
}

static struct shash_node *_shash_new_node(const char *key, uint32_t key_len, void *value)
{
    struct shash_node *node = calloc(1, sizeof(*node));
    node->key = malloc(key_len);
    memcpy(node->key, key, key_len);
    node->key_len = key_len;
    node->value = value;
    return node;
}

static void _shash_free_node(struct shash_node *node)
{
    free(node->key);
    free(node);
}

void shash_free(struct shash *shash)
{
    int i;
    for (i = 0; i < 2; ++i) {
        struct shash_node *buckets = shash->buckets[i];
        if (!buckets)
            break;
        uint64_t num_buckets = 1ULL << shash->num_buckets[i];
        uint64_t b;
        for (b = 0; b < num_buckets; ++b) {
            struct shash_node *node = buckets[b].next;
            while (node) {
                struct shash_node *next = node->next;
                _shash_free_node(node);
                node = next;
            }
        }
        free(buckets);
    }
    free(shash);
}

static uint64_t _shash_hash_key(const char *key, uint32_t key_len)
{
    uint64_t hash = 0;
    uint32_t i;
    for (i = 0; i < key_len; ++i){
        hash = hash * 33 + (uint8_t)key[i];
		printf("hash:%d,key%d---%x\n",hash,i,(uint8_t)key[i]);
    	}
	#if 0
	printf("hash = %d\n",sizeof(hash));
	uint8_t buff[10];
	memcpy(buff,&hash,8);
	for(i =0 ; i < 8 ;i ++)
		printf("%u",buff[i]);
	printf("\n");
	#endif
    return hash;
}

static int _shash_key_compare(const char *key1, uint32_t key1_len, const char *key2, uint32_t key2_len)
{
    int min_len = key1_len < key2_len ? key1_len : key2_len;
    int ret = memcmp(key1, key2, min_len);
    if (ret)
        return ret;//最小范围内比较，两者不等时
    if (key1_len == key2_len)
    	return 0;//最小范围内比较，两者相等，然后两者真实长度也相等则返回0
    return key1_len < key2_len ? -1 : 1;//两者相等，但key1长度值小于key2长度时。
}

static void _shash_check_rehash(struct shash *shash)
{
	/* 只在Iterator关闭的情况下才启动rehash */
	if (!shash->buckets[1] && !shash->iterator) {
		uint64_t num_buckets = 1ULL << shash->num_buckets[0];
		if (shash->num_node >= num_buckets &&	//总节点数大于大于表0的桶个数
				shash->num_buckets[0] + 1 <= SHASH_MAX_NUM_BUCKETS) {//表0的指数+1以后小于最大指数
			//上述条件成立则重新定义表0的指数并且准备rehash
			shash->num_buckets[1] = shash->num_buckets[0] + 1;
			shash->buckets[1] = calloc(1ULL << shash->num_buckets[1], sizeof(struct shash_node));
			shash->rehash_bucket = 0;
		}
	}
	/* 只在iterator关闭的情况下执行rehash, 否则会造成用户迭代重复数据 */
	if (shash->buckets[1] && !shash->iterator) {
		//渐进式rehash，取第rehash_bucket个桶
		struct shash_node *head = &shash->buckets[0][shash->rehash_bucket];
		//取该桶的第一个成员
		struct shash_node *node = head->next;
		//计算掩码，eg.当指数为10时，则掩码的值为1023
		uint64_t buckets_mask = (1ULL << shash->num_buckets[1]) - 1;
		while (node) {//该桶下的第一个成员存在
			//取该桶第一个成员的hash值
			uint64_t hash = _shash_hash_key(node->key, node->key_len);
			uint64_t bucket_index = hash & buckets_mask;
			//to指向表1的第bucket_index个桶
			struct shash_node *to = &shash->buckets[1][bucket_index];
			
			while (to->next) {//如果该桶已经存在成员
				//表0的node和表1的成员进行比较，node的key大于to的key
				//则to指向下一个成员
				int ret = _shash_key_compare(node->key, node->key_len, to->next->key, to->next->key_len);
				if (ret <= 0)
					break;
				to = to->next;
			}
			//暂存node的下一个节点
			struct shash_node *next = node->next;
			//node被插入到to节点之后
			//node的next指向to之后的成员
			node->next = to->next;
			if (to->next)//如果to不是最末节点，则to节点的prev指向node节点
				to->next->prev = node;
			to->next = node;//to的next指向node
			node->prev = to;
			
			node = next;
		}
		head->next = NULL;
		/* rehash结束 */
		if (++shash->rehash_bucket == (1ULL << shash->num_buckets[0])) {
			free(shash->buckets[0]);
			shash->buckets[0] = shash->buckets[1];
			shash->buckets[1] = NULL;
			shash->num_buckets[0] = shash->num_buckets[1];
		}
	}
}

int shash_insert(struct shash *shash, const char *key, uint32_t key_len, void *value)
{
	_shash_check_rehash(shash);
    uint64_t hash = _shash_hash_key(key, key_len);
    struct shash_node *buckets = shash->buckets[0];
    uint64_t bucket_index = hash & ((1ULL << shash->num_buckets[0]) - 1);

    if (shash->buckets[1] && bucket_index < shash->rehash_bucket) {
        buckets = shash->buckets[1];
        bucket_index = hash & ((1ULL << shash->num_buckets[1]) - 1);
    }

    struct shash_node *node = &buckets[bucket_index];
    while (node->next) {
    	int ret = _shash_key_compare(key, key_len, node->next->key, node->next->key_len);
    	if (ret == 0)//不支持相同的key
    		{
    		printf("key common not insert\n");
    		return -1;
    		}
    	else if (ret < 0)//找到合适的节点位置
    		break;
    	node = node->next;
    }

    struct shash_node *new_node = _shash_new_node(key, key_len, value);
    new_node->next = node->next;
    if (node->next)
    	node->next->prev = new_node;
    node->next = new_node;
    new_node->prev = node;
    ++shash->num_node;
    return 0;
}

static struct shash_node *_shash_find(struct shash *shash, const char *key, uint32_t key_len)
{
    uint64_t hash = _shash_hash_key(key, key_len);
    struct shash_node *buckets = shash->buckets[0];
    uint64_t bucket_index = hash & ((1ULL << shash->num_buckets[0]) - 1);

    if (shash->buckets[1] && bucket_index < shash->rehash_bucket) {
        buckets = shash->buckets[1];
        bucket_index = hash & ((1ULL << shash->num_buckets[1]) - 1);
    }

    struct shash_node *node = &buckets[bucket_index];
    while (node->next) {
    	int ret = _shash_key_compare(key, key_len, node->next->key, node->next->key_len);
    	if (ret == 0)
    		return node->next;
    	else if (ret < 0)
    		break;
    	node = node->next;
    }
    return NULL;
}

int shash_erase(struct shash *shash, const char *key, uint32_t key_len)
{
	_shash_check_rehash(shash);
	struct shash_node *node = _shash_find(shash, key, key_len);
	if (!node)
		return -1;
	/* 删除的节点是迭代器指向的节点 */
	if (node == shash->iterator) {
		shash_iterate(shash, NULL, NULL, NULL);
	}
	if (node->next)
		node->next->prev = node->prev;
	node->prev->next = node->next;
	_shash_free_node(node);
	--shash->num_node;
	return 0;
}

int shash_find(struct shash *shash, const char *key, uint32_t key_len, void **value)
{
	_shash_check_rehash(shash);
	struct shash_node *node = _shash_find(shash, key, key_len);
	if (!node)
		return -1;
	if (value)
		*value = node->value;
	return 0;
}

void shash_begin_iterate(struct shash *shash)
{
	int i;
	for (i = 0; i < 2; ++i) {
		if (!shash->buckets[i])
			break;
		struct shash_node *buckets = shash->buckets[i];
		uint64_t num_buckets = 1ULL << shash->num_buckets[i];
		uint64_t b;
		for (b = 0; b < num_buckets; ++b) {
			if (buckets[b].next) {
				shash->iterator_index = i;
				shash->iterator_bucket = b;
				shash->iterator = buckets[b].next;
				return;
			}
		}
	}
	shash->iterator = NULL;
}

int shash_iterate(struct shash *shash, const char **key, uint32_t *key_len, void **value)
{
	if (!shash->iterator)
		return -1;
	if (key)
	    *key = shash->iterator->key;
	if (key_len)
	    *key_len = shash->iterator->key_len;
	if (value)
	    *value = shash->iterator->value;

	shash->iterator = shash->iterator->next;
	if (!shash->iterator) {	/* iterator为空则换桶*/
	    ++shash->iterator_bucket;
	    uint64_t num_buckets = 1ULL << shash->num_buckets[shash->iterator_index];
	    int i;
	    for (i = shash->iterator_index; i < 2; ++i) {
	        struct shash_node *buckets = shash->buckets[i];
	        if (!buckets)
	            break;
	        uint64_t b;
	        for (b = shash->iterator_bucket; b < num_buckets; ++b) {
	            if (buckets[b].next) {
	                shash->iterator_bucket = b;
	                shash->iterator = buckets[b].next;
	                shash->iterator_index = i;
	                return 0;
	            }
	        }
	        if (i == 0) {
	            shash->iterator_bucket = 0;
	        }
	    }
	}
	return 0;
}

void shash_end_iterate(struct shash *shash)
{
	shash->iterator = NULL;
}

uint64_t shash_size(struct shash *shash)
{
    return shash->num_node;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
