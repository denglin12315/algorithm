/*
 * Copyright (C) 2002-2018 Igor Sysoev
 * Copyright (C) 2011-2018 Nginx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
#include "rbtree.h"
 
static inline void rbtree_left_rotate(rbtree_node_t **root,
		rbtree_node_t *sentinel, rbtree_node_t *node);
static inline void rbtree_right_rotate(rbtree_node_t **root,
		rbtree_node_t *sentinel, rbtree_node_t *node);
 
static void
rbtree_insert_value(rbtree_node_t *temp, rbtree_node_t *node,
		rbtree_node_t *sentinel)
{
	rbtree_node_t  **p;
 
	for ( ;; ) {
		/* 
		 * 从root节点开始往下搜索， rbtree_key_compare返回值>=0，表示当前是右子树，否则是左子树;
		 * 其中，p是当前node指针
		 */
		p = rbtree_key_compare(LKP_ALL, &node->key, &temp->key) < 0 ?
			&temp->left : &temp->right;
 
		/* 如果已经查找到树的边界，就不应该继续搜索了 */
		if (*p == sentinel) {
			break;				/* 这个for循环只能从这里退出 */
		}
 
		temp = *p;
	}
 
	*p = node;					/* 将当前node插入p指向的位置(temp指向的node的left或者right) */
	node->parent = temp;		/* node的父节点就是temp */
	node->left = sentinel;		/* node的left和right指向了边界节点sentinel */
	node->right = sentinel;
	rbt_red(node);				/* 新插入的节点是red节点 */
}

void 
rbtree_print(rbtree_t *tree)
{
	rbtree_node_t *n = rbtree_node_any(tree, LEFT);			/* 找到最小值节点————从地址最小值节点开始打印 */
	printf("tree nodes:\n");
	printf("addr=%lu, size=%lu, color=%d...", n->key.addr, n->key.size, n->color); 
	if(n != tree->root && n->parent->left == n)
		printf("left son of addr=%lu, size=%lu, color=%d\n", n->parent->key.addr, n->parent->key.size, n->parent->color); 
	if(n != tree->root && n->parent->right == n)
		printf("right son of addr=%lu, size=%lu, color=%d\n", n->parent->key.addr, n->parent->key.size, n->parent->color);
	while (n) {
		n = rbtree_next(tree, n);							/* 寻找下一个要打印的节点 */
		if(n) {
			printf("addr=%lu, size=%lu, color=%d...", n->key.addr, n->key.size, n->color); 
			if(n != tree->root && n->parent->left == n)
				printf("left son of addr=%lu, size=%lu, color=%d\n", n->parent->key.addr, n->parent->key.size, n->parent->color); 
			if(n != tree->root && n->parent->right == n)
				printf("right son of addr=%lu, size=%lu, color=%d\n", n->parent->key.addr, n->parent->key.size, n->parent->color); 
		}
	}
}
 
