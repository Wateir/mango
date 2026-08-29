/*
 * See LICENSE file for copyright and license details.
 */
#include "mango.h"
#include "wlr/util/box.h"


/* function declarations */
static void applybounds(
	Client *c,
	struct wlr_box *bbox); // 设置边界规则,能让一些窗口拥有比较适合的大小
static void applyrules(Client *c); // 窗口规则应用,应用config.h中定义的窗口规则
static void apply_window_snap(Client *c);
static void arrange(Monitor *m, bool want_animation,
					bool from_view); // 布局函数,让窗口俺平铺规则移动和重置大小
static void arrangelayer(Monitor *m, struct wl_list *list,
						 struct wlr_box *usable_area, int32_t exclusive);
static void arrangelayers(Monitor *m);
static void handle_print_status(struct wl_listener *listener, void *data);
static void axisnotify(struct wl_listener *listener,
					   void *data); // 滚轮事件处理
static void buttonpress(struct wl_listener *listener,
						void *data); // 鼠标按键事件处理
static bool handle_buttonpress(struct wlr_pointer_button_event *event);
static int32_t ongesture(struct wlr_pointer_swipe_end_event *event);
static void swipe_begin(struct wl_listener *listener, void *data);
static void swipe_update(struct wl_listener *listener, void *data);
static void swipe_end(struct wl_listener *listener, void *data);
static void pinch_begin(struct wl_listener *listener, void *data);
static void pinch_update(struct wl_listener *listener, void *data);
static void pinch_end(struct wl_listener *listener, void *data);
static void hold_begin(struct wl_listener *listener, void *data);
static void hold_end(struct wl_listener *listener, void *data);
static void checkidleinhibitor(struct wlr_surface *exclude);
static void cleanupmon(struct wl_listener *listener, void *data); // 退出清理
static void closemon(Monitor *m);
static void toggle_hotarea(int32_t x_root, int32_t y_root); // 触发热区
static void maplayersurfacenotify(struct wl_listener *listener, void *data);
static void commitlayersurfacenotify(struct wl_listener *listener, void *data);
static void commitnotify(struct wl_listener *listener, void *data);
static void createdecoration(struct wl_listener *listener, void *data);
static void createidleinhibitor(struct wl_listener *listener, void *data);
static void createkeyboard(struct wlr_keyboard *keyboard);
static void requestmonstate(struct wl_listener *listener, void *data);
static void createlayersurface(struct wl_listener *listener, void *data);
static void createlocksurface(struct wl_listener *listener, void *data);
static void createmon(struct wl_listener *listener, void *data);
static void createnotify(struct wl_listener *listener, void *data);
static void createpointer(struct wlr_pointer *pointer);
static void configure_pointer(struct wlr_input_device *wlr_device,
							  struct libinput_device *device);
static void destroyinputdevice(struct wl_listener *listener, void *data);
static void createswitch(struct wlr_switch *switch_device);
static void switch_toggle(struct wl_listener *listener, void *data);
static void createpointerconstraint(struct wl_listener *listener, void *data);
static void cursorconstrain(struct wlr_pointer_constraint_v1 *constraint);
static void commitpopup(struct wl_listener *listener, void *data);
static void createpopup(struct wl_listener *listener, void *data);
static void cursorframe(struct wl_listener *listener, void *data);
static void cursorwarptohint(void);
static void destroydecoration(struct wl_listener *listener, void *data);
static void destroydragicon(struct wl_listener *listener, void *data);
static void destroyidleinhibitor(struct wl_listener *listener, void *data);
static void destroylayernodenotify(struct wl_listener *listener, void *data);
static void destroylock(SessionLock *lock, int32_t unlocked);
static void destroylocksurface(struct wl_listener *listener, void *data);
static void destroynotify(struct wl_listener *listener, void *data);
static void destroypointerconstraint(struct wl_listener *listener, void *data);
static void destroysessionlock(struct wl_listener *listener, void *data);
static void destroykeyboardgroup(struct wl_listener *listener, void *data);
static Monitor *dirtomon(enum wlr_direction dir);
static void setcursorshape(struct wl_listener *listener, void *data);

static void focuslayer(LayerSurface *l);
static void focusclient(Client *c, int32_t lift);

static void setborder_color(Client *c);
static Client *focustop(Monitor *m);
static void fullscreennotify(struct wl_listener *listener, void *data);
static void gpureset(struct wl_listener *listener, void *data);

static int32_t keyrepeat(void *data);

static void inputdevice(struct wl_listener *listener, void *data);
static int32_t keybinding(uint32_t state, bool locked, uint32_t mods,
						  xkb_keysym_t sym, uint32_t keycode);
static void keypress(struct wl_listener *listener, void *data);
static void keypressmod(struct wl_listener *listener, void *data);
static bool keypressglobal(struct wlr_surface *last_surface,
						   struct wlr_keyboard *keyboard,
						   struct wlr_keyboard_key_event *event, uint32_t mods,
						   xkb_keysym_t keysym, uint32_t keycode);
static void locksession(struct wl_listener *listener, void *data);
static void mapnotify(struct wl_listener *listener, void *data);
static void maximizenotify(struct wl_listener *listener, void *data);
static void minimizenotify(struct wl_listener *listener, void *data);
static void motionabsolute(struct wl_listener *listener, void *data);
static void motionnotify(uint32_t time, struct wlr_input_device *device,
						 double sx, double sy, double sx_unaccel,
						 double sy_unaccel);
static void motionrelative(struct wl_listener *listener, void *data);

static void reset_foreign_tolevel(Client *c, Monitor *oldmon, Monitor *newmon);
static void add_foreign_topleve(Client *c);
static void exchange_two_client(Client *c1, Client *c2);
static void outputmgrapply(struct wl_listener *listener, void *data);
static void outputmgrapplyortest(struct wlr_output_configuration_v1 *config,
								 int32_t test);
static void outputmgrtest(struct wl_listener *listener, void *data);
static void pointerfocus(Client *c, struct wlr_surface *surface, double sx,
						 double sy, uint32_t time);
static void printstatus(enum ipc_watch_type type);
static void powermgrsetmode(struct wl_listener *listener, void *data);
static void rendermon(struct wl_listener *listener, void *data);
static void requestdecorationmode(struct wl_listener *listener, void *data);
static void requestdrmlease(struct wl_listener *listener, void *data);
static void requeststartdrag(struct wl_listener *listener, void *data);
static void resize(Client *c, struct wlr_box geo, int32_t interact);
static void setcursor(struct wl_listener *listener, void *data);
static void setfloating(Client *c, int32_t floating);
static void setfakefullscreen(Client *c, int32_t fakefullscreen);
static void setfullscreen(Client *c, int32_t fullscreen, bool rearrange);
static void setmaximizescreen(Client *c, int32_t maximizescreen,
							  bool rearrange);
static void reset_maximizescreen_size(Client *c);
static void setgaps(int32_t oh, int32_t ov, int32_t ih, int32_t iv);

static void setmon(Client *c, Monitor *m, uint32_t newtags, bool focus);
static void setpsel(struct wl_listener *listener, void *data);
static void setsel(struct wl_listener *listener, void *data);
static void startdrag(struct wl_listener *listener, void *data);

