
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include "remote.h"
#include "event-loop.h"

#define INVALID_DESCRIPTOR -1

remote_fildes_t remote_desc = INVALID_DESCRIPTOR;
remote_fildes_t listen_desc = INVALID_DESCRIPTOR;

static int socket_read(void *buf, int count);
static int socket_write(const void *buf, int count);

static int handle_accept_event(remote_client_data client_data);

void remote_prepare(char *name)
{
    char *port_str;
    int port;

    struct sockaddr_in sockaddr;
    socklen_t tmp;
    char *port_end;

    port_str = strchr(name, ':');
    if (port_str == NULL) {
        return;
    }

    port = strtoul(port_str + 1, &port_end, 10);
    if (port_str[1] == '\0' || *port_end != '\0')
        printf("Error port argument: %s", name);

    listen_desc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_desc == -1) perror("Can't open socket");

    tmp = 1;
    setsockopt(listen_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp));

    sockaddr.sin_family = PF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_desc, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) ||
            listen(listen_desc, 1))
        perror("Can't bind address");
}

void remote_open(char *name)
{
    char *port_str;
    port_str = strchr(name, ":");

    int port;
    socklen_t len;
    struct sockaddr_in sockaddr;

    len = sizeof(sockaddr);

    if (getsockname(listen_desc, (struct sockaddr *)&sockaddr, &len) < 0 || len < sizeof(sockaddr))
        perror("Can't detect port");

    port = ntohs(sockaddr.sin_port);

    fprintf(stderr, "Listening port %d\n", port);
    fflush(stderr);

    add_fd_handler(listen_desc, handle_accept_event, NULL);
}

void remote_close(void)
{
    delete_fd_handler(remote_desc);

    close(remote_desc);

    remote_desc = INVALID_DESCRIPTOR;
}

void initialize_async_io(void);
void enable_async_io(void);
void disable_async_io(void);


