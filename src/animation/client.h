#ifndef __ANIMATION_CLIENT_H__
#define __ANIMATION_CLIENT_H__ 1

#include "../mango.h"

bool client_is_ignore_output_clip(Client *c);
struct ivec2 compute_edge_offsets(Client *c);

void client_actual_size(Client *client, int32_t *width, int32_t *height);
void set_rect_size(struct wlr_scene_rect *rect, int32_t width, int32_t height);

struct fx_corner_radii set_client_corner_location(Client *client);

bool is_horizontal_stack_layout(Monitor *monitor);
bool is_horizontal_right_stack_layout(Monitor *monitor);

int32_t is_special_animation_rule(Client *client);
void set_overview_enter_animation(Client *c);
void set_client_open_animation(Client *c, struct wlr_box geo);

struct fx_corner_radii set_client_corner_location(Client *c) {
	struct fx_corner_radii current_corner_location =
		corner_radii_all(config.border_radius);

	if (client_is_ignore_output_clip(c))
		return current_corner_location;

	struct wlr_box target_geom =
		config.animations ? c->animation.current : c->geom;
	if (target_geom.x + config.border_radius <= c->mon->m.x) {
		current_corner_location.top_left = 0;
		current_corner_location.bottom_left = 0;
	}
	if (target_geom.x + target_geom.width - config.border_radius >=
		c->mon->m.x + c->mon->m.width) {
		current_corner_location.top_right = 0;
		current_corner_location.bottom_right = 0;
	}
	if (target_geom.y + config.border_radius <= c->mon->m.y) {
		current_corner_location.top_left = 0;
		current_corner_location.top_right = 0;
	}
	if (target_geom.y + target_geom.height - config.border_radius >=
		c->mon->m.y + c->mon->m.height) {
		current_corner_location.bottom_left = 0;
		current_corner_location.bottom_right = 0;
	}
	return current_corner_location;
}

bool is_horizontal_stack_layout(Monitor *m) {
	uint32_t tag = get_mon_curtag(m);
	return m->pertag->ltidxs[tag]->id == TILE ||
		   m->pertag->ltidxs[tag]->id == DECK;
}

bool is_horizontal_right_stack_layout(Monitor *m) {
	uint32_t tag = get_mon_curtag(m);
	return m->pertag->ltidxs[tag]->id == RIGHT_TILE;
}

int32_t is_special_animation_rule(Client *c) {
	if (is_scroller_layout(c->mon) && !c->isfloating) {
		return DOWN;
	} else if (c->mon->visible_tiling_clients == 1 && !c->isfloating) {
		return DOWN;
	} else if (c->mon->visible_tiling_clients == 2 && !c->isfloating &&
			   !config.new_is_master && is_horizontal_stack_layout(c->mon)) {
		return RIGHT;
	} else if (!c->isfloating && config.new_is_master &&
			   is_horizontal_stack_layout(c->mon)) {
		return LEFT;
	} else if (c->mon->visible_tiling_clients == 2 && !c->isfloating &&
			   !config.new_is_master &&
			   is_horizontal_right_stack_layout(c->mon)) {
		return LEFT;
	} else if (!c->isfloating && config.new_is_master &&
			   is_horizontal_right_stack_layout(c->mon)) {
		return RIGHT;
	} else {
		return UNDIR;
	}
}

void set_overview_enter_animation(Client *c) {
	struct wlr_box geo = c->geom;
	c->animainit_geom.width = geo.width * 1.2;
	c->animainit_geom.height = geo.height * 1.2;
	c->animainit_geom.x = geo.x + (geo.width - c->animainit_geom.width) / 2;
	c->animainit_geom.y = geo.y + (geo.height - c->animainit_geom.height) / 2;
}