static void unlocksession(struct wl_listener *listener, void *data);
static void unmaplayersurfacenotify(struct wl_listener *listener, void *data);
static void unmapnotify(struct wl_listener *listener, void *data);
static void updatemons(struct wl_listener *listener, void *data);
static void updatetitle(struct wl_listener *listener, void *data);
static void urgent(struct wl_listener *listener, void *data);
static void view(const Arg *arg, bool want_animation);

static void
handle_keyboard_shortcuts_inhibit_new_inhibitor(struct wl_listener *listener,
												void *data);
static void virtualkeyboard(struct wl_listener *listener, void *data);
static void virtualpointer(struct wl_listener *listener, void *data);
static void warp_cursor(const Client *c);
static Monitor *xytomon(double x, double y);
static Monitor *get_monitor_nearest_to(int32_t x, int32_t y);
static void handle_iamge_copy_capture_new_session(struct wl_listener *listener,
												  void *data);
static void xytonode(double x, double y, struct wlr_surface **psurface,
					 Client **pc, LayerSurface **pl, MangoGroupBar **tb,
					 double *nx, double *ny);
static void clear_fullscreen_flag(Client *c);
static pid_t getparentprocess(pid_t p);
static int32_t isdescprocess(pid_t p, pid_t c);
static Client *termforwin(Client *w);
static void client_replace(Client *c, Client *w, bool is_group_change_member,
						   bool is_swallow);

static void warp_cursor_to_selmon(Monitor *m);
uint32_t want_restore_fullscreen(Client *target_client);
static void overview_restore(Client *c, const Arg *arg);
static void overview_backup(Client *c);
static void set_minimized(Client *c);

static void show_scratchpad(Client *c);
static void show_hide_client(Client *c);
static void tag_client(const Arg *arg, Client *target_client);

static struct wlr_box setclient_coordinate_center(Client *c, Monitor *m,
												  struct wlr_box geom,
												  int32_t offsetx,
												  int32_t offsety);
static uint32_t get_tags_first_tag(uint32_t tags);

static struct wlr_output_mode *
get_nearest_output_mode(struct wlr_output *output, int32_t width,
						int32_t height, float refresh);

static void client_commit(Client *c);
static void layer_commit(LayerSurface *l);
static void client_draw_border(Client *c, struct ivec2 offsets);
static void client_set_opacity(Client *c, double opacity);
static void init_baked_points(void);
static void scene_buffer_apply_opacity(struct wlr_scene_buffer *buffer,
									   int32_t sx, int32_t sy, void *data);

static Client *direction_select(const Arg *arg);
static void view_in_mon(const Arg *arg, bool want_animation, Monitor *m,
						bool changefocus);

static void buffer_set_effect(Client *c, BufferData buffer_data);
static void snap_scene_buffer_apply_effect(struct wlr_scene_buffer *buffer,
										   int32_t sx, int32_t sy, void *data);
static void client_send_frame_done(Client *c, const struct timespec *now);
static void overview_layout_card(Client *c);
static void overview_destroy_card(Client *c);
static void overview_card_set_corner_radii(Client *c,
										   struct fx_corner_radii corners);
static void client_set_pending_state(Client *c);
static void layer_set_pending_state(LayerSurface *l);
static void set_rect_size(struct wlr_scene_rect *rect, int32_t width,
						  int32_t height);
static Client *center_tiled_select(Monitor *m);
static void handlecursoractivity(void);
static int32_t hidecursor(void *data);
static bool check_hit_no_border(Client *c);
static void reset_keyboard_layout(void);
static void client_update_oldmonname_record(Client *c, Monitor *m);
static void pending_kill_client(Client *c);
static uint32_t get_tags_first_tag_num(uint32_t source_tags);
static void set_layer_open_animaiton(LayerSurface *l, struct wlr_box geo);
static void init_fadeout_layers(LayerSurface *l);
static void layer_actual_size(LayerSurface *l, int32_t *width, int32_t *height);
static void get_layer_target_geometry(LayerSurface *l,
									  struct wlr_box *target_box);
static void scene_buffer_apply_effect(struct wlr_scene_buffer *buffer,
									  int32_t sx, int32_t sy, void *data);
static double find_animation_curve_at(double t, int32_t type);

static void apply_opacity_to_rect_nodes(Client *c, struct wlr_scene_node *node,
										double animation_passed);
static struct fx_corner_radii set_client_corner_location(Client *c);
static double all_output_frame_duration_ms();
static struct wlr_scene_tree *
wlr_scene_tree_snapshot(struct wlr_scene_node *node,
						struct wlr_scene_tree *parent);
static bool is_scroller_layout(Monitor *m);
static bool is_monocle_layout(Monitor *m);
static bool is_centertile_layout(Monitor *m);
static void create_output(struct wlr_backend *backend, void *data);
static void get_layout_abbr(char *abbr, const char *full_name);
static void apply_named_scratchpad(Client *target_client);
static Client *get_client_by_id_or_title(const char *arg_id,
										 const char *arg_title);
static bool switch_scratchpad_client_state(Client *c);
static bool check_trackpad_disabled(struct wlr_pointer *pointer);
static uint32_t get_tag_status(uint32_t tag, Monitor *m);
static void view_insert_shift_tags(Monitor *m, uint32_t target);
static void enable_adaptive_sync(Monitor *m, struct wlr_output_state *state);
static Client *get_next_stack_client(Client *c, bool reverse);
static void set_float_malposition(Client *tc);
static void set_size_per(Monitor *m, Client *c);
static void resize_tile_client(Client *grabc, bool isdrag, int32_t offsetx,
							   int32_t offsety, uint32_t time);
static void refresh_monitors_workspaces_status(Monitor *m);
static void init_client_properties(Client *c);
static float *get_border_color(Client *c);
static void clear_fullscreen_and_maximized_state(Monitor *m);
static void request_fresh_all_monitors(void);
static Client *find_client_by_direction(Client *tc, const Arg *arg,
										bool findfloating);
static void exit_scroller_stack(Client *c);
static Client *scroll_get_stack_head_client(Client *c);
static bool client_only_in_one_tag(Client *c);
static Client *get_focused_stack_client(Client *sc,
										Client *custom_focus_client);
static bool client_is_in_same_stack(Client *sc, Client *tc, Client *fc);
static void monitor_stop_skip_frame_timer(Monitor *m);
static int monitor_skip_frame_timeout_callback(void *data);
static Monitor *get_monitor_nearest_to(int32_t lx, int32_t ly);
static bool match_monitor_spec(char *spec, Monitor *m);
static void last_cursor_surface_destroy(struct wl_listener *listener,
										void *data);
static int32_t keep_idle_inhibit(void *data);
static void check_keep_idle_inhibit(Client *c);
static void pre_calculate_before_arrange(Monitor *m, bool want_animation,
										 bool from_view, bool only_calculate);
static void client_pending_fullscreen_state(Client *c, int32_t isfullscreen);
static void client_pending_maximized_state(Client *c, int32_t ismaximized);
static void client_pending_minimized_state(Client *c, int32_t isminimized);
static void scroller_insert_stack(Client *c, Client *target_client,
								  bool insert_before);
static void dwindle_move_client(DwindleNode **root, Client *c, Client *target,
								float ratio, int32_t dir);
static void dwindle_resize_client_step(Monitor *m, Client *c, int32_t dx,
									   int32_t dy);
static void dwindle_resize_client(Monitor *m, Client *c);

