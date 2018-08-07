#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "remote.h"

typedef int (*handler_func) (int, void *);

extern void create_fd_handler(int fd, int mask, handler_func *proc, void * client_data);

#endif