void set_client_open_animation(Client *c, struct wlr_box geo) {
	int32_t slide_direction;
	int32_t horizontal, horizontal_value;
	int32_t vertical, vertical_value;
	int32_t special_direction;
	int32_t center_x, center_y;

	if ((!c->animation_type_open &&
		 strcmp(config.animation_type_open, "fade") == 0) ||
		(c->animation_type_open &&
		 strcmp(c->animation_type_open, "fade") == 0)) {
		c->animainit_geom.width = geo.width;
		c->animainit_geom.height = geo.height;
		c->animainit_geom.x = geo.x;
		c->animainit_geom.y = geo.y;
		return;
	} else if ((!c->animation_type_open &&
				strcmp(config.animation_type_open, "zoom") == 0) ||
			   (c->animation_type_open &&
				strcmp(c->animation_type_open, "zoom") == 0)) {
		c->animainit_geom.width = geo.width * config.zoom_initial_ratio;
		c->animainit_geom.height = geo.height * config.zoom_initial_ratio;
		c->animainit_geom.x = geo.x + (geo.width - c->animainit_geom.width) / 2;
		c->animainit_geom.y =
			geo.y + (geo.height - c->animainit_geom.height) / 2;
		return;
	} else {
		special_direction = is_special_animation_rule(c);
		center_x = c->geom.x + c->geom.width / 2;
		center_y = c->geom.y + c->geom.height / 2;
		if (special_direction == UNDIR) {
			horizontal = c->mon->w.x + c->mon->w.width - center_x <
								 center_x - c->mon->w.x
							 ? RIGHT
							 : LEFT;
			horizontal_value = horizontal == LEFT
								   ? center_x - c->mon->w.x
								   : c->mon->w.x + c->mon->w.width - center_x;
			vertical = c->mon->w.y + c->mon->w.height - center_y <
							   center_y - c->mon->w.y
						   ? DOWN
						   : UP;
			vertical_value = vertical == UP
								 ? center_y - c->mon->w.y
								 : c->mon->w.y + c->mon->w.height - center_y;
			slide_direction =
				horizontal_value < vertical_value ? horizontal : vertical;
		} else {
			slide_direction = special_direction;
		}
		c->animainit_geom.width = c->geom.width;
		c->animainit_geom.height = c->geom.height;
		switch (slide_direction) {
		case UP:
			c->animainit_geom.x = c->geom.x;
			c->animainit_geom.y = c->mon->m.y - c->geom.height;
			break;
		case DOWN:
			c->animainit_geom.x = c->geom.x;
			c->animainit_geom.y =
				c->geom.y + c->mon->m.height - (c->geom.y - c->mon->m.y);
			break;
		case LEFT:
			c->animainit_geom.x = c->mon->m.x - c->geom.width;
			c->animainit_geom.y = c->geom.y;
			break;
		case RIGHT:
			c->animainit_geom.x =
				c->geom.x + c->mon->m.width - (c->geom.x - c->mon->m.x);
			c->animainit_geom.y = c->geom.y;
			break;
		default:
			c->animainit_geom.x = c->geom.x;
			c->animainit_geom.y = 0 - c->geom.height;
		}
	}
}
>>>>>>> c62e01c303b8e19598df0b0b7cd16e466a71fc21

void snap_scene_buffer_apply_effect(struct wlr_scene_buffer *buffer, int32_t sx,
									int32_t sy, void *data);
void scene_buffer_apply_effect(struct wlr_scene_buffer *buffer, int32_t sx,
							   int32_t sy, void *data);
void buffer_set_effect(Client *c, BufferData data);

void client_draw_shadow(Client *c, struct ivec2 offsets);
void client_draw_groupbar(Client *c, struct ivec2 offsets);
void global_draw_group_bar(Client *c, int32_t x, int32_t y, int32_t width,
						   int32_t height);
void client_draw_shield(Client *c, struct ivec2 clip_box);
void client_draw_blur(Client *c, struct ivec2 clip_box);
void client_draw_split_border(Client *c, bool hit_no_border,
<<<<<<< HEAD
							  struct ivec2 offsets);
void client_draw_border(Client *c, struct ivec2 offsets);
struct ivec2 clip_to_hide(Client *c, struct wlr_box *clip_box,
						  struct ivec2 offsets);
