CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: admin-container ecs-agent1 ecs-agent2 ecs-agent3

admin-container: admin-container.o
	gcc -pthread -o admin-container admin-container.o -lrt

ecs-agent1: ecs-agent1.o
	gcc -pthread -o ecs-agent1 ecs-agent1.o

ecs-agent2: ecs-agent2.o
	gcc -pthread -o ecs-agent2 ecs-agent2.o

ecs-agent3: ecs-agent3.o
	gcc -pthread -o ecs-agent3 ecs-agent3.o
	
clean:
	rm -f *.o
