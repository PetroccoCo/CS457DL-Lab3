CC = gcc
OBJS = 
FLAGS = 
DEBUGFLAG = -DDEBUG
SEQFLAGS = -O3 -lm


EXEC = proj1_client proj1_server

all: $(EXEC)

proj1_client: proj1_client.c $(OBJS)
	$(CC) $(SEQFLAGS) $(FLAGS)  -o $@ $< $(OBJS)

proj1_server: proj1_server.c $(OBJS)
	$(CC) $(SEQFLAGS)  $(FLAGS)  -o $@ $< $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)