void client_set_drop_area(Client *c);
=======
							  struct ivec2 offsets) {
	if (c->iskilling || !c->mon || !client_surface(c)->mapped)
		return;

	uint32_t tag = get_client_tag_idx(c);
	const Layout *layout = c->mon->pertag->ltidxs[tag];

	if (hit_no_border || !ISTILED(c) || layout->id != DWINDLE ||
		!config.dwindle_manual_split || c->isfullscreen) {
		if (c->splitindicator[0]->node.enabled)
			wlr_scene_node_set_enabled(&c->splitindicator[0]->node, false);
		if (c->splitindicator[1]->node.enabled)
			wlr_scene_node_set_enabled(&c->splitindicator[1]->node, false);
		return;
	}

	DwindleNode **root = &c->mon->pertag->dwindle_root[tag];
	DwindleNode *dnode = dwindle_find_leaf(*root, c);
	if (!dnode) {
		wlr_scene_node_set_enabled(&c->splitindicator[0]->node, false);
		wlr_scene_node_set_enabled(&c->splitindicator[1]->node, false);
		return;
	}

	if (dnode->custom_leaf_split_h) {
		wlr_scene_node_set_enabled(&c->splitindicator[0]->node, false);
		wlr_scene_node_set_enabled(&c->splitindicator[1]->node, true);
	} else {
		wlr_scene_node_set_enabled(&c->splitindicator[0]->node, true);
		wlr_scene_node_set_enabled(&c->splitindicator[1]->node, false);
	}

	struct wlr_box fullgeom = c->animation.current;
	int32_t bw = (int32_t)c->bw;
	int32_t left = offsets.x, right = offsets.width, top = offsets.y,
			bottom = offsets.height;

	int32_t border_down_width =
		GEZERO(fullgeom.width - 2 * config.border_radius -
			   GEZERO((left + right) - config.border_radius));
	int32_t border_down_height =
		GEZERO(bw - bottom - GEZERO(top + bw - fullgeom.height));

	int32_t border_right_width =
		GEZERO(bw - right - GEZERO(left + bw - fullgeom.width));
	int32_t border_right_height =
		GEZERO(fullgeom.height - 2 * config.border_radius -
			   GEZERO((top + bottom) - config.border_radius));

	int32_t border_down_x =
		GEZERO(config.border_radius + GEZERO(left - config.border_radius));
	int32_t border_down_y =
		GEZERO(fullgeom.height - bw) + GEZERO(top + bw - fullgeom.height);

	int32_t border_right_x =
		GEZERO(fullgeom.width - bw) + GEZERO(left + bw - fullgeom.width);
	int32_t border_right_y =
		GEZERO(config.border_radius + GEZERO(top - config.border_radius));

	set_rect_size(c->splitindicator[0], border_down_width, border_down_height);
	set_rect_size(c->splitindicator[1], border_right_width,
				  border_right_height);
	wlr_scene_node_set_position(&c->splitindicator[0]->node, border_down_x,
								border_down_y);
	wlr_scene_node_set_position(&c->splitindicator[1]->node, border_right_x,
								border_right_y);
}

void client_draw_border(Client *c, struct ivec2 offsets) {
	if (!c || c->iskilling || !client_surface(c)->mapped)
		return;

	if (c->isfullscreen) {
		if (c->border->node.enabled) {
			wlr_scene_node_set_enabled(&c->splitindicator[0]->node, false);
			wlr_scene_node_set_enabled(&c->splitindicator[1]->node, false);
			wlr_scene_node_set_enabled(&c->border->node, false);
			wlr_scene_node_set_position(&c->scene_surface->node, 0, 0);
		}
		return;
	} else {
		if (!c->border->node.enabled)
			wlr_scene_node_set_enabled(&c->border->node, true);
	}

	bool hit_no_border = check_hit_no_border(c);
	client_draw_split_border(c, hit_no_border, offsets);

	struct fx_corner_radii current_corner_location =
		c->isfullscreen || (config.no_radius_when_single && c->mon &&
							c->mon->visible_tiling_clients == 1)
			? corner_radii_none()
			: set_client_corner_location(c);

	if (hit_no_border) {
		c->bw = 0;
		c->fake_no_border = true;
	} else if (!c->isfullscreen && VISIBLEON(c, c->mon)) {
		c->bw = c->isnoborder ? 0 : config.borderpx;
		c->fake_no_border = false;
	}

	struct wlr_box cur = c->animation.current;
	int32_t bw = (int32_t)c->bw;
	int32_t left = offsets.x, right = offsets.width, top = offsets.y,
			bottom = offsets.height;

	int32_t inner_surface_width = GEZERO(cur.width - 2 * bw);
	int32_t inner_surface_height = GEZERO(cur.height - 2 * bw);
	int32_t inner_surface_x = GEZERO(bw - left);
	int32_t inner_surface_y = GEZERO(bw - top);

	int32_t rect_x = left;
	int32_t rect_y = top;
	int32_t rect_width = GEZERO(cur.width - left - right);
	int32_t rect_height = GEZERO(cur.height - top - bottom);

	if (left > c->bw)
		inner_surface_width = inner_surface_width - left + bw;
	if (top > c->bw)
		inner_surface_height = inner_surface_height - top + bw;
	if (right > 0)
		inner_surface_width = MANGO_MIN(cur.width, inner_surface_width + right);
	if (bottom > 0)
		inner_surface_height =
			MANGO_MIN(cur.height, inner_surface_height + bottom);

	struct clipped_region clipped_region = {
		.area = {inner_surface_x, inner_surface_y, inner_surface_width,
				 inner_surface_height},
		.corners = current_corner_location,
	};

	wlr_scene_node_set_position(&c->scene_surface->node, c->bw, c->bw);
	wlr_scene_rect_set_size(c->border, rect_width, rect_height);
	wlr_scene_node_set_position(&c->border->node, rect_x, rect_y);
	wlr_scene_rect_set_corner_radii(c->border, current_corner_location);
	wlr_scene_rect_set_clipped_region(c->border, clipped_region);
}

