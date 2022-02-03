#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "rbtree.h"
 
typedef struct memory_node {
	uint64_t start;
	uint64_t size;
	rbtree_node_t rbtree_node;
} memory_node_t;
 
#define container_of(ptr, type, member) ({			\
		char *__mptr = (void *)(ptr);				\
		((type *)(__mptr - offsetof(type, member))); })
 
static memory_node_t *memory_node_find_by_address(rbtree_t *tree, const void *address,
					 uint64_t size)
{
	memory_node_t *cur = NULL;
	rbtree_key_t key = rbtree_key((unsigned long)address, size);
	uint64_t start;
	uint64_t s;
 
	/* rbtree_lookup_nearest(,,,RIGHT) will return a node with
	 * its size >= key.size and its address >= key.address
	 * if there are two nodes with format(address, size),
	 * (0x100, 16) and (0x110, 8). the key is (0x100, 0),
	 * then node (0x100, 16) will be returned.
	 */
	rbtree_node_t *n = rbtree_lookup_nearest(tree, &key, LKP_ALL, RIGHT);
 
	if (n) {
		cur = container_of(n, memory_node_t, rbtree_node);
		start = (uint64_t)cur->start;
		s = cur->size;
 
		if (start != (uint64_t)address)
			return NULL;
 
		if (size)
			return size == s ? cur : NULL;
 
		/* size is 0, make sure there is only one node whose address == key.address*/
		key = rbtree_key((unsigned long)address, (unsigned long)-1);
		rbtree_node_t *rn = rbtree_lookup_nearest(tree, &key, LKP_ALL, LEFT);
 
		if (rn != n)
			return NULL;
	}
 
	return cur; /* NULL if not found */
}
 
 
int main(int argc, char **argv)
{
	rbtree_t tree;
	memory_node_t *node;
	memory_node_t *node2;
	memory_node_t *get_node;
	memory_node_t *get_node2;
	uint64_t addr; 
	uint64_t size;
 
	//树初始化
	rbtree_init(&tree);
 
	//节点和节点key创建
	addr = 0x40000000;
	size = 0x1000;
	node = (memory_node_t *)malloc(sizeof(memory_node_t));
	node->start = addr;
	node->size = size;
	node->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node->rbtree_node));
 
	//节点和节点key创建
	addr = 0x80000000;
	size = 0x2000;
	node2 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node2->start = addr;
	node2->size = size;
	node2->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node2->rbtree_node));
 
 
	get_node = memory_node_find_by_address(&tree, (const void *)0x40000000, 0x1000);
	printf("node1,start:%lx,size:%lx\n", get_node->start, get_node->size);
 
	get_node2 = memory_node_find_by_address(&tree, (const void *)0x80000000, 0x2000);
	printf("node2,start:%lx,size:%lx\n", get_node2->start, get_node2->size);
	
	return 0;
}
