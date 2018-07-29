
#include "remote.h"

#define INVALID_DESCRIPTOR -1

remote_fildes_t remote_desc = INVALID_DESCRIPTOR;
remote_fildes_t listen_desc = INVALID_DESCRIPTOR;

static int socket_read(void *buf, int count);
static int socket_write(const void *buf, int count);

static int handle_accept_event(remote_client_data client_data);


