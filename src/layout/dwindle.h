#ifndef __DWINDLE_H__
#define __DWINDLE_H__

#include "../mango.h"

DwindleNode *dwindle_locked_h_node = NULL;
DwindleNode *dwindle_locked_v_node = NULL;

static DwindleNode *dwindle_new_leaf(Client *c) {
	DwindleNode *n = calloc(1, sizeof(DwindleNode));
	n->client = c;
	return n;
}

static int count_block_items(DwindleNode *node, bool split_h);
static int get_block_path_and_ratios(DwindleNode *target, bool split_h,
									 DwindleNode ***path, float **p);
static DwindleNode *dwindle_find_leaf(DwindleNode *node, Client *c);
static DwindleNode *dwindle_first_leaf(DwindleNode *node);
static void dwindle_free_tree(DwindleNode *node);
static void dwindle_remove(DwindleNode **root, Client *c);
void dwindle_insert(DwindleNode **root, Client *new_c, Client *focused,
						   float ratio, bool as_first, bool split_h,
						   bool lock);
static void dwindle_assign(DwindleNode *node, int32_t ax, int32_t ay,
						   int32_t aw, int32_t ah, int32_t gap_h,
						   int32_t gap_v);
static void dwindle_move_client(DwindleNode **root, Client *c, Client *target,
								float ratio, int32_t dir);
static void dwindle_swap_clients(Client *c1, Client *c2);
static void dwindle_resize_client(Monitor *m, Client *c);
static void dwindle_resize_client_step(Monitor *m, Client *c, int32_t dx,
									   int32_t dy);
static void dwindle_remove_client(Client *c);
static void dwindle_insert_with_config(DwindleNode **root, Client *new_c,
									   Client *focused, float ratio);
void dwindle(Monitor *m);
void cleanup_monitor_dwindle(Monitor *m);


#endif
