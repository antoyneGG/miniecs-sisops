CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: admin-container ecs-agent1 ecs-agent2 exec

admin-container: admin-container.o
	gcc -o admin-container admin-container.o

ecs-agent1: ecs-agent1.o
	gcc -o ecs-agent1 ecs-agent1.o

ecs-agent2: ecs-agent2.o
	gcc -o ecs-agent2 ecs-agent2.o

exec: exec.o
	gcc -o exec exec.o
	
clean:
	rm -f *.o
