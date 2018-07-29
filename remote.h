#ifndef EVENT_LOOP_REMOTE_H
#define EVENT_LOOP_REMOTE_H

typedef int remote_fildes_t;
typedef void* remote_client_data;

void remote_prepare(char *name);
void remote_open(char *name);
void remote_close(void);

void initialize_async_io(void);
void enable_async_io(void);
void disable_async_io(void);

#endif
