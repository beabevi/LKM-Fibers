SRC = 	src/calqueue.c \
		src/channel.c \
		src/jmp.S \
		src/main.c \
		src/model.c \
		src/numerical.c \
		src/thread.c \
		src/ult.c \
		../../util/log.c \
		../../lib/fibers.c \

# -fno-omit-frame-pointer allows for better profiling through `perf`
CFLAGS = -O3 -g -fno-omit-frame-pointer
EXEC = test
OBJS := $(SRC:%.c=%.o)
OBJS := $(OBJS:%.S=%.o)
LIBS = -lm -lpthread

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(EXEC)

%.o : %.S
	$(CC) $< $(LIBS) -c -o $@

.PHONY: clean
clean:
	-rm $(OBJS)
	-rm $(EXEC)
