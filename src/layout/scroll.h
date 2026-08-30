#ifndef __SCROLL_H__
#define __SCROLL_H__

#include "../mango.h"
#include <wlroots-0.20/wlr/types/wlr_box.h>

void vertical_scroll_adjust_fullandmax(Client *c, struct wlr_box *target_geom);

void vertical_check_scroller_root_inside_mon(Client *c,
    struct wlr_box *geometry);

void horizontal_scroll_adjust_fullandmax(Client *c,
										 struct wlr_box *target_geom);

void horizontal_check_scroller_root_inside_mon(Client *c,
											   struct wlr_box *geometry);

void arrange_stack_node(struct ScrollerStackNode *head, struct wlr_box geometry,
						int32_t gappiv);

void arrange_stack_vertical_node(struct ScrollerStackNode *head,
								 struct wlr_box geometry, int32_t gappih);

void scroller(Monitor *m);

void vertical_scroller(Monitor *m);

void scroller_remove_client(Client *c);

void scroller_insert_stack(Client *c, Client *target_client,
						   bool insert_before);

Client *scroll_get_stack_head_client(Client *c);

Client *scroll_get_stack_tail_client(Client *c);

void exchange_two_scroller_clients(Client *c1, Client *c2);

#endif