static struct TagScrollerState *ensure_scroller_state(Monitor *m, uint32_t tag);
static struct ScrollerStackNode *find_scroller_node(struct TagScrollerState *st,
													Client *c);
static void sync_scroller_state_to_clients(Monitor *m, uint32_t tag);
static void scroller_node_remove(struct TagScrollerState *st,
								 struct ScrollerStackNode *target);
static struct ScrollerStackNode *
scroller_node_create(struct TagScrollerState *st, Client *c);
static void update_scroller_state(Monitor *m);
Client *scroll_get_stack_tail_client(Client *c);
static DwindleNode *dwindle_find_leaf(DwindleNode *node, Client *c);
static void overview_backup_surface(Client *c);

static void create_jump_hints(Monitor *m);
static void finish_jump_mode(Monitor *m);
static void begin_jump_mode(Monitor *m);
static void global_draw_group_bar(Client *c, int32_t x, int32_t y,
								  int32_t width, int32_t height);

static void client_reparent_group(Client *c);
static void client_change_mon(Client *c, Monitor *m);
static void check_vrr_enable(Client *c);
static void output_state_setup_hdr(Monitor *m, bool silent,
								   struct wlr_output_state *state);
static void output_enable_hdr(Monitor *m, struct wlr_output_state *os,
							  bool enabled, bool silent);
static bool mango_scene_output_commit(struct wlr_scene_output *scene_output,
									  struct wlr_output_state *state);
static bool mango_output_commit(Monitor *m);
static bool check_tearing_frame_allow(Monitor *m);
static void client_set_group_config(Client *c);
static const char *xdg_activation_v1_export_token(void);

static int32_t client_is_x11(Client *c);
static struct wlr_surface *client_surface(Client *c);
static int32_t toplevel_from_wlr_surface(struct wlr_surface *s, Client **pc,
										 LayerSurface **pl);
static void client_activate_surface(struct wlr_surface *s, int32_t activated);
static const char *client_get_appid(Client *c);
static int32_t client_get_pid(Client *c);
static void client_get_clip(Client *c, struct wlr_box *clip);
static void client_get_geometry(Client *c, struct wlr_box *geom);
static Client *client_get_parent(Client *c);
static int32_t client_has_children(Client *c);
static const char *client_get_title(Client *c);
static int32_t client_is_float_type(Client *c);
static int32_t client_is_rendered_on_mon(Client *c, Monitor *m);
static int32_t client_is_unmanaged(Client *c);
static void client_notify_enter(struct wlr_surface *s, struct wlr_keyboard *kb);
static void client_send_close(Client *c);
static void client_set_border_color(Client *c, const float color[static 4]);
static void client_set_fullscreen(Client *c, int32_t fullscreen);
static void client_set_scale(struct wlr_surface *s, float scale);
static void client_update_xwayland_clip(Client *c, struct wlr_box *clip);
static void client_update_xwayland_dest_size(Client *c);
static uint32_t client_set_size(Client *c, uint32_t width, uint32_t height);
static void client_set_minimized(Client *c, bool minimized);
static void client_set_maximized(Client *c, bool maximized);
static void client_set_tiled(Client *c, uint32_t edges);
static int32_t client_should_ignore_focus(Client *c);
static int32_t client_is_x11_popup(Client *c);
static int32_t client_should_global(Client *c);
static int32_t client_should_overtop(Client *c);
static int32_t client_wants_focus(Client *c);
static int32_t client_wants_fullscreen(Client *c);
static bool client_request_minimize(Client *c, void *data);
static bool client_request_maximize(Client *c, void *data);
static void client_set_size_bound(Client *c);
static void client_swap_layout_properties(Client *c1, Client *c2);
static void client_swap_monitors_and_tags(Client *c1, Client *c2);
static void finish_exchange_arrange_and_focus(Client *c1, Client *c2,
											  Monitor *m1, Monitor *m2);
void client_tile_resize(Client *c, struct wlr_box geo, int32_t interact);
uint32_t generate_client_id(void);
void client_active(Client *c);
void client_pending_force_kill(Client *c);
void client_add_jump_label_node(Client *c);
void client_add_group_bar(Client *c);
void client_focus_group_member(Client *c);
void client_check_tab_node_visible(Client *c);
void client_raise_group(Client *c);
void client_handle_decorate_click(MangoGroupBar *gb);
void client_set_group_mon(Client *c, Monitor *m);
void client_group_detach(Client *c);
void client_group_replace(Client *old, Client *new);
void mango_surface_frame_done(struct wlr_surface *surface, int sx, int sy,
							  void *data);
bool client_force_render(Client *c);
int32_t is_single_bit_set(uint32_t x);
static bool layer_ignores_focus(LayerSurface *l);

/* 重排后补充的前置声明 */
static void iter_xdg_scene_buffers(struct wlr_scene_buffer *buffer, int32_t sx,
								   int32_t sy, void *user_data);
static void unminimize(Client *c);
Client *termforwin(Client *w);
Client *get_client_by_id_or_title(const char *arg_id, const char *arg_title);
Client *center_tiled_select(Monitor *m);
Client *find_client_by_direction(Client *tc, const Arg *arg, bool findfloating);
Client *direction_select(const Arg *arg);
Client *focustop(Monitor *m);
Client *get_next_stack_client(Client *c, bool reverse);
float *get_border_color(Client *c);
Client *get_focused_stack_client(Client *sc, Client *custom_focus_client);
Client *xytoclient(double x, double y);
Monitor *dirtomon(enum wlr_direction dir);
Monitor *xytomon(double x, double y);
Monitor *get_monitor_nearest_to(int32_t lx, int32_t ly);

#include "data/static_keymap.h"
#include "dispatch/bind_declare.h"
#include "layout/layout.h"

/* variables */
static const char broken[] = "broken";
static pid_t child_pid = -1;
static int32_t locked;
static uint32_t locked_mods = 0;
static void *exclusive_focus;
static struct wl_display *dpy;
static struct wl_event_loop *event_loop;
static struct wlr_backend *backend;
static struct wlr_backend *headless_backend;
static struct wlr_scene *scene;
static struct wlr_scene_tree *layers[NUM_LAYERS];
static struct wlr_renderer *drw;
static struct wlr_allocator *alloc;
static struct wlr_compositor *compositor;

static struct wlr_xdg_shell *xdg_shell;
static struct wlr_xdg_decoration_manager_v1 *xdg_decoration_mgr;
static struct wl_list clients; /* tiling order */
static struct wl_list fstack;  /* focus order */
static struct wl_list fadeout_clients;
static struct wl_list fadeout_layers;
static struct wlr_idle_notifier_v1 *idle_notifier;
static struct wlr_idle_inhibit_manager_v1 *idle_inhibit_mgr;
static struct wlr_layer_shell_v1 *layer_shell;
static struct wlr_output_manager_v1 *output_mgr;
static struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard_mgr;
static struct wlr_keyboard_shortcuts_inhibit_manager_v1
	*keyboard_shortcuts_inhibit;
static struct wlr_virtual_pointer_manager_v1 *virtual_pointer_mgr;
static struct wlr_output_power_manager_v1 *power_mgr;
static struct wlr_ext_image_copy_capture_manager_v1 *ext_image_copy_capture_mgr;
static struct wlr_pointer_gestures_v1 *pointer_gestures;
static struct wlr_drm_lease_v1_manager *drm_lease_manager;
struct mango_print_status_manager *print_status_manager;

