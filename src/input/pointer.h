#ifndef __INPUT_POINTER_H__
#define __INPUT_POINTER_H__

#include "../mango.h"
#include <stdint.h>

bool pointer_is_trackpad(struct wlr_pointer *pointer) {
	struct libinput_device *device;

	if (wlr_input_device_is_libinput(&pointer->base) &&
		(device = wlr_libinput_get_device_handle(&pointer->base))) {
		if (libinput_device_config_tap_get_finger_count(device) > 0) {
			return true;
		}
	}

	return false;
}

void toggle_hotarea(int32_t x_root, int32_t y_root);
bool pointer_is_trackpad(struct wlr_pointer *pointer);
void // 鼠标滚轮事件
axisnotify(struct wl_listener *listener, void *data);
int32_t ongesture(struct wlr_pointer_swipe_end_event *event);
void swipe_begin(struct wl_listener *listener, void *data);
void swipe_update(struct wl_listener *listener, void *data);
void swipe_end(struct wl_listener *listener, void *data);
void pinch_begin(struct wl_listener *listener, void *data);
void pinch_update(struct wl_listener *listener, void *data);
void pinch_end(struct wl_listener *listener, void *data);
void hold_begin(struct wl_listener *listener, void *data);
void hold_end(struct wl_listener *listener, void *data);
Client *find_closest_tiled_client(Client *c);
void place_drag_tile_client(Client *c);
bool check_trackpad_disabled(struct wlr_pointer *pointer);
void // 鼠标按键事件
buttonpress(struct wl_listener *listener, void *data);
bool handle_buttonpress(struct wlr_pointer_button_event *event);
void last_cursor_surface_destroy(struct wl_listener *listener, void *data);
void setcursorshape(struct wl_listener *listener, void *data);


int32_t ongesture(struct wlr_pointer_swipe_end_event *event) {
	uint32_t mods;
	const GestureBinding *g;
	uint32_t motion;
	uint32_t adx = (int32_t)round(fabs(swipe_dx));
	uint32_t ady = (int32_t)round(fabs(swipe_dy));
	int32_t handled = 0;
	int32_t ji;

	if (event->cancelled) {
		return handled;
	}

	// Require absolute distance movement beyond a small thresh-hold
	if (adx * adx + ady * ady <
		config.swipe_min_threshold * config.swipe_min_threshold) {
		return handled;
	}

	if (adx > ady) {
		motion = swipe_dx < 0 ? SWIPE_LEFT : SWIPE_RIGHT;
	} else {
		motion = swipe_dy < 0 ? SWIPE_UP : SWIPE_DOWN;
	}

	mods = keyboard_hard_modifiers();

	for (ji = 0; ji < config.gesture_bindings_count; ji++) {
		g = &config.gesture_bindings[ji];
		if ((g->iscommonmode || (g->isdefaultmode && keymode.isdefault) ||
			 (strcmp(keymode.mode, g->mode) == 0)) &&
			CLEANMASK(mods) == CLEANMASK(g->mod) &&
			swipe_fingers == g->fingers_count && motion == g->motion &&
			g->func) {
			g->func(&g->arg);
			handled = 1;
		}
	}
	return handled;
}

void swipe_begin(struct wl_listener *listener, void *data) {
	struct wlr_pointer_swipe_begin_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward swipe begin event to client
	wlr_pointer_gestures_v1_send_swipe_begin(pointer_gestures, seat,
											 event->time_msec, event->fingers);
}

void swipe_update(struct wl_listener *listener, void *data) {
	struct wlr_pointer_swipe_update_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	swipe_fingers = event->fingers;
	// Accumulate swipe distance
	swipe_dx += event->dx;
	swipe_dy += event->dy;

	// Forward swipe update event to client
	wlr_pointer_gestures_v1_send_swipe_update(
		pointer_gestures, seat, event->time_msec, event->dx, event->dy);
}

void swipe_end(struct wl_listener *listener, void *data) {
	struct wlr_pointer_swipe_end_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	ongesture(event);
	swipe_dx = 0;
	swipe_dy = 0;
	// Forward swipe end event to client
	wlr_pointer_gestures_v1_send_swipe_end(pointer_gestures, seat,
										   event->time_msec, event->cancelled);
}