struct ivec2 clip_to_hide(Client *c, struct wlr_box *clip_box,
						  struct ivec2 offsets) {
	struct ivec2 offset = {0};

	if (!ISSCROLLTILED(c) && !c->animation.tagining && !c->animation.tagouted &&
		!c->animation.tagouting)
		return offset;

	int32_t bw = (int32_t)c->bw;
	int32_t left = offsets.x, right = offsets.width, top = offsets.y,
			bottom = offsets.height;

	if (left > 0) {
		offset.x = GEZERO(left - bw);
		clip_box->x += offset.x;
		clip_box->width -= offset.x;
	} else if (right > 0) {
		offset.width = GEZERO(right - bw);
		clip_box->width -= offset.width;
	}

	if (top > 0) {
		offset.y = GEZERO(top - bw);
		clip_box->y += offset.y;
		clip_box->height -= offset.y;
	} else if (bottom > 0) {
		offset.height = GEZERO(bottom - bw);
		clip_box->height -= offset.height;
	}

	if ((clip_box->width + bw <= 0 || clip_box->height + bw <= 0) &&
		(ISSCROLLTILED(c) || c->animation.tagouting || c->animation.tagining)) {
		c->is_clip_to_hide = true;
		wlr_scene_node_set_enabled(&c->scene->node, false);
	} else if (c->is_clip_to_hide && VISIBLEON(c, c->mon)) {
		c->is_clip_to_hide = false;
		c->is_logic_hide = false;
		wlr_scene_node_set_enabled(&c->scene->node, true);
	}

	return offset;
}

