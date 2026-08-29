#include "client.h"
#include <stdint.h>
#include "../mango.h"

void client_actual_size(Client *c, int32_t *width, int32_t *height) {
	*width = c->animation.current.width - 2 * (int32_t)c->bw;
	*height = c->animation.current.height - 2 * (int32_t)c->bw;
}
