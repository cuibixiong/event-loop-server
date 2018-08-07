#ifndef EVENT_LOOP_REMOTE_H
#define EVENT_LOOP_REMOTE_H

#define remote_READABLE (1 << 1)
#define remote_WRITABLE (1 << 2)
#define remote_EXCEPTION (1 << 3)

void remote_prepare(char *name);
void remote_open(char *name);
void remote_close(void);

void initialize_async_io(void);
void enable_async_io(void);
void disable_async_io(void);

#endif