void client_set_drop_area(Client *c) {
	bool first_draw = false;
	int32_t drop_direction = UNDIR;

	if (!c || !c->mon)
		return;

	if (!c->enable_drop_area_draw && !c->droparea->node.enabled)
		return;

	if (!c->enable_drop_area_draw && c->droparea->node.enabled) {
		wlr_scene_node_lower_to_bottom(&c->droparea->node);
		wlr_scene_node_set_enabled(&c->droparea->node, false);
		return;
	} else if (c->enable_drop_area_draw && !c->droparea->node.enabled) {
		wlr_scene_node_raise_to_top(&c->droparea->node);
		wlr_scene_node_set_enabled(&c->droparea->node, true);
		first_draw = true;
	}

	int32_t bw = (int32_t)c->bw;
	int32_t client_width = c->geom.width - 2 * bw;
	int32_t client_height = c->geom.height - 2 * bw;

	double rel_x = cursor->x - c->geom.x - bw;
	double rel_y = cursor->y - c->geom.y - bw;

	struct wlr_box drop_box;
	const Layout *cur_layout = c->mon->pertag->ltidxs[get_client_tag_idx(c)];
	bool dwindle_familiar =
		cur_layout->id == DWINDLE && config.dwindle_drop_simple_split;

	if (dwindle_familiar) {
		bool split_h = c->geom.width >= c->geom.height;
		float ratio = config.dwindle_split_ratio;
		if (split_h) {
			if (rel_x < client_width * 0.5) {
				drop_direction = LEFT;
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = (int32_t)(client_width * ratio);
				drop_box.height = client_height;
			} else {
				drop_direction = RIGHT;
				drop_box.x = bw + (int32_t)(client_width * ratio);
				drop_box.y = bw;
				drop_box.width = client_width - (int32_t)(client_width * ratio);
				drop_box.height = client_height;
			}
		} else {
			if (rel_y < client_height * 0.5) {
				drop_direction = UP;
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = client_width;
				drop_box.height = (int32_t)(client_height * ratio);
			} else {
				drop_direction = DOWN;
				drop_box.x = bw;
				drop_box.y = bw + (int32_t)(client_height * ratio);
				drop_box.width = client_width;
				drop_box.height =
					client_height - (int32_t)(client_height * ratio);
			}
		}
	} else if (cur_layout->id == TILE || cur_layout->id == DECK ||
			   cur_layout->id == CENTER_TILE || cur_layout->id == RIGHT_TILE) {
		if (c->ismaster) {
			if (c->mon->visible_tiling_clients == 1) {
				if (rel_x < client_width * 0.5) {
					drop_direction = LEFT;
					drop_box.x = bw;
					drop_box.y = bw;
					drop_box.width = client_width / 2;
					drop_box.height = client_height;
				} else {
					drop_direction = RIGHT;
					drop_box.x = bw + client_width / 2;
					drop_box.y = bw;
					drop_box.width = client_width / 2;
					drop_box.height = client_height;
				}
			} else {
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = client_width;
				drop_box.height = client_height;
				drop_direction = UNDIR;
			}
		} else {
			if (rel_y < client_height * 0.5) {
				drop_direction = UP;
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = client_width;
				drop_box.height = client_height / 2;
			} else {
				drop_direction = DOWN;
				drop_box.x = bw;
				drop_box.y = bw + client_height / 2;
				drop_box.width = client_width;
				drop_box.height = client_height / 2;
			}
		}
	} else if (cur_layout->id == VERTICAL_TILE ||
			   cur_layout->id == VERTICAL_DECK) {
		if (c->ismaster) {
			if (c->mon->visible_tiling_clients == 1) {
				if (rel_y < client_height * 0.5) {
					drop_direction = UP;
					drop_box.x = bw;
					drop_box.y = bw;
					drop_box.width = client_width;
					drop_box.height = client_height / 2;
				} else {
					drop_direction = DOWN;
					drop_box.x = bw;
					drop_box.y = bw + client_height / 2;
					drop_box.width = client_width;
					drop_box.height = client_height / 2;
				}
			} else {
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = client_width;
				drop_box.height = client_height;
				drop_direction = UNDIR;
			}
		} else {
			if (rel_x < client_width * 0.5) {
				drop_direction = LEFT;
				drop_box.x = bw;
				drop_box.y = bw;
				drop_box.width = client_width / 2;
				drop_box.height = client_height;
			} else {
				drop_direction = RIGHT;
				drop_box.x = bw + client_width / 2;
				drop_box.y = bw;
				drop_box.width = client_width / 2;
				drop_box.height = client_height;
			}
		}
	} else {
		double dist_left = rel_x;
		double dist_right = client_width - rel_x;
		double dist_top = rel_y;
		double dist_bottom = client_height - rel_y;

		if (dist_left <= dist_right && dist_left <= dist_top &&
			dist_left <= dist_bottom) {
			drop_direction = LEFT;
			drop_box.x = bw;
			drop_box.y = bw;
			drop_box.width = client_width / 2;
			drop_box.height = client_height;
		} else if (dist_right <= dist_top && dist_right <= dist_bottom) {
			drop_direction = RIGHT;
			drop_box.x = bw + client_width / 2;
			drop_box.y = bw;
			drop_box.width = client_width / 2;
			drop_box.height = client_height;
		} else if (dist_top <= dist_bottom) {
			drop_direction = UP;
			drop_box.x = bw;
			drop_box.y = bw;
			drop_box.width = client_width;
			drop_box.height = client_height / 2;
		} else {
			drop_direction = DOWN;
			drop_box.x = bw;
			drop_box.y = bw + client_height / 2;
			drop_box.width = client_width;
			drop_box.height = client_height / 2;
		}
	}

	if (!first_draw && c->drop_direction == drop_direction)
		return;
	c->drop_direction = drop_direction;

	wlr_scene_node_set_position(&c->droparea->node, drop_box.x, drop_box.y);
	wlr_scene_rect_set_size(c->droparea, drop_box.width, drop_box.height);
}
>>>>>>> c62e01c303b8e19598df0b0b7cd16e466a71fc21