void pinch_begin(struct wl_listener *listener, void *data) {
	struct wlr_pointer_pinch_begin_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward pinch begin event to client
	wlr_pointer_gestures_v1_send_pinch_begin(pointer_gestures, seat,
											 event->time_msec, event->fingers);
}

void pinch_update(struct wl_listener *listener, void *data) {
	struct wlr_pointer_pinch_update_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward pinch update event to client
	wlr_pointer_gestures_v1_send_pinch_update(
		pointer_gestures, seat, event->time_msec, event->dx, event->dy,
		event->scale, event->rotation);
}

void pinch_end(struct wl_listener *listener, void *data) {
	struct wlr_pointer_pinch_end_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward pinch end event to client
	wlr_pointer_gestures_v1_send_pinch_end(pointer_gestures, seat,
										   event->time_msec, event->cancelled);
}

void hold_begin(struct wl_listener *listener, void *data) {
	struct wlr_pointer_hold_begin_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward hold begin event to client
	wlr_pointer_gestures_v1_send_hold_begin(pointer_gestures, seat,
											event->time_msec, event->fingers);
}

void hold_end(struct wl_listener *listener, void *data) {
	struct wlr_pointer_hold_end_event *event = data;

	if (config.disable_trackpad) {
		return;
	}

	// Forward hold end event to client
	wlr_pointer_gestures_v1_send_hold_end(pointer_gestures, seat,
										  event->time_msec, event->cancelled);
}

Client *find_closest_tiled_client(Client *c) {
	Client *tc, *closest = NULL;
	long min_dist = LONG_MAX;
	Monitor *cursor_mon = xytomon(cursor->x, cursor->y);

	wl_list_for_each(tc, &clients, link) {
		if (tc == c || !ISTILED(tc) || !VISIBLEON(tc, cursor_mon))
			continue;

		if (cursor->x >= tc->geom.x &&
			cursor->x < tc->geom.x + tc->geom.width &&
			cursor->y >= tc->geom.y &&
			cursor->y < tc->geom.y + tc->geom.height) {
			return tc;
		}

		int32_t dx = tc->geom.x + (int32_t)(tc->geom.width / 2) - cursor->x;
		int32_t dy = tc->geom.y + (int32_t)(tc->geom.height / 2) - cursor->y;
		long dist = (long)dx * dx + (long)dy * dy;

		if (dist < min_dist) {
			min_dist = dist;
			closest = tc;
		}
	}

	return closest;
}

void place_drag_tile_client(Client *c) {
	Client *closest = find_closest_tiled_client(c);

	if (closest && closest->mon) {
		const Layout *layout =
			closest->mon->pertag->ltidxs[get_client_tag_idx(closest)];

		if (closest->drop_direction == UNDIR) {
			setfloating(c, 0);
			wl_list_safe_reinsert_prev(&closest->link, &c->link);
			arrange(closest->mon, false, false);
			return;
		}

		if (layout->id == SCROLLER) {
			scroller_drop_tile(c, closest, 0);
			return;
		}
		if (layout->id == VERTICAL_SCROLLER) {
			scroller_drop_tile(c, closest, 1);
			return;
		}
		if (layout->id == DWINDLE) {
			uint32_t tag = get_client_tag_idx(c);
			bool insert_before = (closest->drop_direction == LEFT ||
								  closest->drop_direction == UP);
			bool split_h = (closest->drop_direction == LEFT ||
							closest->drop_direction == RIGHT);
			dwindle_insert(&c->mon->pertag->dwindle_root[tag], c, closest,
						   config.dwindle_split_ratio, insert_before, split_h,
						   !config.dwindle_drop_simple_split);
			setfloating(c, 0);
			return;
		}

		if (layout->id == RIGHT_TILE) {
			if (closest->drop_direction == LEFT) {
				wl_list_safe_reinsert_next(&closest->link, &c->link);
			} else if (closest->drop_direction == RIGHT) {
				wl_list_safe_reinsert_prev(&closest->link, &c->link);
			} else if (closest->drop_direction == UP) {
				wl_list_safe_reinsert_prev(&closest->link, &c->link);
			} else {
				wl_list_safe_reinsert_next(&closest->link, &c->link);
			}
			setfloating(c, 0);
			return;
		}

		if (closest->drop_direction == LEFT || closest->drop_direction == UP) {
			wl_list_safe_reinsert_prev(&closest->link, &c->link);
		} else {
			wl_list_safe_reinsert_next(&closest->link, &c->link);
		}
	}

	setfloating(c, 0);
}

