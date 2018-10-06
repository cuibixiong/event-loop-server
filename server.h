#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string.h>

#include "remote.h"
#include "event-loop.h"

#define PBUFSIZ 16384

/* Functions from event-loop.c.  */
extern void add_fd_handler (int fd, handler_func *proc, void *client_data);
extern void delete_fd_handler (int fd);

extern void start_event_loop (void);
extern void event_initialize(void);

/* Functions from server.c.  */
extern int handle_serial_event (int err, void *client_data);
extern int handle_target_event (int err, void *client_data);

#endif