/* ---------- central rendering entry point ---------- */
void client_apply_clip(Client *c, float factor);
void fadeout_client_animation_next_tick(Client *c);
void client_animation_next_tick(Client *c);
void init_fadeout_client(Client *c);

/* 无动画时应用窗口最终状态：位置、裁剪/可见性以及几何状态同步 */
void client_apply_finish_geometry(Client *c);
void client_commit(Client *c);
void client_set_pending_state(Client *c);

typedef struct ResizeOpts {
	bool interact;			 // 交互式 resize（鼠标拖动调整）
	bool skip_ov_enter_anim; // 预排阶段：跳过 overview 进入放大
} ResizeOpts;

void resize_apply(Client *c, struct wlr_box geo, ResizeOpts opts);
void resize(Client *c, struct wlr_box geo, int32_t interact);
bool client_draw_fadeout_frame(Client *c);
void client_set_focused_opacity_animation(Client *c);
void client_set_unfocused_opacity_animation(Client *c);
bool client_apply_focus_opacity(Client *c);
bool client_draw_frame(Client *c);

<<<<<<< HEAD
#endif
=======
	if (!c->mon)
		return;

	c->need_output_flush = true;
	c->dirty = true;

	struct wlr_box *bbox = (opts.interact || c->isfloating || c->isfullscreen)
							   ? &sgeom
							   : &c->mon->w;
	struct wlr_box clip;

	if (is_scroller_layout(c->mon) && (!c->isfloating || c == grabc)) {
		c->geom = geo;
		c->geom.width = MANGO_MAX(1 + 2 * (int32_t)c->bw, c->geom.width);
		c->geom.height = MANGO_MAX(1 + 2 * (int32_t)c->bw, c->geom.height);
	} else {
		c->geom = geo;
		applybounds(c, bbox);
	}

	if (!c->isnosizehint && !c->ismaximizescreen && !c->isfullscreen &&
		c->isfloating)
		client_set_size_bound(c);

	if (!c->is_pending_open_animation)
		c->animation.begin_fade_in = false;

	if (c->animation.overining)
		c->animation.action = OVERVIEW;
	else if (c->animation.action == OPEN && !c->animation.tagining &&
			 !c->animation.tagouting && wlr_box_equal(&c->geom, &c->current))
		; /* keep current action */
	else if (c->animation.tagouting) {
		c->animation.duration = config.animation_duration_tag;
		c->animation.action = TAG;
	} else if (c->animation.tagining) {
		c->animation.duration = config.animation_duration_tag;
		c->animation.action = TAG;
	} else if (c->is_pending_open_animation) {
		c->animation.duration = config.animation_duration_open;
		c->animation.action = OPEN;
	} else {
		c->animation.duration = config.animation_duration_move;
		c->animation.action = MOVE;
	}

	if (c->animation.tagouting)
		c->animainit_geom = c->animation.current;
	else if (c->animation.tagining) {
		c->animainit_geom.height = c->animation.current.height;
		c->animainit_geom.width = c->animation.current.width;
	} else if (c->is_pending_open_animation)
		set_client_open_animation(c, c->geom);
	else
		c->animainit_geom = c->animation.current;

	if (c->isnoborder || c->iskilling)
		c->bw = 0;

	bool hit_no_border = check_hit_no_border(c);
	if (hit_no_border) {
		c->bw = 0;
		c->fake_no_border = true;
	}

	if (!c->mon->isoverview)
		c->configure_serial = client_set_size(c, c->geom.width - 2 * c->bw,
											  c->geom.height - 2 * c->bw);

	if (c->configure_serial != 0)
		c->mon->resizing_count_pending++;

	if (c == grabc) {
		struct ivec2 offsets = compute_edge_offsets(c);

		c->animation.running = false;
		c->need_output_flush = false;
		c->animainit_geom = c->current = c->pending = c->animation.current =
			c->geom;
		wlr_scene_node_set_position(&c->scene->node, c->geom.x, c->geom.y);

		client_draw_border(c, offsets);
		client_get_clip(c, &clip);
		client_draw_groupbar(c, offsets);
		client_draw_shadow(c, offsets);

		struct ivec2 surface_clip_offset = clip_to_hide(c, &clip, offsets);
		client_draw_shield(c, surface_clip_offset);
		client_draw_blur(c, surface_clip_offset);

		if (client_is_x11(c))
			client_update_xwayland_clip(c, &clip);
		else
			wlr_scene_subsurface_tree_set_clip(&c->scene_surface->node, &clip);
		return;
	}

	if (!c->animation.tagouting && !c->iskilling)
		c->pending = c->geom;

	if (c->swallowing && c->animation.action == OPEN)
		c->animainit_geom = c->swallowing->animation.current;

	if (c->swallowdby)
		c->animainit_geom = c->geom;

	if ((c->isglobal || c->isunglobal) && c->isfloating &&
		c->animation.action == TAG)
		c->animainit_geom = c->geom;

	if (c->scratchpad_switching_mon && c->isfloating)
		c->animainit_geom = c->geom;

	if (!opts.skip_ov_enter_anim) {
		/* 清除进入动画标志（含焦点窗口），避免切换焦点时重复放大 */
		if (config.animations && c->mon->isoverview && c->animation.overining &&
			!c->animation.overview_enter_anim_set)
			c->animation.overining = false;

		/* 设置进入放大动画：除 sel 外的窗口 */
		if (config.animations && c->mon->isoverview && c != c->mon->sel &&
			c->animation.action == OVERVIEW &&
			!c->animation.overview_enter_anim_set) {
			c->animation.overview_enter_anim_set = true;
			set_overview_enter_animation(c);
		}
	}

	client_set_pending_state(c);
	setborder_color(c);
}

