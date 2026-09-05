#ifndef ___ARRANGE_H__
#define ___ARRANGE_H__

#include "../mango.h"

<<<<<<< HEAD
void set_size_per(Monitor *m, Client *c);
=======
	uint32_t tag = get_mon_curtag(m);
	const Layout *current_layout = m->pertag->ltidxs[tag];

	wl_list_for_each(fc, &clients, link) {
		if (VISIBLEON(fc, m) && ISTILED(fc) && fc != c) {
			if (current_layout->id == CENTER_TILE &&
				(fc->isleftstack ^ c->isleftstack))
				continue;
			c->master_mfact_per = fc->master_mfact_per;
			c->master_inner_per = fc->master_inner_per;
			c->stack_inner_per = fc->stack_inner_per;
			found = true;
			break;
		}
	}

	if (!found || c->isfloating) {
		c->master_mfact_per = m->pertag->mfacts[tag];
		c->master_inner_per = 1.0f;
		c->stack_inner_per = 1.0f;
	}

	if (!c->iscustom_scroller_proportion) {
		c->scroller_proportion = m->pertag->scroller_default_proportion[tag];
	}

	if (!c->iscustom_scroller_proportion_single) {
		c->scroller_proportion_single =
			m->pertag->scroller_default_proportion_single[tag];
	}
}
>>>>>>> c62e01c303b8e19598df0b0b7cd16e466a71fc21

void resize_tile_master_horizontal(Client *grabc, bool isdrag, int32_t offsetx,
								   int32_t offsety, uint32_t time,
								   int32_t type);

void resize_tile_master_vertical(Client *grabc, bool isdrag, int32_t offsetx,
								 int32_t offsety, uint32_t time, int32_t type);

void resize_tile_dwindle(Client *grabc, bool isdrag, int32_t offsetx,
						 int32_t offsety, uint32_t time, bool isvertical);

void resize_tile_grid_fair(Client *grabc, bool isdrag, int32_t offsetx,
						   int32_t offsety, uint32_t time);

void resize_tile_scroller(Client *grabc, bool isdrag, int32_t offsetx,
						  int32_t offsety, uint32_t time, bool isvertical);

void resize_tile_client(Client *grabc, bool isdrag, int32_t offsetx,
						int32_t offsety, uint32_t time);

void check_size_per_valid(Client *c);
/* If there are no calculation omissions,
these two functions will never be triggered.
Just in case to facilitate the final investigation*/

void check_size_per_valid(Client *c) {
	if (c->ismaster) {
		assert(c->master_inner_per > 0.0f && c->master_inner_per <= 1.0f);
	} else {
		assert(c->stack_inner_per > 0.0f && c->stack_inner_per <= 1.0f);
	}
}

void reset_size_per_mon(Monitor *m, int32_t tile_cilent_num,
						double total_left_stack_hight_percent,
						double total_right_stack_hight_percent,
						double total_stack_hight_percent,
						double total_master_inner_percent, int32_t master_num,
						int32_t stack_num);

// normal-tag client kept visible as background under the special overlay
static bool special_keep_bg_client(Monitor *m, Client *c) {
	return is_special_active(m) && !c->is_logic_hide && !c->isminimized &&
		   ((m->pertag->prevtag > 0 &&
			 (c->tags & (1 << (m->pertag->prevtag - 1)))) ||
			(c->tags & (m->tagset[m->seltags ^ 1] & ~TAG0_MASK)));
}

void pre_calculate_before_arrange(Monitor *m, bool want_animation,
								  bool from_view, bool only_calculate);

// remap tags through map; unmapped tags stay as-is.
static uint32_t tag_remap_mask(uint32_t tags, const uint32_t *map) {
	uint32_t out = tags & ~tagmask;
	uint32_t i;

	for (i = 1; i <= (uint32_t)config.tag_num; i++) {
		if (tags & (1u << (i - 1)))
			out |= map[i] ? (1u << (map[i] - 1)) : (1u << (i - 1));
	}
	return out;
}

