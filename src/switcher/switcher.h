#ifndef __SWITCHER_SWITCHER_H__
#define __SWITCHER_SWITCHER_H__ 1

#include "../mango.h"

#define SW_PAD 6
#define SW_GAP 14
#define SW_MARGIN 28
#define SW_PANEL_FRAC 0.8f
#define SW_TILE_FRAC 0.25f
#define SW_ASPECT_MIN (9.0f / 16.0f)
#define SW_ASPECT_MAX (12.0f / 5.0f)

extern const float switcher_panel_color[4];

struct switcher_tile {
	Client *c;
	struct wlr_scene_tree *tree;
	struct wlr_scene_rect *frame;
	struct wlr_scene_tree *content;
	struct wl_list surfaces; /* switcher_surface.link */
	int cw;
	int row;
};

struct switcher_surface {
	struct switcher_tile *tile;
	struct wlr_surface *surface;
	struct wlr_scene_buffer *buffer;
	int sx, sy;
	bool is_root;
	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener output_sample;
	struct wl_listener frame_done;
	struct wl_list link;
};

struct switcher_state {
	Monitor *mon;
	struct wlr_scene_tree *tree;
	struct wlr_scene_rect *bg;
	struct switcher_tile **tiles;
	int count;
	int index;
	int tile_h;
	int scope;
};
extern struct switcher_state sw; // TODO refractor-header: move this global variable

static bool switcher_is_active(void) { return sw.tree != NULL; }

static bool switcher_candidate(Client *c) {
	if (!c->mon || c->iskilling || c->isminimized || c->isunglobal ||
		c->is_logic_hide || !client_surface(c) || !client_surface(c)->mapped ||
		client_is_unmanaged(c) || client_is_x11_popup(c))
		return false;

	if (sw.scope == SW_CURRENT_TAG)
		return c->mon == sw.mon && VISIBLEON(c, sw.mon);
	if (sw.scope == SW_ALL_TAG)
		return c->mon == sw.mon;
	return (int32_t)c->tags > 0;
}

static void switcher_content_size(Client *c, float *w, float *h) {
#ifdef XWAYLAND
	if (client_is_x11(c)) {
		struct wlr_surface *s = client_surface(c);
		*w = s->current.width;
		*h = s->current.height;
		return;
	}
#endif
	*w = c->surface.xdg->geometry.width;
	*h = c->surface.xdg->geometry.height;
}

/* Function Definitions */
bool switcher_is_active(void);
bool switcher_candidate(Client *c);
void switcher_content_size(Client *c, float *w, float *h);
// map the surface tree into the fixed tile box, re-run after each commit
// because the content size or clip may change
void switcher_tile_layout(struct switcher_tile *tile);
void switcher_surface_finish(struct switcher_surface *entry);
void switcher_surface_update_buffer(struct switcher_surface *entry);
void switcher_surface_commit(struct wl_listener *listener, void *data);
void switcher_surface_destroy(struct wl_listener *listener, void *data);
void switcher_surface_output_sample(struct wl_listener *listener, void *data);
// hidden clients are paced by the preview buffer
void switcher_surface_frame_done(struct wl_listener *listener, void *data);
void switcher_tile_add_surface(struct wlr_surface *surface, int sx, int sy,
							   void *data);
void switcher_tile_create(struct switcher_tile *tile, Client *c);
void switcher_layout(void);
void switcher_apply_highlight(void);
void switcher_close(void);
void switcher_remove_client(Client *c);
void switcher_commit_client(Client *tc);
void switcher_commit(void);
Client *switcher_client_at(double lx, double ly);
void switcher_open(int scope);
void switcher_cycle(int dir);
void switcher(const Arg *arg);

#endif
static void switcher_tile_add_surface(struct wlr_surface *surface, int sx,
									  int sy, void *data) {
	struct switcher_tile *tile = data;

	// only the real scene surface may update wl_surface output membership
	struct wlr_scene_buffer *buffer =
		wlr_scene_buffer_create(tile->content, NULL);
	if (!buffer)
		return;

	struct switcher_surface *entry = ecalloc(1, sizeof(*entry));
	entry->tile = tile;
	entry->surface = surface;
	entry->buffer = buffer;
	entry->sx = sx;
	entry->sy = sy;
	entry->is_root = surface == client_surface(tile->c);
	wlr_scene_buffer_set_filter_mode(entry->buffer, WLR_SCALE_FILTER_BILINEAR);
	switcher_surface_update_buffer(entry);

	LISTEN(&surface->events.commit, &entry->commit, switcher_surface_commit);
	LISTEN(&surface->events.destroy, &entry->destroy, switcher_surface_destroy);
	LISTEN(&buffer->events.output_sample, &entry->output_sample,
		   switcher_surface_output_sample);
	LISTEN(&buffer->events.frame_done, &entry->frame_done,
		   switcher_surface_frame_done);
	wl_list_insert(&tile->surfaces, &entry->link);
}

