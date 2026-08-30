#ifndef __LAYOUT_LAYOUT_H__
#define __LAYOUT_LAYOUT_H__ 1

#include "horizontal.h"
#include "vertical.h"
#include "scroll.h"
#include "dwindle.h"
#include "overview.h"
#include "../mango.h"


/* layout(s) */
Layout overviewlayout = {"󰃇", overview, "overview"};

enum {
	TILE,
	SCROLLER,
	GRID,
	MONOCLE,
	DECK,
	CENTER_TILE,
	VERTICAL_SCROLLER,
	VERTICAL_TILE,
	VERTICAL_GRID,
	VERTICAL_DECK,
	RIGHT_TILE,
	DWINDLE,
	FAIR,
	VERTICAL_FAIR,
};

Layout layouts[] = {
	// 最少两个,不能删除少于两个
	/* symbol     arrange function   name */
	{"T", tile, "tile", TILE},						 // 平铺布局
	{"S", scroller, "scroller", SCROLLER},			 // 滚动布局
	{"G", grid, "grid", GRID},						 // 格子布局
	{"M", monocle, "monocle", MONOCLE},				 // 单屏布局
	{"K", deck, "deck", DECK},						 // 卡片布局
	{"CT", center_tile, "center_tile", CENTER_TILE}, // 居中布局
	{"RT", right_tile, "right_tile", RIGHT_TILE},	 // 右布局
	{"VS", vertical_scroller, "vertical_scroller",
	 VERTICAL_SCROLLER},								   // 垂直滚动布局
	{"VT", vertical_tile, "vertical_tile", VERTICAL_TILE}, // 垂直平铺布局
	{"VG", vertical_grid, "vertical_grid", VERTICAL_GRID}, // 垂直格子布局
	{"VK", vertical_deck, "vertical_deck", VERTICAL_DECK}, // 垂直卡片布局
	{"DW", dwindle, "dwindle", DWINDLE},
	{"F", fair, "fair", FAIR},
	{"VF", vertical_fair, "vertical_fair", VERTICAL_FAIR},
};

#endif
