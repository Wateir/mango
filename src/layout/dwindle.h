#ifndef __DWINDLE_H__
#define __DWINDLE_H__

#include "../mango.h"

void dwindle(Monitor *m);

void dwindle_insert(DwindleNode **root, Client *new_c, Client *focused,
						   float ratio, bool as_first, bool split_h,
						   bool lock);

void cleanup_monitor_dwindle(Monitor *m);

#endif
