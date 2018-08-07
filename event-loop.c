#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "server.h"
#include "remote.h"
#include "queue.h"

typedef struct remote_event remote_event;
typedef int(event_handler_func)(int);

static int handle_event(void);

typedef struct fd_handler {
    int error;
    int fd;
    int mask;
    int ready_mask;
    handler_func *proc;
    void * client_data;

    struct fd_handler *next_fd;
} fd_handler;

typedef struct remote_event {
    int fd;
    event_handler_func *proc;
} * remote_event_t;

DECLARE_QUEUE_P(remote_event_t);
static QUEUE(remote_event_t) *event_queue = NULL;
DEFINE_QUEUE_P(remote_event_t);

static struct {
    fd_handler *first_fd_handler;
    fd_set check_masks[3];
    fd_set ready_masks[3];
    int num_fds;
} remote_notifier;

void create_fd_handler(int fd, int mask, handler_func *proc, void * client_data) 
{
    fd_handler *fd_ptr;

    for (fd_ptr = remote_notifier.first_fd_handler; fd_ptr != NULL;
            fd_ptr = fd_ptr->next_fd)
        if (fd_ptr->fd == fd) break;

    if (fd_ptr == NULL) {
        fd_ptr = (fd_handler *)malloc(sizeof(*fd_ptr));
        fd_ptr->fd = fd;
        fd_ptr->ready_mask = 0;
        fd_ptr->next_fd = remote_notifier.first_fd_handler;
        remote_notifier.first_fd_handler = fd_ptr;

        if (mask & remote_READABLE)
            FD_SET(fd, &remote_notifier.check_masks[0]);
        else
            FD_CLR(fd, &remote_notifier.check_masks[0]);

        if (mask & remote_WRITABLE)
            FD_SET(fd, &remote_notifier.check_masks[1]);
        else
            FD_CLR(fd, &remote_notifier.check_masks[1]);

        if (mask & remote_EXCEPTION)
            FD_SET(fd, &remote_notifier.check_masks[2]);
        else
            FD_CLR(fd, &remote_notifier.check_masks[2]);

        if (remote_notifier.num_fds <= fd) remote_notifier.num_fds = fd + 1;
    }

    fd_ptr->proc = proc;
    fd_ptr->client_data = client_data;
    fd_ptr->mask = mask;
}

void delete_fd_handler(int fd)
{
  int i;
  fd_handler *file_ptr, *prev_ptr = NULL;

  for (file_ptr = remote_notifier.first_fd_handler; file_ptr != NULL;
       file_ptr = file_ptr->next_fd)
    if (file_ptr->fd == fd) break;

  if (file_ptr == NULL) return;

  if (file_ptr->mask & remote_READABLE)
    FD_CLR(fd, &remote_notifier.check_masks[0]);
  if (file_ptr->mask & remote_WRITABLE)
    FD_CLR(fd, &remote_notifier.check_masks[1]);
  if (file_ptr->mask & remote_EXCEPTION)
    FD_CLR(fd, &remote_notifier.check_masks[2]);

  if ((fd + 1) == remote_notifier.num_fds) {
    remote_notifier.num_fds--;
    for (i = remote_notifier.num_fds; i; i--) {
      if (FD_ISSET(i - 1, &remote_notifier.check_masks[0]) ||
          FD_ISSET(i - 1, &remote_notifier.check_masks[1]) ||
          FD_ISSET(i - 1, &remote_notifier.check_masks[2]))
        break;
    }
    remote_notifier.num_fds = i;
  }

  file_ptr->mask = 0;

  /* Get rid of the file handler in the file handler list.  */
  if (file_ptr == remote_notifier.first_fd_handler)
    remote_notifier.first_fd_handler = file_ptr->next_fd;
  else {
    for (prev_ptr = remote_notifier.first_fd_handler;
         prev_ptr->next_fd != file_ptr; prev_ptr = prev_ptr->next_fd)
      ;
    prev_ptr->next_fd = file_ptr->next_fd;
  }

  free(file_ptr);
}

