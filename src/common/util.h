#ifndef __COMMON_UTIL_H__
#define __COMMON_UTIL_H__ 1

/* See LICENSE.dwm file for copyright and license details. */
#include <time.h>
#include <wayland-util.h>
#include "../mango.h"
#include "../ipc/ipc.h"
#include "../input/pointer.h"
#include "../input/device.h"
#include "../input/keyboard.h"
<<<<<<< HEAD
#include "../manage/client.h"
#include "../manage/layer.h"
#include "../manage/misc.h"
#include "../manage/monitor.h"
=======
#include "../manage/monitor.h"
#include "../manage/misc.h"
#include "../manage/layer.h"
>>>>>>> 9a5a20c (fix: Add missing include)

/* Global variables */
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
#include "../config/parse_config.h"

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
float xwayland_client_scale(Client *c);
float xwayland_preferred_scale(Client *c);
void xwayland_apply_scale(Client *c);
void xwayland_logical_to_x11(struct wlr_box *box, float scale);
void xwayland_x11_to_logical(struct wlr_box *box, float scale);
bool
xwayland_scene_buffer_point_accepts_input(struct wlr_scene_buffer *buffer,
										  double *sx, double *sy);
void fix_xwayland_coordinate(struct wlr_box *geom);
int32_t synckeymap(void *data);
void activatex11(struct wl_listener *listener, void *data);
void configurex11(struct wl_listener *listener, void *data);
void createnotifyx11(struct wl_listener *listener, void *data);
void dissociatex11(struct wl_listener *listener, void *data);
void commitx11(struct wl_listener *listener, void *data);
void associatex11(struct wl_listener *listener, void *data);
void sethints(struct wl_listener *listener, void *data);
void xwaylandready(struct wl_listener *listener, void *data);
void setgeometrynotify(struct wl_listener *listener, void *data);
static struct wl_listener new_xwayland_surface = {.notify = createnotifyx11};
static struct wl_listener xwayland_ready = {.notify = xwaylandready};
static struct wlr_xwayland *xwayland;
static struct wl_event_source *sync_keymap;
#endif

// config variable defined here
Config config;

/* Functions */
void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
int32_t fd_set_nonblock(int32_t fd);
int32_t regex_match(const char *pattern_mb, const char *str_mb);
void wl_list_append(struct wl_list *list, struct wl_list *object);
uint32_t get_now_in_ms(void);
uint32_t timespec_to_ms(struct timespec *ts);
char *join_strings(char *arr[], const char *sep);
char *join_strings_with_suffix(char *arr[], const char *suffix,
							   const char *sep);
char *string_printf(const char *fmt, ...);
void wl_list_swap(struct wl_list *l1, struct wl_list *l2);
void wl_list_safe_reinsert_prev(struct wl_list *l1, struct wl_list *l2);
void wl_list_safe_reinsert_next(struct wl_list *l1, struct wl_list *l2);

#endif
