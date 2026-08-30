#ifndef __EXT_PROTOCOL_HDR_H__
#define __EXT_PROTOCOL_HDR_H__ 1

#include "../mango.h"
#include "../config/parse_config.h"
#include <stdint.h>
#include <drm_fourcc.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static uint32_t output_formats_8bit[] = {
	DRM_FORMAT_XRGB8888, DRM_FORMAT_XBGR8888, DRM_FORMAT_RGBX8888,
	DRM_FORMAT_BGRX8888, DRM_FORMAT_ARGB8888, DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888, DRM_FORMAT_BGRA8888, DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
};

static uint32_t output_formats_10bit[] = {
	DRM_FORMAT_XRGB2101010, DRM_FORMAT_XBGR2101010, DRM_FORMAT_RGBX1010102,
	DRM_FORMAT_BGRX1010102, DRM_FORMAT_ARGB2101010, DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_RGBA1010102, DRM_FORMAT_BGRA1010102,
};

/* Declarations */

static bool output_set_render_format(Monitor *m, uint32_t candidates[],
									  size_t count,
									  struct wlr_output_state *state);
static bool output_format_in_candidates(uint32_t format, uint32_t candidates[],
										size_t count);
static enum render_bit_depth bit_depth_from_format(uint32_t render_format);
static bool output_supports_hdr(const Monitor *m, const char **reason);
void output_enable_hdr(Monitor *m, struct wlr_output_state *os, bool enabled,
						bool silent);
void output_state_setup_hdr(Monitor *m, bool silent,
							struct wlr_output_state *state);
/* togglehdr[,on|off|toggle][,<monitor name>|all] -- apply to one output */
static bool togglehdr_output(Monitor *target, bool want);
void togglehdr(const Arg *arg);




#endif
