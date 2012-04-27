CC = gcc
OBJS = 
FLAGS = -Wall
DEBUGFLAG = -g -DDEBUG
SEQFLAGS = -O3 -lm


EXEC = myresolver 

all: $(EXEC)

proj1_client: myresolver.c $(OBJS)
	$(CC) $(SEQFLAGS) $(FLAGS)  -o $@ $< $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)
