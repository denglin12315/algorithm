#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "rbtree.h"
 
 /*
  * rbtree管理的“内存节点”数据结构
  */
typedef struct memory_node {
	uint64_t start;					//节点管理虚拟内存的起始地址
	uint64_t size;					//节点管理虚拟内存的大小
	rbtree_node_t rbtree_node;		//红黑树节点————这个成员使得struct memory_node有挂接到rbtree的能力(Linux kernel中常用的继承实现方法)
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
	memory_node_t *node3;
	memory_node_t *node4;
	memory_node_t *node5;
	memory_node_t *node6;
	memory_node_t *node7;
	memory_node_t *node8;
	memory_node_t *node9;
	memory_node_t *node10;
	memory_node_t *node11;
#if 0	
	memory_node_t *get_node;
	memory_node_t *get_node2;
#endif
	uint64_t addr; 
	uint64_t size;
 
	//树初始化
	rbtree_init(&tree);
 
	//节点和节点key创建
	addr = 10;
	size = 1;
	node = (memory_node_t *)malloc(sizeof(memory_node_t));
	node->start = addr;		//填充memory node的start
	node->size = size;		//填充memory node的size
	node->rbtree_node.key = rbtree_key(addr, size);		//填充memory node的key
	//将node插入到tree中
	rbtree_insert(&tree, &(node->rbtree_node));
 
 #if 0
	//节点和节点key创建
	addr = 4;
	size = 1;
	node2 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node2->start = addr;
	node2->size = size;
	node2->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node2->rbtree_node));
#endif

	//节点和节点key创建
	addr = 3;
	size = 1;
	node3 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node3->start = addr;
	node3->size = size;
	node3->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node3->rbtree_node));

	//节点和节点key创建
	addr = 8;
	size = 1;
	node4 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node4->start = addr;
	node4->size = size;
	node4->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node4->rbtree_node));
#if 0
	//节点和节点key创建
	addr = 0;
	size = 1;
	node5 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node5->start = addr;
	node5->size = size;
	node5->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node5->rbtree_node));
#endif
	//节点和节点key创建
	addr = 7;
	size = 1;
	node6 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node6->start = addr;
	node6->size = size;
	node6->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node6->rbtree_node));

	//节点和节点key创建
	addr = 5;
	size = 1;
	node7 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node7->start = addr;
	node7->size = size;
	node7->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node7->rbtree_node));

	//节点和节点key创建
	addr = 9;
	size = 1;
	node8 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node8->start = addr;
	node8->size = size;
	node8->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node8->rbtree_node));

	//节点和节点key创建
	addr = 1;
	size = 1;
	node9 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node9->start = addr;
	node9->size = size;
	node9->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node9->rbtree_node));

#if 0
	//节点和节点key创建
	addr = 2;
	size = 1;
	node10 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node10->start = addr;
	node10->size = size;
	node10->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node10->rbtree_node));
#endif

	//节点和节点key创建
	addr = 6;
	size = 1;
	node11 = (memory_node_t *)malloc(sizeof(memory_node_t));
	node11->start = addr;
	node11->size = size;
	node11->rbtree_node.key = rbtree_key(addr, size);
	//将node插入到tree中
	rbtree_insert(&tree, &(node11->rbtree_node));
 
 #if 0
	get_node = memory_node_find_by_address(&tree, (const void *)0x40000000, 0x1000);
	printf("node1,start:%lx,size:%lx\n", get_node->start, get_node->size);
 
	get_node2 = memory_node_find_by_address(&tree, (const void *)0x80000000, 0x2000);
	printf("node2,start:%lx,size:%lx\n", get_node2->start, get_node2->size);
#endif

	//rbtree_print(&tree);

	return 0;
}
