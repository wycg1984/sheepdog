#ifndef __RBTREE_H_
#define __RBTREE_H_

struct rb_node {
	unsigned long  rb_parent_color;
#define RB_RED          0
#define RB_BLACK        1
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};

struct rb_root {
	struct rb_node *rb_node;
};


#define rb_parent(r)   ((struct rb_node *)((r)->rb_parent_color & ~3))
#define rb_color(r)   ((r)->rb_parent_color & 1)
#define rb_is_red(r)   (!rb_color(r))
#define rb_is_black(r) rb_color(r)
#define rb_set_red(r)  do { (r)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(r)  do { (r)->rb_parent_color |= 1; } while (0)

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}
static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

#define RB_ROOT { .rb_node = NULL }
static inline void INIT_RB_ROOT(struct rb_root *root)
{
	root->rb_node = NULL;
}

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)     ((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node)     (rb_parent(node) == node)
#define RB_CLEAR_NODE(node)     (rb_set_parent(node, node))

static inline void rb_init_node(struct rb_node *rb)
{
	rb->rb_parent_color = 0;
	rb->rb_right = NULL;
	rb->rb_left = NULL;
	RB_CLEAR_NODE(rb);
}

void rb_insert_color(struct rb_node *, struct rb_root *);
void rb_erase(struct rb_node *, struct rb_root *);

typedef void (*rb_augment_f)(struct rb_node *node, void *data);

void rb_augment_insert(struct rb_node *node,
		rb_augment_f func, void *data);
struct rb_node *rb_augment_erase_begin(struct rb_node *node);
void rb_augment_erase_end(struct rb_node *node,
		rb_augment_f func, void *data);

/* Find logical next and previous nodes in a tree */
struct rb_node *rb_next(const struct rb_node *);
struct rb_node *rb_prev(const struct rb_node *);
struct rb_node *rb_first(const struct rb_root *);
struct rb_node *rb_last(const struct rb_root *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
void rb_replace_node(struct rb_node *victim, struct rb_node *new,
		struct rb_root *root);

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
		struct rb_node **rb_link)
{
	node->rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

/*
 * Search for a value in the rbtree.  This returns NULL when the key is not
 * found in the rbtree.
 */
#define rb_search(root, key, member, compar)				\
({									\
	struct rb_node *__n = (root)->rb_node;				\
	typeof(key) __ret = NULL, __data;				\
									\
	while (__n) {							\
		__data = rb_entry(__n, typeof(*key), member);		\
		int __cmp = compar(key, __data);			\
									\
		if (__cmp < 0)						\
			__n = __n->rb_left;				\
		else if (__cmp > 0)					\
			__n = __n->rb_right;				\
		else {							\
			__ret = __data;					\
			break;						\
		}							\
	}								\
	__ret;								\
})

/*
 * Insert a new node into the rbtree.  This returns NULL on success, or the
 * existing node on error.
 */
#define rb_insert(root, new, member, compar)				\
({									\
	struct rb_node **__n = &(root)->rb_node, *__parent = NULL;	\
	typeof(new) __old = NULL, __data;				\
									\
	while (*__n) {							\
		__data = rb_entry(*__n, typeof(*new), member);		\
		int __cmp = compar(new, __data);			\
									\
		__parent = *__n;					\
		if (__cmp < 0)						\
			__n = &((*__n)->rb_left);			\
		else if (__cmp > 0)					\
			__n = &((*__n)->rb_right);			\
		else {							\
			__old = __data;					\
			break;						\
		}							\
	}								\
									\
	if (__old == NULL) {						\
		/* Add new node and rebalance tree. */			\
		rb_link_node(&((new)->member), __parent, __n);		\
		rb_insert_color(&((new)->member), root);		\
	}								\
									\
	__old;							\
})

/* Iterate over a rbtree safe against removal of rbnode */
#define rb_for_each(pos, root)					\
	pos = rb_first(root);					\
	for (struct rb_node *__n = rb_next(pos);		\
	     pos && ({ __n = rb_next(pos); 1; });		\
	     pos = __n)

/* Iterate over a rbtree of given type safe against removal of rbnode */
#define rb_for_each_entry(pos, root, member)				\
	for (struct rb_node *__pos = rb_first(root), *__n;		\
	     __pos && ({ __n = rb_next(__pos); 1; }) &&			\
		     ({ pos =  rb_entry(__pos, typeof(*pos), member); 1; }); \
	     __pos = __n)

#endif /* __RBTREE_H_ */