static void switcher_tile_create(struct switcher_tile *tile, Client *c) {
	tile->c = c;
	wl_list_init(&tile->surfaces);
	float src_w, src_h;
	switcher_content_size(c, &src_w, &src_h);
	float aspect = src_w > 0 && src_h > 0 ? src_w / src_h : 1.0f;
	aspect = MANGO_MAX(SW_ASPECT_MIN, MANGO_MIN(aspect, SW_ASPECT_MAX));
	tile->cw = MANGO_MAX(1, (int)(sw.tile_h * aspect));

	tile->tree = wlr_scene_tree_create(sw.tree);
	tile->frame = wlr_scene_rect_create(tile->tree, 1, 1, config.bordercolor);
	tile->content = wlr_scene_tree_create(tile->tree);
	wlr_scene_node_set_position(&tile->content->node, SW_PAD, SW_PAD);
	wlr_surface_for_each_surface(client_surface(c), switcher_tile_add_surface,
								 tile);
	switcher_tile_layout(tile);
}

static void switcher_layout(void) {
	int frame_h = sw.tile_h + 2 * SW_PAD;
	int maxrow_w =
		MANGO_MAX(1, (int)(sw.mon->m.width * SW_PANEL_FRAC) - 2 * SW_MARGIN);

	int nrows = 1;
	int row_w = 0;
	int content_w = 0;
	for (int i = 0; i < sw.count; i++) {
		struct switcher_tile *tile = sw.tiles[i];
		int fw = tile->cw + 2 * SW_PAD;
		if (row_w > 0 && row_w + SW_GAP + fw > maxrow_w) {
			nrows++;
			row_w = fw;
		} else {
			row_w = row_w == 0 ? fw : row_w + SW_GAP + fw;
		}
		tile->row = nrows - 1;
		content_w = MANGO_MAX(content_w, row_w);
	}

	int *rowsw = ecalloc(nrows, sizeof(*rowsw));
	for (int i = 0; i < sw.count; i++) {
		struct switcher_tile *tile = sw.tiles[i];
		int fw = tile->cw + 2 * SW_PAD;
		rowsw[tile->row] += rowsw[tile->row] == 0 ? fw : SW_GAP + fw;
	}

	int panel_w = content_w + 2 * SW_MARGIN;
	int panel_h = 2 * SW_MARGIN + nrows * frame_h + (nrows - 1) * SW_GAP;
	wlr_scene_node_set_position(&sw.tree->node,
								sw.mon->m.x + (sw.mon->m.width - panel_w) / 2,
								sw.mon->m.y + (sw.mon->m.height - panel_h) / 2);
	wlr_scene_rect_set_size(sw.bg, panel_w, panel_h);

	int row = -1;
	int x = 0;
	for (int i = 0; i < sw.count; i++) {
		struct switcher_tile *tile = sw.tiles[i];
		if (tile->row != row) {
			row = tile->row;
			x = SW_MARGIN + (content_w - rowsw[row]) / 2;
		}
		wlr_scene_node_set_position(&tile->tree->node, x,
									SW_MARGIN + row * (frame_h + SW_GAP));
		wlr_scene_rect_set_size(tile->frame, tile->cw + 2 * SW_PAD, frame_h);
		x += tile->cw + 2 * SW_PAD + SW_GAP;
	}
	free(rowsw);
}

static void switcher_apply_highlight(void) {
	for (int i = 0; i < sw.count; i++)
		wlr_scene_rect_set_color(sw.tiles[i]->frame, i == sw.index
														 ? config.focuscolor
														 : config.bordercolor);
}

static void switcher_close(void) {
	if (!switcher_is_active())
		return;
	for (int i = 0; i < sw.count; i++) {
		struct switcher_surface *entry, *tmp;
		wl_list_for_each_safe(entry, tmp, &sw.tiles[i]->surfaces, link) {
			switcher_surface_finish(entry);
		}
	}
	wlr_scene_node_destroy(&sw.tree->node);
	for (int i = 0; i < sw.count; i++)
		free(sw.tiles[i]);
	free(sw.tiles);
	memset(&sw, 0, sizeof(sw));
}

// remove a single tile while the switcher stays open, then relayout
static void switcher_remove_client(Client *c) {
	if (!switcher_is_active())
		return;
	int i;
	for (i = 0; i < sw.count; i++) {
		if (sw.tiles[i]->c == c)
			break;
	}
	if (i == sw.count)
		return;

	struct switcher_surface *entry, *tmp;
	wl_list_for_each_safe(entry, tmp, &sw.tiles[i]->surfaces, link) {
		switcher_surface_finish(entry);
	}
	wlr_scene_node_destroy(&sw.tiles[i]->tree->node);
	free(sw.tiles[i]);

	if (sw.index > i)
		sw.index--;
	memmove(&sw.tiles[i], &sw.tiles[i + 1],
			(sw.count - i - 1) * sizeof(*sw.tiles));
	sw.count--;
	if (sw.count == 0) {
		switcher_close();
		return;
	}
	if (sw.index >= sw.count)
		sw.index = sw.count - 1;
	switcher_layout();
	switcher_apply_highlight();
}

