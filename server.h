#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string.h>

#define PBUFSIZ 16384

typedef int remote_fildes_t;

/* Functions from event-loop.c.  */
typedef void *remote_client_data;
typedef int (handler_func) (int, remote_client_data);
typedef int (callback_handler_func) (remote_client_data);

extern void delete_file_handler (remote_fildes_t fd);
extern void add_file_handler (remote_fildes_t fd, handler_func *proc, remote_client_data client_data);
extern int append_callback_event (callback_handler_func *proc, remote_client_data client_data);
extern void delete_callback_event (int id);

extern void start_event_loop (void);
extern void initialize_event_loop (void);

/* Functions from server.c.  */
extern int handle_serial_event (int err, remote_client_data client_data);
extern int handle_target_event (int err, remote_client_data client_data);

#endif
