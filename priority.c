#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct execution{
    int priority;
    char name[20];
    char petition[20];
} execution;

execution scheduler[100];
int N = 0;

int priority_handler(){
    int i, head = 0;
    for(i = 0; i < N; i++){
        if(scheduler[i].priority < scheduler[head].priority){
            head = i;
        }
    }
    return head;
}

void scheduler_cleaner(int e){
    int i;
    for(i = e; i < N; i++){
        scheduler[i] = scheduler[i + 1];
    }
    N--;
}

void add(char * name, char * petition, int p){
    strcpy(scheduler[N].name, name);
    strcpy(scheduler[N].petition, petition);
    scheduler[N].priority = p;
    N++;
}

int main(){
    int top;
    char * priority;
    priority = "5";
    printf("%d\n", atoi(priority));
    add("cont1", "create", atoi(priority));
    add("cont2", "create", 2);
    add("cont3", "create", 7);
    add("cont4", "create", 4);

    top = priority_handler();
    printf("Name: %s\nPetition: %s\nPriority: %d\n", scheduler[top].name, scheduler[top].petition, scheduler[top].priority);
    scheduler_cleaner(top);
    top = priority_handler();
    printf("Name: %s\nPetition: %s\nPriority: %d\n", scheduler[top].name, scheduler[top].petition, scheduler[top].priority);
    scheduler_cleaner(top);
    add("cont9", "create", 1);
    top = priority_handler();
    printf("Name: %s\nPetition: %s\nPriority: %d\n", scheduler[top].name, scheduler[top].petition, scheduler[top].priority);
    
}