void
rbtree_insert(rbtree_t *tree, rbtree_node_t *node)
{
	rbtree_node_t  **root, *temp, *sentinel;
 
	root = &tree->root;					//找到树的根节点指针(sentinel节点的指针)
	sentinel = &tree->sentinel;
 
	/* 情形一：空树插入第一个节点 */
	if (*root == sentinel) {
		node->parent = NULL;
		node->left = sentinel;
		node->right = sentinel;
		rbt_black(node);
		*root = node;
 
		return;
	}
 
	/*
	 * 情形二：非空树插入节点————这种情况往往需要re-balance tree，通过下面的while循环来实现
	 * 参数一：当前根节点
	 * 参数二：要插入的节点
	 * 参数三：哨兵节点————树的界限
	 */
	rbtree_insert_value(*root, node, sentinel);
	
	printf("\ninsert value****************************************\n");
	rbtree_print(tree);

	/* 插入完成节点后，紧接着需要re-balance tree, 实际上就是旋转tree */
	
	/* 1.插入的节点不是根节点 并且 2.插入节点的父节点是red(出现连续两个红色的节点)，需要reblance */
	while (node != *root && rbt_is_red(node->parent)) {
		/*
		 *  该情况节点图示：
		 *						 ...
		 *				          /
		 *                   grand_parent
		 * 						/   \
		 * 					   /     \
		 * 			   (red)parent   aunt
		 * 					 /  \
		 * 					/    \
		 * 			 (red)son1 (red)son2
		 * 				
		 * 	
		 * 新插入的node可能出现在son1位置，也有可能出现在son2位置
		 */
		/* parent节点是grand_parent节点的左子节点 */
		if (node->parent == node->parent->parent->left) {
			temp = node->parent->parent->right;			/* temp指向aunt节点 */
 
			/* 如果aunt节点是红色，需要进行颜色变换 */
			if (rbt_is_red(temp)) {
				printf("color filp *************************\n");
				rbt_black(node->parent);		/* parent变成黑色 */
				rbt_black(temp);				/* aunt变成黑色 */
				rbt_red(node->parent->parent);	/* grand_parent变成红色 */
				node = node->parent->parent;	/* 将node指向grand_parent————下次while循环会用到 */
				rbtree_print(tree);
			/* 如果aunt节点是黑色，需要进行旋转 */
			} else {

				/* 如果当前插入节点是right子节点 */
				if (node == node->parent->right) {
					node = node->parent;
					printf("\nbefore left rotate.....\n");
					rbtree_print(tree);
					rbtree_left_rotate(root, sentinel, node);					/* 进行左旋转 */
					printf("\nafter left rotate.....\n");
					rbtree_print(tree);
				}
				/* 如果当前插入节点是left子节点  */
				rbt_black(node->parent);
				rbt_red(node->parent->parent);
				printf("\nbefore right rotate.....\n");
				rbtree_print(tree);
				rbtree_right_rotate(root, sentinel, node->parent->parent);		/* 进行右旋转 */
				printf("\nafter right rotate.....\n");
				rbtree_print(tree);
			}


		/*
		 *  该情况节点图示：
		 *						 ...
		 *				          /
		 *                   grand_parent
		 * 						/   \
		 * 					   /     \
		 * 			         aunt  (red)parent
		 *		 					 /  \
		 * 							/    \
		 * 			 		(red)son1 (red)son2
		 * 				
		 * 	
		 * 新插入的node可能出现在son1位置，也有可能出现在son2位置
		 */
		/* parent节点是grand_parent节点的右子节点 */
		} else {
			temp = node->parent->parent->left;	/* temp指向aunt节点 */
 
			/* 
			 * 如果aunt节点是红色，需要进行颜色变换————因为红黑树的规则之一就是aunt节点是黑色。
			 * 另外一个规则是连续两个节点不能都为红色，此时通过颜色变换可以同时修复这两个规则冲突。
			 */
			if (rbt_is_red(temp)) {
				printf("color filp *************************\n");
				rbt_black(node->parent);
				rbt_black(temp);
				rbt_red(node->parent->parent);
				node = node->parent->parent;			/* 
				  										 * 将node指向grand_parent ———— 下次while循环会用到(因为目前为止，grand_parent作为
				                                         * root的子树已经满足红黑树规则，下一步需要向上检查grand_parent作为叶子节点的子树是否满
														 * 足红黑树规则)
														 */
				rbtree_print(tree);
 
			/*
			 * 如果aunt节点是黑色，需要进行旋转 ———— 因为此时aunt节点已经是黑色,但是仍然违反了连续两个
			 * 节点不能为红色的规则，所以，此时不能用颜色变换的方式，需要用旋转的方式来满足红黑树的规则
			 */
			} else {

				/* 如果当前插入节点是left子节点 */
				if (node == node->parent->left) {
					node = node->parent;
					printf("\nbefore right rotate.....\n");
					rbtree_print(tree);
					rbtree_right_rotate(root, sentinel, node);						/* 基于问题节点的parent_node进行右旋转 */
					printf("\nafter right rotate.....\n");
					rbtree_print(tree);
				}
 
				/* 如果当前插入节点是right子节点 */
				rbt_black(node->parent);
				rbt_red(node->parent->parent);
				printf("\nbefore left rotate.....\n");
				rbtree_print(tree);
				rbtree_left_rotate(root, sentinel, node->parent->parent);			/* 基于问题节点的parent_node进行左旋转 */
				printf("\nafter left rotate.....\n");
				rbtree_print(tree);
			}
		}
	}
 
	rbt_black(*root);
}
 
 
void
rbtree_delete(rbtree_t *tree, rbtree_node_t *node)
{
	unsigned int red;
	rbtree_node_t  **root, *sentinel, *subst, *temp, *w;
 
	/* a binary tree delete */
 
	root = &tree->root;
	sentinel = &tree->sentinel;
 
	if (node->left == sentinel) {
		temp = node->right;
		subst = node;
 
	} else if (node->right == sentinel) {
		temp = node->left;
		subst = node;
 
	} else {
		subst = rbtree_min(node->right, sentinel);
 
		if (subst->left != sentinel) {
			temp = subst->left;
		} else {
			temp = subst->right;
		}
	}
 
	if (subst == *root) {
		*root = temp;
		rbt_black(temp);
 
		return;
	}
 
	red = rbt_is_red(subst);
 
	if (subst == subst->parent->left) {
		subst->parent->left = temp;
 
	} else {
		subst->parent->right = temp;
	}
 
	if (subst == node) {
 
		temp->parent = subst->parent;
 
	} else {
 
		if (subst->parent == node) {
			temp->parent = subst;
 
		} else {
			temp->parent = subst->parent;
		}
 
		subst->left = node->left;
		subst->right = node->right;
		subst->parent = node->parent;
		rbt_copy_color(subst, node);
 
		if (node == *root) {
			*root = subst;
 
		} else {
			if (node == node->parent->left) {
				node->parent->left = subst;
			} else {
				node->parent->right = subst;
			}
		}
 
		if (subst->left != sentinel) {
			subst->left->parent = subst;
		}
 
		if (subst->right != sentinel) {
			subst->right->parent = subst;
		}
	}
 
	if (red) {
		return;
	}
 
	/* a delete fixup */
 
	while (temp != *root && rbt_is_black(temp)) {
 
		if (temp == temp->parent->left) {
			w = temp->parent->right;
 
			if (rbt_is_red(w)) {
				rbt_black(w);
				rbt_red(temp->parent);
				rbtree_left_rotate(root, sentinel, temp->parent);
				w = temp->parent->right;
			}
 
			if (rbt_is_black(w->left) && rbt_is_black(w->right)) {
				rbt_red(w);
				temp = temp->parent;
 
			} else {
				if (rbt_is_black(w->right)) {
					rbt_black(w->left);
					rbt_red(w);
					rbtree_right_rotate(root, sentinel, w);
					w = temp->parent->right;
				}
 
				rbt_copy_color(w, temp->parent);
				rbt_black(temp->parent);
				rbt_black(w->right);
				rbtree_left_rotate(root, sentinel, temp->parent);
				temp = *root;
			}
 
		} else {
			w = temp->parent->left;
 
			if (rbt_is_red(w)) {
				rbt_black(w);
				rbt_red(temp->parent);
				rbtree_right_rotate(root, sentinel, temp->parent);
				w = temp->parent->left;
			}
 
			if (rbt_is_black(w->left) && rbt_is_black(w->right)) {
				rbt_red(w);
				temp = temp->parent;
 
			} else {
				if (rbt_is_black(w->left)) {
					rbt_black(w->right);
					rbt_red(w);
					rbtree_left_rotate(root, sentinel, w);
					w = temp->parent->left;
				}
 
				rbt_copy_color(w, temp->parent);
				rbt_black(temp->parent);
				rbt_black(w->left);
				rbtree_right_rotate(root, sentinel, temp->parent);
				temp = *root;
			}
		}
	}
 
	rbt_black(temp);
}
 
 
static inline void
rbtree_left_rotate(rbtree_node_t **root, rbtree_node_t *sentinel,
		rbtree_node_t *node)
{
	rbtree_node_t  *temp;
 
	temp = node->right;
	node->right = temp->left;
 
	if (temp->left != sentinel) {
		temp->left->parent = node;
	}
 
	temp->parent = node->parent;
 
	if (node == *root) {
		*root = temp;
 
	} else if (node == node->parent->left) {
		node->parent->left = temp;
 
	} else {
		node->parent->right = temp;
	}
 
	temp->left = node;
	node->parent = temp;
}
 
 
static inline void
rbtree_right_rotate(rbtree_node_t **root, rbtree_node_t *sentinel,
		rbtree_node_t *node)
{
	rbtree_node_t  *temp;
 
	temp = node->left;
	node->left = temp->right;
 
	if (temp->right != sentinel) {
		temp->right->parent = node;
	}
 
	temp->parent = node->parent;
 
	if (node == *root) {
		*root = temp;
 
	} else if (node == node->parent->right) {
		node->parent->right = temp;
 
	} else {
		node->parent->left = temp;
	}
 
	temp->right = node;
	node->parent = temp;
}
 
 
 /*
  * 查找整棵树中当前节点的下一个节点,假设整棵树如下：
  *					     8
  *                    /   \
  *					 /       \
  *                 5         10
  *                /  \      /  \
  *               3     7    9  哨兵
  *              / \   / \
  *             1 哨兵 6  哨兵
  * 7的下一个节点是8,8的下一个节点是9,9的下一个节点是10,依此类推。
  * 同理，假设当前节点是1，1->3->5->6->7->8->9->10
  * 同理，假设当前节点是8，8->9->10
  */
rbtree_node_t *
rbtree_next(rbtree_t *tree, rbtree_node_t *node)
{
	rbtree_node_t  *root, *sentinel, *parent;
 
	sentinel = &tree->sentinel;
 
	/* 右子树所有的节点都比当前节点值大，所以当前节点的下一个节点就是右子树中最小的节点 */
	if (node->right != sentinel) {
		return rbtree_min(node->right, sentinel);
	}
 
	/* 父节点的值大于当前节点的值，所以当前节点的下一个节点就是父节点 */
	root = tree->root;
	for ( ;; ) {
		parent = node->parent;

		if (node == root) {
			return NULL;
		}

		if (node == parent->left) {
			return parent;
		}
 
		node = parent;
	}
}

