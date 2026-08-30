#ifndef __INPUT_TABLET_H__
#define __INPUT_TABLET_H__ 1

#include <wlr/types/wlr_tablet_pad.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_v2.h>

static void createtablet(struct wlr_input_device *device);
static void destroytablet(struct wl_listener *listener, void *data);
static void createtabletpad(struct wlr_input_device *device);
static void destroytabletpad(struct wl_listener *listener, void *data);
static void tabletpadtabletdestroy(struct wl_listener *listener, void *data);
static void tabletpadattach(struct wl_listener *listener, void *data);
static void destroytabletsurfacenotify(struct wl_listener *listener,
									   void *data);
static void destroytablettool(struct wl_listener *listener, void *data);
static void tablettoolsetcursor(struct wl_listener *listener, void *data);

void tablettoolproximity(struct wl_listener *listener, void *data);
void tablettoolaxis(struct wl_listener *listener, void *data);
void tablettoolbutton(struct wl_listener *listener, void *data);
void tablettooltip(struct wl_listener *listener, void *data);
static struct wlr_tablet_manager_v2 *tablet_mgr;

struct Tablet {
	struct wlr_tablet_v2_tablet *tablet_v2;
	struct wl_listener destroy;
	struct wlr_input_device *device;
	struct wl_list link;
};
static struct wl_list tablets;

struct TabletTool {
	struct wlr_tablet_v2_tablet_tool *tool_v2;
	struct Tablet *tablet;
	struct wlr_surface *curr_surface;
	struct wl_listener destroy;
	struct wl_listener surface_destroy;
	struct wl_listener set_cursor;
	double tilt_x, tilt_y;
};

struct TabletPad {
	struct wlr_tablet_v2_tablet_pad *pad_v2;
	struct wlr_input_device *device;
	struct Tablet *tablet;
	struct wl_listener tablet_destroy;
	struct wl_listener attach;
	struct wl_listener destroy;
	struct wl_list link;
};
static struct wl_list tablet_pads;

static void attach_tablet_pad(struct TabletPad *tablet_pad,
							  struct Tablet *tablet);
static void tablettoolmotion(struct TabletTool *tool, bool change_x,
							 bool change_y, double x, double y, double dx,
							 double dy);

static struct wl_listener tablet_tool_axis = {.notify = tablettoolaxis};
static struct wl_listener tablet_tool_button = {.notify = tablettoolbutton};
static struct wl_listener tablet_tool_proximity = {.notify =
													   tablettoolproximity};
static struct wl_listener tablet_tool_tip = {.notify = tablettooltip};


#endif