// reset a pertag slot to its tagrule state.
static void tag_gather_reset_slot(Monitor *m, uint32_t tag) {
	int32_t i;

	tag_slot_set_defaults(m, tag);
	m->pertag->no_hide[tag] = 0;
	m->pertag->no_render_border[tag] = 0;
	m->pertag->open_as_floating[tag] = 0;
	m->pertag->dwindle_root[tag] = NULL;
	m->pertag->scroller_state[tag] = NULL;

	for (i = 0; i < config.tag_rules_count; i++) {
		const ConfigTagRule *tr = &config.tag_rules[i];

		if (tag_rule_matches_monitor(tr, m) &&
			(tr->id_wildcard || tr->id == (int32_t)tag))
			tag_rule_apply_to_slot(m, tr, tag);
	}
}

// move pertag state from src to dst, then reset src.
static void tag_gather_move_pertag(Monitor *m, uint32_t dst, uint32_t src) {
	m->pertag->nmasters[dst] = m->pertag->nmasters[src];
	m->pertag->mfacts[dst] = m->pertag->mfacts[src];
	m->pertag->no_hide[dst] = m->pertag->no_hide[src];
	m->pertag->no_render_border[dst] = m->pertag->no_render_border[src];
	m->pertag->open_as_floating[dst] = m->pertag->open_as_floating[src];
	m->pertag->scroller_default_proportion[dst] =
		m->pertag->scroller_default_proportion[src];
	m->pertag->scroller_default_proportion_single[dst] =
		m->pertag->scroller_default_proportion_single[src];
	m->pertag->scroller_ignore_proportion_single[dst] =
		m->pertag->scroller_ignore_proportion_single[src];
	m->pertag->dwindle_root[dst] = m->pertag->dwindle_root[src];
	m->pertag->ltidxs[dst] = m->pertag->ltidxs[src];
	m->pertag->scroller_state[dst] = m->pertag->scroller_state[src];
	tag_gather_reset_slot(m, src);
}

// Compact occupied tags on this monitor to 1..k (e.g. 1,3,9 -> 1,2,3).
void tag_gather_apply(Monitor *m);

uint32_t tag_remap_mask(uint32_t tags, const uint32_t *map);
void tag_gather_move_pertag(Monitor *m, uint32_t dst, uint32_t src);

void arrange(Monitor *m, bool want_animation, bool from_view);
#endif
	
// empty special view: keep it when entered via a view switch, exit otherwise.
// returns true when arrange should stop because the view was closed
static bool special_handle_empty_view(Monitor *m, bool from_view) {
	if (!is_special_active(m)) {
		m->special_empty_view = false;
		return false;
	}
	if (special_has_clients(m)) {
		m->special_empty_view = false;
		return false;
	}
	if (from_view) {
		m->special_empty_view = true;
		return false;
	}
	if (!m->special_empty_view) {
		toggle_special_tag_mon(m);
		return true;
	}
	return false;
}

void // 17
arrange(Monitor *m, bool want_animation, bool from_view) {

	if (!m || m->iscleanuping)
		return;

	if (!m->wlr_output->enabled)
		return;

	if (!m->sel) {
		m->sel = focustop(m);
	}

	if (special_handle_empty_view(m, from_view))
		return;

	pre_calculate_before_arrange(m, want_animation, from_view, false);

	bool is_tag0 = is_special_active(m);
	int32_t saved_oh = m->gappoh, saved_ov = m->gappov;
	int32_t saved_ih = m->gappih, saved_iv = m->gappiv;

	if (is_tag0) {
		m->gappoh = m->special_gappoh;
		m->gappov = m->special_gappov;
		m->gappih = m->special_gappih;
		m->gappiv = m->special_gappiv;
	}

	if (m->isoverview) {
		overviewlayout.arrange(m);
	} else {
		m->pertag->ltidxs[get_mon_curtag(m)]->arrange(m);
	}

	if (is_tag0) {
		m->gappoh = saved_oh;
		m->gappov = saved_ov;
		m->gappih = saved_ih;
		m->gappiv = saved_iv;
	}

	// gather after layout/animation setup so tag-switch animations still play.
	if (config.tag_gather) {
		tag_gather_apply(m);
	}

	if (!start_drag_window) {
		motionnotify(0, NULL, 0, 0, 0, 0);
		checkidleinhibitor(NULL);
	}

	printstatus(IPC_WATCH_ARRANGGE);
}
>>>>>>> c62e01c303b8e19598df0b0b7cd16e466a71fc21
