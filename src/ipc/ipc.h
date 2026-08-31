#ifndef ___IPC_H__
#define ___IPC_H__

#include <cjson/cJSON.h>
#include <stdint.h>
#include <wayland-util.h>
#include "../mango.h"

struct ipc_watch_client {
	struct wl_list link;
	int fd;
	struct wl_event_source *source;
	enum ipc_watch_type type;
	union {
		struct {
			char name[64];
		} monitor;
		struct {
			uint32_t id;
		} client;
		struct {
			char mon_name[64];
		} tags;
	} target;
};

void ipc_remove_watch_client(struct ipc_watch_client *wc);
void ipc_notify_json_to_fd(int fd, cJSON *json);

void ipc_notify_monitor(Monitor *m);

void ipc_notify_last_surface_ws_name(Monitor *m);

void ipc_notify_focusing_client(void);

void ipc_notify_device_event(struct wlr_input_device *dev);

void ipc_notify_client(Client *c);

void ipc_notify_tags(Monitor *m);

void ipc_notify_all_monitors(void);

void ipc_notify_all_clients(void);

void ipc_notify_all_tags(void);

void ipc_notify_keymode(void);

void ipc_notify_kb_layout(void);

void ipc_notify_device_event(struct wlr_input_device *dev);

void printstatus(enum ipc_watch_type type);

void handle_print_status(struct wl_listener *listener, void *data);

void ipc_init(struct wl_event_loop *event_loop);

void ipc_cleanup(void);

#endif