void resize(Client *c, struct wlr_box geo, int32_t interact) {
	resize_apply(c, geo, (ResizeOpts){.interact = interact});
}

bool client_draw_fadeout_frame(Client *c) {
	if (!c)
		return false;

	fadeout_client_animation_next_tick(c);
	return true;
}

void client_set_focused_opacity_animation(Client *c) {
	float *border_color = get_border_color(c);
	wlr_scene_node_lower_to_bottom(&c->border->node);

	if (!config.animations) {
		setborder_color(c);
		return;
	}

	c->opacity_animation.duration = config.animation_duration_focus;
	memcpy(c->opacity_animation.target_border_color, border_color,
		   sizeof(c->opacity_animation.target_border_color));
	c->opacity_animation.target_opacity = c->focused_opacity;
	c->opacity_animation.time_started = get_now_in_ms();
	memcpy(c->opacity_animation.initial_border_color,
		   c->opacity_animation.current_border_color,
		   sizeof(c->opacity_animation.initial_border_color));
	c->opacity_animation.initial_opacity = c->opacity_animation.current_opacity;
	c->opacity_animation.running = true;
}

void client_set_unfocused_opacity_animation(Client *c) {
	float *border_color = get_border_color(c);
	wlr_scene_node_raise_to_top(&c->border->node);
	if (!config.animations) {
		setborder_color(c);
		return;
	}

	c->opacity_animation.duration = config.animation_duration_focus;
	memcpy(c->opacity_animation.target_border_color, border_color,
		   sizeof(c->opacity_animation.target_border_color));
	c->opacity_animation.target_opacity = c->unfocused_opacity;
	c->opacity_animation.time_started = get_now_in_ms();
	memcpy(c->opacity_animation.initial_border_color,
		   c->opacity_animation.current_border_color,
		   sizeof(c->opacity_animation.initial_border_color));
	c->opacity_animation.initial_opacity = c->opacity_animation.current_opacity;
	c->opacity_animation.running = true;
}

