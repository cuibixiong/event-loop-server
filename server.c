#include "server.h"
#include "remote.h"

#include <setjmp.h>
#include <unistd.h>
#include <signal.h>

int
main (int argc, char *argv[])
{
	char *port;
	char **next_arg = &argv[1];

	port = *next_arg;

	if (port == NULL) {
		exit (1);
	}

	remote_prepare (port);

	//initialize_async_io ();
	event_initialize();

	while (1)
	{
		remote_open (port);
		start_event_loop ();
	}
}

static int
process_serial_event (void)
{
	return 0;
}

int
handle_serial_event (int err, void *client_data)
{
  /* Really handle it.  */
  if (process_serial_event () < 0)
    return -1;

  return 0;
}