static struct wlr_cursor *cursor;
static struct wlr_xcursor_manager *cursor_mgr;
static struct wlr_session *session;

static struct wlr_scene_rect *root_bg;
static struct wlr_session_lock_manager_v1 *session_lock_mgr;
static struct wlr_scene_rect *locked_bg;
static struct wlr_session_lock_v1 *cur_lock;
static const int32_t layermap[] = {LyrBg, LyrBottom, LyrTop, LyrOverlay};
static struct wlr_scene_tree *drag_icon;
static struct wlr_cursor_shape_manager_v1 *cursor_shape_mgr;
static struct wlr_pointer_constraints_v1 *pointer_constraints;
static struct wlr_relative_pointer_manager_v1 *relative_pointer_mgr;
static struct wlr_pointer_constraint_v1 *active_constraint;

static struct wlr_seat *seat;
static KeyboardGroup *kb_group;
static struct wlr_keyboard
	*last_active_keyboard; /* 最后按键的键盘，get keyboardlayout 用 */
static struct wl_list inputdevices;
static struct wl_list standalone_keyboards; /* 独立键盘链表 */
static struct wl_list keyboard_shortcut_inhibitors;
static uint32_t cursor_mode;
static Client *grabc, *dropc;
static int32_t rzcorner;
static int32_t grabcx, grabcy; /* client-relative */

static struct wlr_ext_foreign_toplevel_image_capture_source_manager_v1
	*ext_foreign_toplevel_image_capture_source_manager_v1;
static struct wl_listener new_foreign_toplevel_capture_request;
static struct wlr_ext_foreign_toplevel_list_v1 *foreign_toplevel_list;

static int32_t drag_begin_cursorx, drag_begin_cursory; /* client-relative */
static bool start_drag_window = false;
static int32_t last_apply_drap_time = 0;

static struct wlr_output_layout *output_layout;
static struct wlr_box sgeom;
static struct wl_list mons;
static Monitor *selmon;
static struct wlr_scene_output_layout *scene_layout;

static int32_t enablegaps = 1; /* enables gaps, used by togglegaps */
static int32_t axis_apply_time = 0;
static int32_t axis_apply_dir = 0;
static int32_t scroller_focus_lock = 0;

static uint32_t swipe_fingers = 0;
static double swipe_dx = 0;
static double swipe_dy = 0;

bool render_border = true;

uint32_t chvt_backup_tag = 0;
bool allow_frame_scheduling = true;
char chvt_backup_selmon[32] = {0};

struct dvec2 *baked_points_move;
struct dvec2 *baked_points_open;
struct dvec2 *baked_points_tag;
struct dvec2 *baked_points_close;
struct dvec2 *baked_points_focus;
struct dvec2 *baked_points_opafadein;
struct dvec2 *baked_points_opafadeout;

static struct wl_event_source *hide_cursor_source;
static struct wl_event_source *keep_idle_inhibit_source;
static bool cursor_hidden = false;
static bool tag_combo = false;
static char cli_config_path[1024] = {0};
static int active_capture_count = 0;
static bool cli_debug_log = false;
static uint32_t last_hold_keycode = 0;

static KeyMode keymode = {
	.mode = {'d', 'e', 'f', 'a', 'u', 'l', 't', '\0'},
	.isdefault = true,
};

static char *env_vars[] = {"DISPLAY",
						   "WAYLAND_DISPLAY",
						   "XDG_CURRENT_DESKTOP",
						   "XDG_SESSION_TYPE",
						   "XCURSOR_THEME",
						   "XCURSOR_SIZE",
						   "MANGO_INSTANCE_SIGNATURE",
						   NULL};
#include "common/log.h"
#include "config/parse_config.h"

static struct wl_signal mango_print_status;

static struct wl_listener print_status_listener = {.notify =
													   handle_print_status};
static struct wl_listener cursor_axis = {.notify = axisnotify};
static struct wl_listener cursor_button = {.notify = buttonpress};
static struct wl_listener cursor_frame = {.notify = cursorframe};
static struct wl_listener cursor_motion = {.notify = motionrelative};
static struct wl_listener cursor_motion_absolute = {.notify = motionabsolute};
static struct wl_listener gpu_reset = {.notify = gpureset};
static struct wl_listener layout_change = {.notify = updatemons};
static struct wl_listener new_idle_inhibitor = {.notify = createidleinhibitor};
static struct wl_listener new_input_device = {.notify = inputdevice};
static struct wl_listener new_virtual_keyboard = {.notify = virtualkeyboard};
static struct wl_listener new_virtual_pointer = {.notify = virtualpointer};
static struct wl_listener new_pointer_constraint = {
	.notify = createpointerconstraint};
static struct wl_listener new_output = {.notify = createmon};
static struct wl_listener new_xdg_toplevel = {.notify = createnotify};
static struct wl_listener new_xdg_popup = {.notify = createpopup};
static struct wl_listener new_xdg_decoration = {.notify = createdecoration};
static struct wl_listener new_layer_surface = {.notify = createlayersurface};
static struct wl_listener output_mgr_apply = {.notify = outputmgrapply};
static struct wl_listener output_mgr_test = {.notify = outputmgrtest};
static struct wl_listener output_power_mgr_set_mode = {.notify =
														   powermgrsetmode};
static struct wl_listener ext_image_copy_capture_mgr_new_session = {
	.notify = handle_iamge_copy_capture_new_session};
static struct wl_listener request_cursor = {.notify = setcursor};
static struct wl_listener request_set_psel = {.notify = setpsel};
static struct wl_listener request_set_sel = {.notify = setsel};
static struct wl_listener request_set_cursor_shape = {.notify = setcursorshape};
static struct wl_listener request_start_drag = {.notify = requeststartdrag};
static struct wl_listener start_drag = {.notify = startdrag};
static struct wl_listener new_session_lock = {.notify = locksession};
static struct wl_listener drm_lease_request = {.notify = requestdrmlease};
static struct wl_listener keyboard_shortcuts_inhibit_new_inhibitor = {
	.notify = handle_keyboard_shortcuts_inhibit_new_inhibitor};
static struct wl_listener last_cursor_surface_destroy_listener = {
	.notify = last_cursor_surface_destroy};

#ifdef XWAYLAND
static float xwayland_client_scale(Client *c);
static float xwayland_preferred_scale(Client *c);
static void xwayland_apply_scale(Client *c);
static void xwayland_logical_to_x11(struct wlr_box *box, float scale);
static void xwayland_x11_to_logical(struct wlr_box *box, float scale);
static bool
xwayland_scene_buffer_point_accepts_input(struct wlr_scene_buffer *buffer,
										  double *sx, double *sy);
static void fix_xwayland_coordinate(struct wlr_box *geom);
static int32_t synckeymap(void *data);
static void activatex11(struct wl_listener *listener, void *data);
static void configurex11(struct wl_listener *listener, void *data);
static void createnotifyx11(struct wl_listener *listener, void *data);
static void dissociatex11(struct wl_listener *listener, void *data);
static void commitx11(struct wl_listener *listener, void *data);
static void associatex11(struct wl_listener *listener, void *data);
static void sethints(struct wl_listener *listener, void *data);
static void xwaylandready(struct wl_listener *listener, void *data);
static void setgeometrynotify(struct wl_listener *listener, void *data);
static struct wl_listener new_xwayland_surface = {.notify = createnotifyx11};
static struct wl_listener xwayland_ready = {.notify = xwaylandready};
static struct wlr_xwayland *xwayland;
static struct wl_event_source *sync_keymap;
#endif