static void switcher_commit_client(Client *tc) {
	switcher_close();
	if (!tc || !tc->mon || tc->iskilling || tc->isminimized || tc->isunglobal ||
		tc->is_logic_hide || !client_surface(tc) ||
		!client_surface(tc)->mapped || client_is_unmanaged(tc) ||
		client_is_x11_popup(tc))
		return;
	if (!VISIBLEON(tc, tc->mon))
		view_in_mon(&(Arg){.ui = get_tags_first_tag(tc->tags)}, true, tc->mon,
					true);
	focusclient(tc, 1);
}

static void switcher_commit(void) {
	if (!switcher_is_active())
		return;
	switcher_commit_client(sw.tiles[sw.index]->c);
}

static Client *switcher_client_at(double lx, double ly) {
	if (!switcher_is_active())
		return NULL;
	for (int i = 0; i < sw.count; i++) {
		struct switcher_tile *tile = sw.tiles[i];
		struct wlr_box box = {
			.width = tile->cw + 2 * SW_PAD,
			.height = sw.tile_h + 2 * SW_PAD,
		};
		if (!wlr_scene_node_coords(&tile->tree->node, &box.x, &box.y) ||
			!wlr_box_contains_point(&box, lx, ly))
			continue;
		return tile->c;
	}
	return NULL;
}

static void switcher_open(int scope) {
	Client *c;
	Monitor *m;
	int n = 0;

	wl_list_for_each(m, &mons, link) {
		if (m->isoverview)
			return;
	}

	sw.mon = selmon;
	sw.scope = scope;

	wl_list_for_each(c, &fstack, flink) {
		if (switcher_candidate(c))
			n++;
	}
	if (n == 0)
		return;

	int max_row_w = (int)(sw.mon->m.width * SW_PANEL_FRAC) - 2 * SW_MARGIN;
	int max_panel_h = (int)(sw.mon->m.height * SW_PANEL_FRAC);
	// size against the widest allowed tile so the panel always fits
	sw.tile_h = 1;
	for (int cols = 1; cols <= n; cols++) {
		int rows = (n + cols - 1) / cols;
		int h_by_w =
			(int)((max_row_w - (cols - 1) * SW_GAP - 2 * cols * SW_PAD) /
				  (cols * SW_ASPECT_MAX));
		int h_by_h =
			(max_panel_h - 2 * SW_MARGIN - (rows - 1) * SW_GAP) / rows -
			2 * SW_PAD;
		int h = MANGO_MIN((int)(sw.mon->m.height * SW_TILE_FRAC),
						  MANGO_MIN(h_by_w, h_by_h));
		sw.tile_h = MANGO_MAX(sw.tile_h, h);
	}

	sw.tree = wlr_scene_tree_create(layers[LyrOverlay]);
	sw.bg = wlr_scene_rect_create(sw.tree, 1, 1, switcher_panel_color);
	sw.tiles = ecalloc(n, sizeof(*sw.tiles));
	wl_list_for_each(c, &fstack, flink) {
		if (!switcher_candidate(c))
			continue;
		struct switcher_tile *tile = ecalloc(1, sizeof(*tile));
		switcher_tile_create(tile, c);
		sw.tiles[sw.count++] = tile;
	}
	// 当前窗口位于 fstack 头部（tiles[0]），选中第二个并提交后会被插到
	// 最前，下次打开时原来的第一个变成第二个，从而在最近两个窗口间往返
	sw.index = sw.count > 1 ? 1 : 0;
	switcher_layout();
	switcher_apply_highlight();

	// restart render loops of clients idling without frame callbacks so
	// tiles of hidden windows stay live, like overview cards
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	for (int i = 0; i < sw.count; i++)
		client_send_frame_done(sw.tiles[i]->c, &now);
}

static void switcher_cycle(int dir) {
	sw.index = (sw.index + dir + sw.count) % sw.count;
	switcher_apply_highlight();
}

void switcher(const Arg *arg) {
	int dir = arg && arg->i == PREV ? -1 : 1;
	int scope = arg ? arg->i2 : SW_CURRENT_TAG;
	if (locked || !selmon || selmon->is_jump_mode)
		return;
	if (switcher_is_active()) {
		if (scope != sw.scope) {
			switcher_close();
			switcher_open(scope);
		} else {
			switcher_cycle(dir);
		}
	} else {
		switcher_open(scope);
	}
}
