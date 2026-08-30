#ifndef __OVERVIEW_H__
#define __OVERVIEW_H__

#include "../mango.h"

void overview_scale(Monitor *m);

void overview_scale_tab(Monitor *m);

void create_jump_hints(Monitor *m);

void begin_jump_mode(Monitor *m);

void finish_jump_mode(Monitor *m);

void overview(Monitor *m);

#endif