/* export an activation token for the process we're about to spawn */

static void ipc_notify_device_event(struct wlr_input_device *device);

#include "animation/client.h"
#include "animation/common.h"
#include "animation/layer.h"
#include "animation/tag.h"
#include "dispatch/bind_define.h"
#include "ext-protocol/all.h"
#include "input/touch.h"
#include "input/switch.h"
#include "input/tablet.h"
#include "ipc/ipc.h"
#include "layout/arrange.h"
#include "layout/dwindle.h"
#include "layout/horizontal.h"
#include "layout/overview.h"
#include "layout/scroll.h"
#include "layout/vertical.h"
#include "overview/overview.h"
#include "switcher/switcher.h"

#include "manage/client.h"
#include "input/keyboard.h"
#include "manage/layer.h"
#include "input/pointer.h"
#include "manage/monitor.h"
#include "input/device.h"
#include "manage/misc.h"

void handlesig(int32_t signo) {
	if (signo == SIGCHLD)
		while (waitpid(-1, NULL, WNOHANG) > 0)
			;
	else if (signo == SIGINT || signo == SIGTERM)
		quit(NULL);
}

void cleanuplisteners(void) {
	wl_list_remove(&ext_manager_commit_listener.link); // 0.7
	wl_list_remove(&print_status_listener.link);
	wl_list_remove(&cursor_axis.link);
	wl_list_remove(&cursor_button.link);
	wl_list_remove(&cursor_frame.link);
	wl_list_remove(&cursor_motion.link);
	wl_list_remove(&cursor_motion_absolute.link);
	wl_list_remove(&cursor_touch_down.link);
	wl_list_remove(&cursor_touch_up.link);
	wl_list_remove(&cursor_touch_cancel.link);
	wl_list_remove(&cursor_touch_motion.link);
	wl_list_remove(&cursor_touch_frame.link);
	wl_list_remove(&tablet_tool_proximity.link);
	wl_list_remove(&tablet_tool_axis.link);
	wl_list_remove(&tablet_tool_button.link);
	wl_list_remove(&tablet_tool_tip.link);
	wl_list_remove(&gpu_reset.link);
	wl_list_remove(&new_idle_inhibitor.link);
	wl_list_remove(&layout_change.link);
	wl_list_remove(&new_input_device.link);
	wl_list_remove(&new_virtual_keyboard.link);
	wl_list_remove(&new_virtual_pointer.link);
	wl_list_remove(&new_pointer_constraint.link);
	wl_list_remove(&new_output.link);
	wl_list_remove(&new_xdg_toplevel.link);
	wl_list_remove(&new_xdg_decoration.link);
	wl_list_remove(&new_xdg_popup.link);
	wl_list_remove(&new_layer_surface.link);
	wl_list_remove(&output_mgr_apply.link);
	wl_list_remove(&output_mgr_test.link);
	wl_list_remove(&output_power_mgr_set_mode.link);
	wl_list_remove(&request_cursor.link);
	wl_list_remove(&request_set_psel.link);
	wl_list_remove(&request_set_sel.link);
	wl_list_remove(&request_set_cursor_shape.link);
	wl_list_remove(&request_start_drag.link);
	wl_list_remove(&start_drag.link);
	wl_list_remove(&new_session_lock.link);
	wl_list_remove(&new_foreign_toplevel_capture_request.link);
	wl_list_remove(&tearing_new_object.link);
	wl_list_remove(&keyboard_shortcuts_inhibit_new_inhibitor.link);
	wl_list_remove(&ext_image_copy_capture_mgr_new_session.link);
	if (drm_lease_manager) {
		wl_list_remove(&drm_lease_request.link);
	}
#ifdef XWAYLAND
	wl_list_remove(&new_xwayland_surface.link);
	wl_list_remove(&xwayland_ready.link);
#endif
}

void cleanup(void) {
	allow_frame_scheduling = false;

	ipc_cleanup();
	cleanuplisteners();
#ifdef XWAYLAND
	wlr_xwayland_destroy(xwayland);
	xwayland = NULL;
#endif

	wl_display_destroy_clients(dpy);
	if (child_pid > 0) {
		kill(-child_pid, SIGTERM);
		waitpid(child_pid, NULL, 0);
	}
	wlr_xcursor_manager_destroy(cursor_mgr);

	destroykeyboardgroup(&kb_group->destroy, NULL);

	mango_im_relay_finish(mango_input_method_relay);

	/* If it's not destroyed manually it will cause a use-after-free of
	 * wlr_seat. Destroy it until it's fixed in the wlroots side */
	wlr_backend_destroy(backend);

	wl_display_destroy(dpy);
	/* Destroy after the wayland display (when the monitors are already
	   destroyed) to avoid destroying them with an invalid scene output. */
	wlr_scene_node_destroy(&scene->tree.node);

	mango_text_global_finish();
}

// 修改printstatus函数，接受掩码参数

void quitsignal(int32_t signo) { quit(NULL); }

void set_activation_env() {
	if (!getenv("DBUS_SESSION_BUS_ADDRESS")) {
		mango_error(true, WLR_INFO,
					"Not updating dbus execution environment: "
					"DBUS_SESSION_BUS_ADDRESS not set");
		return;
	}

	mango_error(true, WLR_INFO, "Updating dbus execution environment");

	char *env_keys = join_strings(env_vars, " ");

	// first command: dbus-update-activation-environment
	const char *arg1 = env_keys;
	char *cmd1 = string_printf("dbus-update-activation-environment %s", arg1);
	if (!cmd1) {
		mango_error(true, WLR_ERROR, "Failed to allocate command string");
		goto cleanup;
	}
	spawn(&(Arg){.v = cmd1});
	free(cmd1);

	// second command: systemctl --user
	const char *action = "import-environment";
	char *cmd2 = string_printf("systemctl --user %s %s", action, env_keys);
	if (!cmd2) {
		mango_error(true, WLR_ERROR, "Failed to allocate command string");
		goto cleanup;
	}
	spawn(&(Arg){.v = cmd2});
	free(cmd2);

cleanup:
	free(env_keys);
}

