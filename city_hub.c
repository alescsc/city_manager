#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 256

void exec_start_monitor()
{
    pid_t hub_monitor_pid = fork(); // facem un proces copil pentru a rula in fundal
    if (hub_monitor_pid == 0)
    {
        int pipefd[2];
        if(pipe(pipefd) == -1)
            exit(1);

        pid_t monitor_pid = fork(); // proces copil pentru monitor_reports
        if (monitor_pid == 0)
        {
            close(pipefd[0]); // inchidem capatul de pipe pe care nu il folosim
            dup2(pipefd[1], STDOUT_FILENO); // redirectam iesirea standard
            close(pipefd[1]);
            execlp("./monitor_reports", "monitor_reports", NULL);
            exit(1);
        }
        else if(monitor_pid > 0)
        {
            close(pipefd[1]);
            char buffer[1024];
            int bytes_read;
            while((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes_read] = '\0';
                printf("\n[HUB_MONITOR] a interceptat: %s", buffer);
                printf("city_hub> ");
                fflush(stdout);
            }
            close(pipefd[0]);
            printf("\n[HUB_MONITOR] Monitor inchis! Procesul hub_monitor se termina.\n");
            printf("city_hub> ");
            fflush(stdout);

            waitpid(monitor_pid, NULL, 0);
            exit(0);
        }
        else
            exit(1);
    }


}

int main()
{
    char command[MAX_CMD_LEN];
    printf("> Bun venit in City Hub! <\n");

    while(1)
    {
        printf("city_hub> ");
        fflush(stdout);
        if(fgets(command, sizeof(command), stdin) == NULL) // verificam daca utilizatorul a transmis Ctrl + D
            break;

        command[strcspn(command, "\n")] = 0;
        if(strlen(command) == 0) // verificam daca utilizatorul a apasat doar enter
            continue;

        if(strcmp(command, "exit") == 0)
            break;

        else if(strcmp(command, "start_monitor") == 0)
            exec_start_monitor();

        else
            printf("Comanda necunoscuta!\n");
    }
    return 0;
}