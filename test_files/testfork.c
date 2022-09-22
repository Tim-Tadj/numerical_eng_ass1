#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include<sys/wait.h>
#include <time.h>

#define NUM_PROCESSES 7

int main()
{
    

pid_t child_pid, wpid;
int status = 0;

//Father code (before child processes start)
int n = NUM_PROCESSES;

for (int id=0; id<n; id++) {
    if ((child_pid = fork()) == 0) {
        //get prc id
        int prc_id = getpid();
        //sleep 2 seconds
        sleep(0.5);
        printf("Child process %d\n", prc_id);
        exit(0);
    }
}

while ((wpid = wait(&status)) > 0);
int prc_id = getpid();
printf("Father process %d\n", prc_id);

}