void // 17
run(char *startup_cmd, int readiness_fd) {
	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(dpy);
	if (!socket)
		die("startup: display_add_socket_auto");
	setenv("WAYLAND_DISPLAY", socket, 1);

	set_env_display();

	/* Start the backend. This will enumerate outputs and inputs, become the
	 * DRM master, etc */
	if (!wlr_backend_start(backend))
		die("startup: backend_start");

	/* Now that the socket exists and the backend is started, run the
	 * startup command */

	if (startup_cmd) {
		int32_t piperw[2];
		if (pipe(piperw) < 0)
			die("startup: pipe:");
		if ((child_pid = fork()) < 0)
			die("startup: fork:");
		if (child_pid == 0) {
			setsid();
			dup2(piperw[0], STDIN_FILENO);
			close(piperw[0]);
			close(piperw[1]);
			execl("/bin/sh", "/bin/sh", "-c", startup_cmd, NULL);
			die("startup: execl:");
		}
		dup2(piperw[1], STDOUT_FILENO);
		close(piperw[1]);
		close(piperw[0]);
	}

	/* Mark stdout as non-blocking to avoid people who does not close stdin
	 * nor consumes it in their startup script getting dwl frozen */
	if (fd_set_nonblock(STDOUT_FILENO) < 0)
		close(STDOUT_FILENO);

	printstatus(IPC_WATCH_ARRANGGE);

	/* At this point the outputs are initialized, choose initial selmon
	 * based on cursor position, and set default cursor image */
	selmon = xytomon(cursor->x, cursor->y);

	/* TODO hack to get cursor to display in its initial location (100, 100)
	 * instead of (0, 0) and then jumping. still may not be fully
	 * initialized, as the image/coordinates are not transformed for the
	 * monitor when displayed here */
	wlr_cursor_warp_closest(cursor, NULL, cursor->x, cursor->y);
	wlr_cursor_set_xcursor(cursor, cursor_mgr, "left_ptr");
	handlecursoractivity();

	set_activation_env();

	run_exec();
	run_exec_once();

	/*
	 * If running inside supervision suite like s6, notify about successfull
	 * startup by writing \n to the provided file descriptor and closing it
	 */
	if (readiness_fd > 2) {
		write(readiness_fd, "\n", 1);
		close(readiness_fd);
	}

	/* Run the Wayland event loop. This does not return until you exit the
	 * compositor. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */

	wl_display_run(dpy);
}

// 修改信号处理函数，接收掩码参数

