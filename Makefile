all: event-loop.exe

SRCS += $(wildcard *.c)
OBJS += $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -g -O0

event-loop.exe: $(OBJS)
	gcc $(OBJS) -o $@

clean:
	rm -rf $(OBJS)
