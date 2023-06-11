#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char *in;
    char *out;
} TrapData;

void trap(TrapData *data)
{
    sleep(2);
    sprintf(data->out, "out(%s)", data->in);
}

typedef struct {
    int incnt;
    int outcnt;
    char end;
} TrapState;

void loop(TrapState state)
{
    
}


/*
    111 2   4 55 111 2   4 55 111 2   4 55 
          3            3            3   


    111 111 111 1111 2 2 2 2 111 111 111 111 2 2 2 2 4 4 4 4 55 55 55 55 111 111 111 2 2 2 2 
                             3 3 3 3                 3 3 3 3                         3 3 3 3
*/

int main()
{
    return 0;
}