void setup(void) {
	setenv("XDG_CURRENT_DESKTOP", "mango", 1);
	setenv("_JAVA_AWT_WM_NONREPARENTING", "1", 1);

	parse_config();
	if (cli_debug_log) {
		config.log_level = WLR_DEBUG;
	}
	init_baked_points();

	set_env_without_display();

	int32_t drm_fd, i;
	int32_t sig[] = {SIGCHLD, SIGINT,
					 SIGTERM}; // 不设置SIGPIPE,因为ipc发送失败不应该影响主程序
	struct sigaction sa = {.sa_flags = SA_RESTART, .sa_handler = handlesig};
	sigemptyset(&sa.sa_mask);

	for (i = 0; i < LENGTH(sig); i++)
		sigaction(sig[i], &sa, NULL);

	// 单独为 SIGPIPE 设置忽略
	struct sigaction sa_pipe = {.sa_flags = 0, .sa_handler = SIG_IGN};
	sigemptyset(&sa_pipe.sa_mask);
	sigaction(SIGPIPE, &sa_pipe, NULL);

	wlr_log_init(config.log_level, NULL);

	/* The Wayland display is managed by libwayland. It handles accepting
	 * clients from the Unix socket, manging Wayland globals, and so on. */
	dpy = wl_display_create();

	wl_display_set_default_max_buffer_size(dpy, 1024 * 1024);

	event_loop = wl_display_get_event_loop(dpy);

	ipc_init(event_loop);

	tablet_mgr = wlr_tablet_v2_create(dpy);
	/* The backend is a wlroots feature which abstracts the underlying input
	 * and output hardware. The autocreate option will choose the most
	 * suitable backend based on the current environment, such as opening an
	 * X11 window if an X11 server is running. The NULL argument here
	 * optionally allows you to pass in a custom renderer if wlr_renderer
	 * doesn't meet your needs. The backend uses the renderer, for example,
	 * to fall back to software cursors if the backend does not support
	 * hardware cursors (some older GPUs don't). */
	if (!(backend = wlr_backend_autocreate(event_loop, &session)))
		die("couldn't create backend");

	headless_backend = wlr_headless_backend_create(event_loop);
	if (!headless_backend) {
		mango_error(true, WLR_ERROR,
					"Failed to create secondary headless backend");
	} else {
		wlr_multi_backend_add(backend, headless_backend);
	}

	/* Initialize the scene graph used to lay out windows */
	scene = wlr_scene_create();
	root_bg = wlr_scene_rect_create(&scene->tree, 0, 0, config.rootcolor);
	for (i = 0; i < NUM_LAYERS; i++)
		layers[i] = wlr_scene_tree_create(&scene->tree);
	drag_icon = wlr_scene_tree_create(&scene->tree);
	wlr_scene_node_place_below(&drag_icon->node, &layers[LyrBlock]->node);

	/* Create a renderer with the default implementation */
	if (!(drw = fx_renderer_create(backend)))
		die("couldn't create renderer");

	if (drw->features.input_color_transform) {
		const enum wp_color_manager_v1_render_intent render_intents[] = {
			WP_COLOR_MANAGER_V1_RENDER_INTENT_PERCEPTUAL,
		};
		size_t transfer_functions_len = 0;
		enum wp_color_manager_v1_transfer_function *transfer_functions =
			wlr_color_manager_v1_transfer_function_list_from_renderer(
				drw, &transfer_functions_len);

		size_t primaries_len = 0;
		enum wp_color_manager_v1_primaries *primaries =
			wlr_color_manager_v1_primaries_list_from_renderer(drw,
															  &primaries_len);

		struct wlr_color_manager_v1 *cm = wlr_color_manager_v1_create(
			dpy, 2,
			&(struct wlr_color_manager_v1_options){
				.features =
					{
						.parametric = true,
						.set_mastering_display_primaries = true,
					},
				.render_intents = render_intents,
				.render_intents_len = ARRAY_SIZE(render_intents),
				.transfer_functions = transfer_functions,
				.transfer_functions_len = transfer_functions_len,
				.primaries = primaries,
				.primaries_len = primaries_len,
			});

		free(transfer_functions);
		free(primaries);

		if (cm) {
			wlr_scene_set_color_manager_v1(scene, cm);
		} else {
			mango_error(true, WLR_ERROR, "unable to create color manager");
		}
	}

	wlr_color_representation_manager_v1_create_with_renderer(dpy, 1, drw);

	wl_signal_add(&drw->events.lost, &gpu_reset);

	/* Create shm, drm and linux_dmabuf interfaces by ourselves.
	 * The simplest way is call:
	 *      wlr_renderer_init_wl_display(drw);
	 * but we need to create manually the linux_dmabuf interface to
	 * integrate it with wlr_scene. */
	wlr_renderer_init_wl_shm(drw, dpy);

	if (wlr_renderer_get_texture_formats(drw, WLR_BUFFER_CAP_DMABUF)) {
		wlr_drm_create(dpy, drw);
		wlr_scene_set_linux_dmabuf_v1(
			scene, wlr_linux_dmabuf_v1_create_with_renderer(dpy, 4, drw));
	}

	if (config.syncobj_enable && (drm_fd = wlr_renderer_get_drm_fd(drw)) >= 0 &&
		drw->features.timeline && backend->features.timeline)
		wlr_linux_drm_syncobj_manager_v1_create(dpy, 1, drm_fd);

	/* Create a default allocator */
	if (!(alloc = wlr_allocator_autocreate(backend, drw)))
		die("couldn't create allocator");

	/* This creates some hands-off wlroots interfaces. The compositor is
	 * necessary for clients to allocate surfaces and the data device
	 * manager handles the clipboard. Each of these wlroots interfaces has
	 * room for you to dig your fingers in and play with their behavior if
	 * you want. Note that the clients cannot set the selection directly
	 * without compositor approval, see the setsel() function. */
	compositor = wlr_compositor_create(dpy, 6, drw);
	wlr_export_dmabuf_manager_v1_create(dpy);
	wlr_screencopy_manager_v1_create(dpy);
	wlr_ext_output_image_capture_source_manager_v1_create(dpy, 1);
	wlr_data_control_manager_v1_create(dpy);
	wlr_data_device_manager_create(dpy);
	wlr_primary_selection_v1_device_manager_create(dpy);
	wlr_viewporter_create(dpy);
	wlr_single_pixel_buffer_manager_v1_create(dpy);
	wlr_fractional_scale_manager_v1_create(dpy, 1);
	wlr_presentation_create(dpy, backend, 2);
	wlr_subcompositor_create(dpy);
	wlr_alpha_modifier_v1_create(dpy);
	wlr_ext_data_control_manager_v1_create(dpy, 1);
	wlr_fixes_create(dpy, 1);

	// 在 setup 函数中
	wl_signal_init(&mango_print_status);
	wl_signal_add(&mango_print_status, &print_status_listener);

	ext_image_copy_capture_mgr =
		wlr_ext_image_copy_capture_manager_v1_create(dpy, 1);
	wl_signal_add(&ext_image_copy_capture_mgr->events.new_session,
				  &ext_image_copy_capture_mgr_new_session);

	xdg_activation_init();

	wlr_scene_set_gamma_control_manager_v1(
		scene, wlr_gamma_control_manager_v1_create(dpy));

	power_mgr = wlr_output_power_manager_v1_create(dpy);
	wl_signal_add(&power_mgr->events.set_mode, &output_power_mgr_set_mode);

	foreign_toplevel_list = wlr_ext_foreign_toplevel_list_v1_create(dpy, 1);
	ext_foreign_toplevel_image_capture_source_manager_v1 =
		wlr_ext_foreign_toplevel_image_capture_source_manager_v1_create(dpy, 1);
	new_foreign_toplevel_capture_request.notify =
		handle_new_foreign_toplevel_capture_request;
	wl_signal_add(&ext_foreign_toplevel_image_capture_source_manager_v1->events
					   .new_request,
				  &new_foreign_toplevel_capture_request);

	tearing_control = wlr_tearing_control_manager_v1_create(dpy, 1);
	tearing_new_object.notify = handle_tearing_new_object;
	wl_signal_add(&tearing_control->events.new_object, &tearing_new_object);

	/* Creates an output layout, which a wlroots utility for working with an
	 * arrangement of screens in a physical layout. */
	output_layout = wlr_output_layout_create(dpy);
	wl_signal_add(&output_layout->events.change, &layout_change);
	/* 自定义 xdg-output：对 XWayland 单独处理 */
	xdg_output_init();

	/* Configure a listener to be notified when new outputs are available on
	 * the backend. */
	wl_list_init(&mons);
	wl_signal_add(&backend->events.new_output, &new_output);
	scene_layout = wlr_scene_attach_output_layout(scene, output_layout);

	/* Set up our client lists and the xdg-shell. The xdg-shell is a
	 * Wayland protocol which is used for application windows. For more
	 * detail on shells, refer to the article:
	 *
	 * https://drewdevault.com/2018/07/29/Wayland-shells.html
	 */
	wl_list_init(&clients);
	wl_list_init(&fstack);
	wl_list_init(&fadeout_clients);
	wl_list_init(&fadeout_layers);

	idle_notifier = wlr_idle_notifier_v1_create(dpy);

	idle_inhibit_mgr = wlr_idle_inhibit_v1_create(dpy);
	wl_signal_add(&idle_inhibit_mgr->events.new_inhibitor, &new_idle_inhibitor);

	keep_idle_inhibit_source = wl_event_loop_add_timer(
		wl_display_get_event_loop(dpy), keep_idle_inhibit, NULL);

	layer_shell = wlr_layer_shell_v1_create(dpy, 4);
	wl_signal_add(&layer_shell->events.new_surface, &new_layer_surface);

	xdg_shell = wlr_xdg_shell_create(dpy, 6);
	wl_signal_add(&xdg_shell->events.new_toplevel, &new_xdg_toplevel);
	wl_signal_add(&xdg_shell->events.new_popup, &new_xdg_popup);

	session_lock_mgr = wlr_session_lock_manager_v1_create(dpy);
	wl_signal_add(&session_lock_mgr->events.new_lock, &new_session_lock);

	locked_bg =
		wlr_scene_rect_create(layers[LyrBlock], sgeom.width, sgeom.height,
							  (float[4]){0.1, 0.1, 0.1, 1.0});
	wlr_scene_node_set_enabled(&locked_bg->node, false);

	/* Use decoration protocols to negotiate server-side decorations */
	wlr_server_decoration_manager_set_default_mode(
		wlr_server_decoration_manager_create(dpy),
		WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
	xdg_decoration_mgr = wlr_xdg_decoration_manager_v1_create(dpy);
	wl_signal_add(&xdg_decoration_mgr->events.new_toplevel_decoration,
				  &new_xdg_decoration);

	pointer_constraints = wlr_pointer_constraints_v1_create(dpy);
	wl_signal_add(&pointer_constraints->events.new_constraint,
				  &new_pointer_constraint);

	relative_pointer_mgr = wlr_relative_pointer_manager_v1_create(dpy);

	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(cursor, output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
	 * Xcursor themes to source cursor images from and makes sure that
	 * cursor images are available at all scale factors on the screen
	 * (necessary for HiDPI support). Scaled cursors will be loaded with
	 * each output. */

	set_xcursor_env();

	cursor_mgr =
		wlr_xcursor_manager_create(config.cursor_theme, config.cursor_size);
	/*
	 * wlr_cursor *only* displays an image on screen. It does not move
	 * around when the pointer moves. However, we can attach input devices
	 * to it, and it will generate aggregate events for all of them. In
	 * these events, we can choose how we want to process them, forwarding
	 * them to clients and moving the cursor around. More detail on this
	 * process is described in my input handling blog post:
	 *
	 * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
	 *
	 * And more comments are sprinkled throughout the notify functions
	 * above.
	 */
	wl_signal_add(&cursor->events.motion, &cursor_motion);
	wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
	wl_signal_add(&cursor->events.button, &cursor_button);
	wl_signal_add(&cursor->events.axis, &cursor_axis);
	wl_signal_add(&cursor->events.frame, &cursor_frame);
	wl_signal_add(&cursor->events.tablet_tool_proximity,
				  &tablet_tool_proximity);
	wl_signal_add(&cursor->events.tablet_tool_axis, &tablet_tool_axis);
	wl_signal_add(&cursor->events.tablet_tool_button, &tablet_tool_button);
	wl_signal_add(&cursor->events.tablet_tool_tip, &tablet_tool_tip);

	// 这两句代码会造成obs窗口里的鼠标光标消失,不知道注释有什么影响
	cursor_shape_mgr = wlr_cursor_shape_manager_v1_create(dpy, 1);
	wl_signal_add(&cursor_shape_mgr->events.request_set_shape,
				  &request_set_cursor_shape);
	hide_cursor_source = wl_event_loop_add_timer(wl_display_get_event_loop(dpy),
												 hidecursor, cursor);
	/*
	 * Configures a seat, which is a single "seat" at which a user sits and
	 * operates the computer. This conceptually includes up to one keyboard,
	 * pointer, touch, and drawing tablet device. We also rig up a listener
	 * to let us know when new input devices are available on the backend.
	 */
	wl_list_init(&inputdevices);
	wl_list_init(&standalone_keyboards);
	wl_list_init(&tablets);
	wl_list_init(&tablet_pads);
	wl_list_init(&keyboard_shortcut_inhibitors);
	wl_signal_add(&backend->events.new_input, &new_input_device);
	virtual_keyboard_mgr = wlr_virtual_keyboard_manager_v1_create(dpy);
	wl_signal_add(&virtual_keyboard_mgr->events.new_virtual_keyboard,
				  &new_virtual_keyboard);
	virtual_pointer_mgr = wlr_virtual_pointer_manager_v1_create(dpy);
	wl_signal_add(&virtual_pointer_mgr->events.new_virtual_pointer,
				  &new_virtual_pointer);

	pointer_gestures = wlr_pointer_gestures_v1_create(dpy);
	LISTEN_STATIC(&cursor->events.swipe_begin, swipe_begin);
	LISTEN_STATIC(&cursor->events.swipe_update, swipe_update);
	LISTEN_STATIC(&cursor->events.swipe_end, swipe_end);
	LISTEN_STATIC(&cursor->events.pinch_begin, pinch_begin);
	LISTEN_STATIC(&cursor->events.pinch_update, pinch_update);
	LISTEN_STATIC(&cursor->events.pinch_end, pinch_end);
	LISTEN_STATIC(&cursor->events.hold_begin, hold_begin);
	LISTEN_STATIC(&cursor->events.hold_end, hold_end);

	/* 触摸支持：初始化触点链表并连接 cursor 触摸事件 */
	wl_list_init(&touch_points);
	wl_signal_add(&cursor->events.touch_down, &cursor_touch_down);
	wl_signal_add(&cursor->events.touch_up, &cursor_touch_up);
	wl_signal_add(&cursor->events.touch_cancel, &cursor_touch_cancel);
	wl_signal_add(&cursor->events.touch_motion, &cursor_touch_motion);
	wl_signal_add(&cursor->events.touch_frame, &cursor_touch_frame);

	seat = wlr_seat_create(dpy, "seat0");

	wl_list_init(&last_cursor_surface_destroy_listener.link);
	wl_signal_add(&seat->events.request_set_cursor, &request_cursor);
	wl_signal_add(&seat->events.request_set_selection, &request_set_sel);
	wl_signal_add(&seat->events.request_set_primary_selection,
				  &request_set_psel);
	wl_signal_add(&seat->events.request_start_drag, &request_start_drag);
	wl_signal_add(&seat->events.start_drag, &start_drag);

	kb_group = createkeyboardgroup();
	last_active_keyboard = kb_group->keyboard;
	wl_list_init(&kb_group->destroy.link);

	keyboard_shortcuts_inhibit = wlr_keyboard_shortcuts_inhibit_v1_create(dpy);
	wl_signal_add(&keyboard_shortcuts_inhibit->events.new_inhibitor,
				  &keyboard_shortcuts_inhibit_new_inhibitor);

	output_mgr = wlr_output_manager_v1_create(dpy);
	wl_signal_add(&output_mgr->events.apply, &output_mgr_apply);
	wl_signal_add(&output_mgr->events.test, &output_mgr_test);

	wlr_scene_set_blur_data(
		scene, config.blur_params.num_passes, config.blur_params.radius,
		config.blur_params.noise, config.blur_params.brightness,
		config.blur_params.contrast, config.blur_params.saturation);

	/* create text_input-, and input_method-protocol relevant globals */
	input_method_manager = wlr_input_method_manager_v2_create(dpy);
	text_input_manager = wlr_text_input_manager_v3_create(dpy);

	mango_input_method_relay = mango_im_relay_create();

	drm_lease_manager = wlr_drm_lease_v1_manager_create(dpy, backend);
	if (drm_lease_manager) {
		wl_signal_add(&drm_lease_manager->events.request, &drm_lease_request);
	} else {
		mango_error(true, WLR_DEBUG,
					"Failed to create wlr_drm_lease_device_v1.");
		mango_error(true, WLR_INFO, "VR will not be available.");
	}

	// 创建顶层管理句柄
	foreign_toplevel_manager = wlr_foreign_toplevel_manager_v1_create(dpy);
	struct wlr_xdg_foreign_registry *foreign_registry =
		wlr_xdg_foreign_registry_create(dpy);
	wlr_xdg_foreign_v1_create(dpy, foreign_registry);
	wlr_xdg_foreign_v2_create(dpy, foreign_registry);

	// ext-workspace协议
	workspaces_init();
#ifdef XWAYLAND
	/*
	 * Initialise the XWayland X server.
	 * It will be started when the first X client is started.
	 */
	xwayland =
		wlr_xwayland_create(dpy, compositor, !config.xwayland_persistence);
	if (xwayland) {
		wl_signal_add(&xwayland->events.ready, &xwayland_ready);
		wl_signal_add(&xwayland->events.new_surface, &new_xwayland_surface);

		setenv("DISPLAY", xwayland->display_name, 1);
	} else {
		mango_error(
			true, WLR_ERROR,
			"failed to setup XWayland X server, continuing without it\n");
	}
	sync_keymap = wl_event_loop_add_timer(wl_display_get_event_loop(dpy),
										  synckeymap, NULL);
#endif
}

int32_t main(int32_t argc, char *argv[]) {
	char *startup_cmd = NULL;
	int32_t c;
	int readiness_fd = 0;

	while ((c = getopt(argc, argv, "s:c:r:hdvp")) != -1) {
		if (c == 's') {
			startup_cmd = optarg;
		} else if (c == 'd') {
			cli_debug_log = true;
		} else if (c == 'v') {
			printf("mango " VERSION "\n");
			return EXIT_SUCCESS;
		} else if (c == 'c') {
			snprintf(cli_config_path, sizeof(cli_config_path), "%s", optarg);
		} else if (c == 'p') {
			return parse_config() ? EXIT_SUCCESS : EXIT_FAILURE;
		} else if (c == 'r') {
			readiness_fd = atoi(optarg);
			if (readiness_fd < 3) {
				goto usage;
			}
		} else {
			goto usage;
		}
	}
	if (optind < argc)
		goto usage;

	/* Wayland requires XDG_RUNTIME_DIR for creating its communications
	 * socket
	 */
	if (!getenv("XDG_RUNTIME_DIR"))
		die("XDG_RUNTIME_DIR must be set");
	setup();
	run(startup_cmd, readiness_fd);
	cleanup();
	return EXIT_SUCCESS;
usage:
	printf("Usage: mango [OPTIONS]\n"
		   "\n"
		   "Options:\n"
		   "  -v             Show mango version\n"
		   "  -d             Enable debug log\n"
		   "  -c <file>      Use custom configuration file\n"
		   "  -s <command>   Execute startup command\n"
		   "  -r <fdnum>     When WM is ready, write '\\n' to the given file "
		   "descriptor and close it. fdnum >= 3\n"
		   "  -p             Check configuration file error\n");
	return EXIT_SUCCESS;
}
