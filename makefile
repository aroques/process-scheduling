CC = gcc
IDIR =./include
CFLAGS = -I$(IDIR) -g -Wall

EXEC1 = oss
OBJS1 = oss.o

EXEC2 = user
OBJS2 = user.o

SHARE = helpers.o message_queue.o shared_memory.o queue.o

DEPS = global_constants.h helpers.h message_queue.h shared_memory.h global_structs.h queue.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: oss user

oss: $(OBJS1) $(SHARE)
	gcc -o $(EXEC1) $^ $(CFLAGS)
	
user: $(OBJS2) $(SHARE)
	gcc -o $(EXEC2) $^ $(CFLAGS)

clean:
	rm $(EXEC1) $(OBJS1) $(EXEC2) $(OBJS2) $(SHARE)
