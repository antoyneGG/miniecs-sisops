CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: admin-container ecs-agent1 ecs-agent2 ecs-agent3 agent-scheduler1 agent-scheduler2 client

admin-container: admin-container.o
	gcc -pthread -o admin-container admin-container.o -lrt

ecs-agent1: ecs-agent1.o
	gcc -pthread -o ecs-agent1 ecs-agent1.o

ecs-agent2: ecs-agent2.o
	gcc -pthread -o ecs-agent2 ecs-agent2.o

ecs-agent3: ecs-agent3.o
	gcc -pthread -o ecs-agent3 ecs-agent3.o

agent-scheduler1: agentScheduler1.o
	gcc -pthread -o agentScheduler1 agentScheduler1.o

agent-scheduler2: agentScheduler2.o
	gcc -pthread -o agentScheduler2 agentScheduler2.o

client: client.o
	g++ -pthread -o client client.cpp
	
clean:
	rm -f *.o