bool check_trackpad_disabled(struct wlr_pointer *pointer) {
	if (!config.disable_trackpad) {
		return false;
	}

	return pointer_is_trackpad(pointer);
}

void // 鼠标按键事件
buttonpress(struct wl_listener *listener, void *data) {
	struct wlr_pointer_button_event *event = data;

	ipc_notify_device_event(&event->pointer->base);

	if (!handle_buttonpress(event))
		wlr_seat_pointer_notify_button(seat, event->time_msec, event->button,
									   event->state);
}

bool handle_buttonpress(struct wlr_pointer_button_event *event) {
	uint32_t mods;
	Client *c = NULL;
	LayerSurface *l = NULL;
	MangoGroupBar *gb = NULL;
	struct wlr_surface *surface;
	Client *tmpc = NULL;
	int32_t ji;
	const MouseBinding *m;
	struct wlr_surface *old_pointer_focus_surface =
		seat->pointer_state.focused_surface;

	handlecursoractivity();
	wlr_idle_notifier_v1_notify_activity(idle_notifier, seat);

	if (event->pointer && check_trackpad_disabled(event->pointer)) {
		return true;
	}

	switch (event->state) {
	case WL_POINTER_BUTTON_STATE_PRESSED:
		cursor_mode = CurPressed;
		selmon = xytomon(cursor->x, cursor->y);
		if (locked)
			break;

		if (switcher_is_active() &&
			(event->button == BTN_LEFT || event->button == BTN_RIGHT)) {
			Client *switcher_c = switcher_client_at(cursor->x, cursor->y);
			if (!switcher_c)
				switcher_close();
			else if (event->button == BTN_LEFT)
				switcher_commit_client(switcher_c);
			else
				pending_kill_client(switcher_c);
			wlr_seat_pointer_notify_clear_focus(seat);
			return true;
		}

		xytonode(cursor->x, cursor->y, &surface, NULL, NULL, &gb, NULL, NULL);
		if (toplevel_from_wlr_surface(surface, &c, &l) >= 0) {
			if (c && c->scene && c->scene->node.enabled &&
				VISIBLEON(c, c->mon) &&
				(!client_is_unmanaged(c) || client_wants_focus(c)))
				focusclient(c, 1);

			if (surface != old_pointer_focus_surface) {
				wlr_seat_pointer_notify_clear_focus(seat);
				motionnotify(0, NULL, 0, 0, 0, 0);
			}

			// 聚焦按需要交互焦点的layer，但注意不能抢占独占焦点的layer
			if (l && !exclusive_focus &&
				l->layer_surface->current.keyboard_interactive ==
					ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND) {
				focuslayer(l);
			}
		}

		// overview模式下鼠标左键跳转，右键关闭窗口
		if (selmon && selmon->isoverview && event->button == BTN_LEFT && c) {
			toggleoverview(&(Arg){.tc = c});
			return true;
		}

		if (selmon && selmon->isoverview && event->button == BTN_RIGHT && c) {
			pending_kill_client(c);
			return true;
		}

		// handle click on tile node
		client_handle_decorate_click(gb);

		mods = keyboard_hard_modifiers();

		for (ji = 0; ji < config.mouse_bindings_count; ji++) {
			m = &config.mouse_bindings[ji];

			if ((m->iscommonmode || (m->isdefaultmode && keymode.isdefault) ||
				 (strcmp(keymode.mode, m->mode) == 0)) &&
				CLEANMASK(mods) == CLEANMASK(m->mod) &&
				event->button == m->button && m->func &&
				(CLEANMASK(m->mod) != 0 ||
				 (event->button != BTN_LEFT && event->button != BTN_RIGHT))) {
				m->func(&m->arg);
				return true;
			}
		}
		break;
	case WL_POINTER_BUTTON_STATE_RELEASED:
		/* If you released any buttons, we exit interactive move/resize mode. */
		if (!locked && cursor_mode != CurNormal && cursor_mode != CurPressed) {
			cursor_mode = CurNormal;
			/* Clear the pointer focus, this way if the cursor is over a surface
			 * we will send an enter event after which the client will provide
			 * us a cursor surface */
			wlr_seat_pointer_clear_focus(seat);
			motionnotify(0, NULL, 0, 0, 0, 0);
			/* Drop the window off on its new monitor */
			if (grabc == selmon->sel) {
				selmon->sel = NULL;
			}
			selmon = xytomon(cursor->x, cursor->y);
			client_update_oldmonname_record(grabc, selmon);
			setmon(grabc, selmon, 0, true);
			/* if the view changed mid-drag, drop onto the current tag
			 * instead of silently returning to the original one */
			if (!VISIBLEON(grabc, selmon))
				grabc->tags = selmon->tagset[selmon->seltags];
			selmon->prevsel = ISTILED(selmon->sel) ? selmon->sel : NULL;
			selmon->sel = grabc;
			tmpc = grabc;
			grabc = NULL;
			start_drag_window = false;
			last_apply_drap_time = 0;
			if (tmpc->drag_to_tile && config.drag_tile_to_tile) {
				place_drag_tile_client(tmpc);
				tmpc->float_geom = tmpc->drag_tile_float_backup_geom;
			} else {
				apply_window_snap(tmpc);
			}
			tmpc->drag_to_tile = false;
			if (dropc) {
				dropc->enable_drop_area_draw = false;
				client_set_drop_area(dropc);
				dropc = NULL;
			}
			return true;
		} else {
			cursor_mode = CurNormal;
		}
		break;
	}
	/* If the event wasn't handled by the compositor, return false */
	return false;
}