bool client_apply_focus_opacity(Client *c) {

	if (config.blur && !c->noblur && c->blur_opacity != 1.0f &&
		c->animation.action != OPEN) {
		c->blur_opacity = 1.0f;
		wlr_scene_blur_set_strength(c->blur, 1.0f);
		wlr_scene_blur_set_alpha(c->blur, 1.0f);
	}

	float *border_color = get_border_color(c);
	if (c->isfullscreen) {
		c->opacity_animation.running = false;
		client_set_opacity(c, 1);
	} else if (c->animation.running && c->animation.action == OPEN) {
		c->opacity_animation.running = false;
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);

		int32_t passed_time = timespec_to_ms(&now) - c->animation.time_started;
		double linear_progress =
			c->animation.duration
				? (double)passed_time / (double)c->animation.duration
				: 1.0;

		double opacity_eased_progress =
			find_animation_curve_at(linear_progress, OPAFADEIN);
		float percent = config.animation_fade_in && !c->nofadein
							? opacity_eased_progress
							: 1.0;
		float opacity =
			c == selmon->sel ? c->focused_opacity : c->unfocused_opacity;
		float target_opacity = percent * (1.0 - config.fadein_begin_opacity) +
							   config.fadein_begin_opacity;
		if (target_opacity > opacity)
			target_opacity = opacity;

		memcpy(c->opacity_animation.current_border_color,
			   c->opacity_animation.target_border_color,
			   sizeof(c->opacity_animation.current_border_color));
		c->opacity_animation.current_opacity = target_opacity;
		client_set_opacity(c, target_opacity);
		if (config.blur && !c->noblur && !config.blur_optimized) {
			float blur_val = MIN(percent * (1.0 - config.fadein_begin_opacity) +
									 config.fadein_begin_opacity,
								 1.0);
			c->blur_opacity = blur_val;
			wlr_scene_blur_set_strength(c->blur, blur_val);
			wlr_scene_blur_set_alpha(c->blur, blur_val);
		}
		client_set_border_color(c, c->opacity_animation.target_border_color);
	} else if (config.animations && c->opacity_animation.running) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);

		int32_t passed_time =
			timespec_to_ms(&now) - c->opacity_animation.time_started;
		double linear_progress =
			c->opacity_animation.duration
				? (double)passed_time / (double)c->opacity_animation.duration
				: 1.0;

		float eased_progress = find_animation_curve_at(linear_progress, FOCUS);

		c->opacity_animation.current_opacity =
			c->opacity_animation.initial_opacity +
			(c->opacity_animation.target_opacity -
			 c->opacity_animation.initial_opacity) *
				eased_progress;
		client_set_opacity(c, c->opacity_animation.current_opacity);

		for (int32_t i = 0; i < 4; i++) {
			c->opacity_animation.current_border_color[i] =
				c->opacity_animation.initial_border_color[i] +
				(c->opacity_animation.target_border_color[i] -
				 c->opacity_animation.initial_border_color[i]) *
					eased_progress;
		}
		client_set_border_color(c, c->opacity_animation.current_border_color);
		if (linear_progress >= 1.0f)
			c->opacity_animation.running = false;
		else
			return true;
	} else if (c == selmon->sel) {
		c->opacity_animation.running = false;
		c->opacity_animation.current_opacity = c->focused_opacity;
		memcpy(c->opacity_animation.current_border_color, border_color,
			   sizeof(c->opacity_animation.current_border_color));
		client_set_opacity(c, c->focused_opacity);
	} else {
		c->opacity_animation.running = false;
		c->opacity_animation.current_opacity = c->unfocused_opacity;
		memcpy(c->opacity_animation.current_border_color, border_color,
			   sizeof(c->opacity_animation.current_border_color));
		client_set_opacity(c, c->unfocused_opacity);
	}

	return false;
}

bool client_draw_frame(Client *c) {

	bool need_next_tick = false;
	bool force_render = false;

	if (!c || !client_surface(c)->mapped)
		return false;

	// always render when scene is disabled
	if (c->force_render && !c->scene->node.enabled) {
		force_render = client_force_render(c);
		need_next_tick = force_render || need_next_tick;
	}

	if (!c->need_output_flush)
		return client_apply_focus_opacity(c) || need_next_tick;

	if (config.animations && c->animation.running) {
		need_next_tick = true;
		client_animation_next_tick(c);
	} else {
		client_apply_finish_geometry(c);
	}

	bool need_fade_focus = client_apply_focus_opacity(c);

	return need_next_tick || need_fade_focus;
}