static int handle_fd_event(int event_fd_desc) {
    fd_handler *fd_ptr;
    int mask;

    /* Search the fd handler list to find one that matches the fd in
       the event.  */
    for (fd_ptr = remote_notifier.first_fd_handler; fd_ptr != NULL;
            fd_ptr = fd_ptr->next_fd) {
        if (fd_ptr->fd == event_fd_desc) {
            /* See if the desired events (mask) match the received
               events (ready_mask).  */

            if (fd_ptr->ready_mask & remote_EXCEPTION) {
                fd_ptr->error = 1;
            } else
                fd_ptr->error = 0;
            mask = fd_ptr->ready_mask & fd_ptr->mask;

            /* Clear the received events for next time around.  */
            fd_ptr->ready_mask = 0;

            /* If there was a match, then call the handler.  */
            if (mask != 0) {
                if ((*fd_ptr->proc)(fd_ptr->error, fd_ptr->client_data) < 0)
                    return -1;
            }
            break;
        }
    }
    return 0;
}

static remote_event *create_fd_event(int fd)
{
    remote_event *fd_event_ptr;

    fd_event_ptr = (remote_event_t)malloc(sizeof(remote_event));
    fd_event_ptr->proc = handle_fd_event;
    fd_event_ptr->fd = fd;
    return fd_event_ptr;
}

static void remote_event_free(struct remote_event *event)
{ 
    free(event); 
}

void event_initialize()
{
    event_queue = QUEUE_alloc(remote_event_t, remote_event_free);
}

static int handle_event(void)
{
    if (!QUEUE_is_empty(remote_event_t, event_queue)) {
        remote_event *event_ptr = QUEUE_deque(remote_event_t, event_queue);
        event_handler_func *proc = event_ptr->proc;
        int fd = event_ptr->fd;

        remote_event_free(event_ptr);

        int ret = (*proc)(fd);

        if (ret)
            // failed
            return -1;

        // ok, need to handle it.
        return 1;
    }

    // not event need to callback handle.
    return 0;
}

static int wait_for_event(void)
{
    fd_handler *fd_ptr;

    remote_notifier.ready_masks[0] = remote_notifier.check_masks[0];
    remote_notifier.ready_masks[1] = remote_notifier.check_masks[1];
    remote_notifier.ready_masks[2] = remote_notifier.check_masks[2];

    //readfs, writefs, errorfs
    int num_found = select(remote_notifier.num_fds, &remote_notifier.ready_masks[0],
            &remote_notifier.ready_masks[1],
            &remote_notifier.ready_masks[2], NULL);

    if (num_found == -1) {
        FD_ZERO(&remote_notifier.ready_masks[0]);
        FD_ZERO(&remote_notifier.ready_masks[1]);
        FD_ZERO(&remote_notifier.ready_masks[2]);
    }

    for (fd_ptr = remote_notifier.first_fd_handler;
            fd_ptr != NULL && num_found > 0; fd_ptr = fd_ptr->next_fd) {
        int mask = 0;

        if (FD_ISSET(fd_ptr->fd, &remote_notifier.ready_masks[0]))
            mask |= remote_READABLE;
        if (FD_ISSET(fd_ptr->fd, &remote_notifier.ready_masks[1]))
            mask |= remote_WRITABLE;
        if (FD_ISSET(fd_ptr->fd, &remote_notifier.ready_masks[2]))
            mask |= remote_EXCEPTION;

        if (mask == 0)
            continue;
        else
            num_found--;

        if (fd_ptr->ready_mask == 0) {
            remote_event *fd_event_ptr = create_fd_event(fd_ptr->fd);
            QUEUE_enque(remote_event_t, event_queue, fd_event_ptr);
        }

        fd_ptr->ready_mask = mask;
    }
    return 0;
}

void start_event_loop(void)
{
    while (1) {
        int ret = handle_event();
        if (ret == -1)
            return;
        if (ret)
            continue;

        if (wait_for_event() < 0)
            return;
    }
}