void last_cursor_surface_destroy(struct wl_listener *listener, void *data) {
	last_cursor.surface = NULL;
	wl_list_remove(&listener->link);
}

void setcursorshape(struct wl_listener *listener, void *data) {
	struct wlr_cursor_shape_manager_v1_request_set_shape_event *event = data;
	if (cursor_mode != CurNormal && cursor_mode != CurPressed)
		return;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. If so, we can tell the cursor to
	 * use the provided cursor shape. */
	if (event->seat_client == seat->pointer_state.focused_client) {
		/* Remove surface destroy listener if active */
		if (last_cursor.surface &&
			last_cursor_surface_destroy_listener.link.prev != NULL)
			wl_list_remove(&last_cursor_surface_destroy_listener.link);

		last_cursor.shape = event->shape;
		last_cursor.surface = NULL;
		if (!cursor_hidden)
			wlr_cursor_set_xcursor(cursor, cursor_mgr,
								   wlr_cursor_shape_v1_name(event->shape));
	}
}

void pointer_set_accel(struct libinput_device *device, bool natural_scrolling,
					   uint32_t mouse_accel_profile, double mouse_accel_speed);
void configure_pointer(struct wlr_input_device *wlr_device,
					   struct libinput_device *device);
void createpointer(struct wlr_pointer *pointer);
void createpointerconstraint(struct wl_listener *listener, void *data);
void cursorconstrain(struct wlr_pointer_constraint_v1 *constraint);
void cursorframe(struct wl_listener *listener, void *data);
void cursorwarptohint(void);
void destroydragicon(struct wl_listener *listener, void *data);
void destroypointerconstraint(struct wl_listener *listener, void *data);
void motionabsolute(struct wl_listener *listener, void *data);
void resize_floating_window(Client *grabc);
void motionnotify(uint32_t time, struct wlr_input_device *device, double dx,
				  double dy, double dx_unaccel, double dy_unaccel);
void motionrelative(struct wl_listener *listener, void *data);
void pointerfocus(Client *c, struct wlr_surface *surface, double sx, double sy,
				  uint32_t time);
void requeststartdrag(struct wl_listener *listener, void *data);
void setcursor(struct wl_listener *listener, void *data);
void startdrag(struct wl_listener *listener, void *data);
void handlecursoractivity(void);
int32_t hidecursor(void *data);
void warp_cursor(const Client *c);
void warp_cursor_to_selmon(Monitor *m);
void virtualpointer(struct wl_listener *listener, void *data);

#endif
