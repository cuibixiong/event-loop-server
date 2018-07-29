#ifndef EVENT_LOOP_EVENT_LOOP_H
#define EVENT_LOOP_EVENT_LOOP_H

#include "remote.h"

typedef int (* handler_func_ptr) (int, remote_client_data);

void add_fd_handler(remote_fildes_t fd, handler_func_ptr proc, remote_client_data data);
void delete_fd_handler(remote_fildes_t fd);

#endif
