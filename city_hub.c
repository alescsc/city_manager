#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

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
                printf("[HUB_MONITOR] a interceptat: %s", buffer);
                printf("city_hub> ");
                fflush(stdout);
            }
            close(pipefd[0]);
            printf("[HUB_MONITOR] Monitor inchis! Procesul hub_monitor se termina.\n");
            printf("city_hub> ");
            fflush(stdout);

            waitpid(monitor_pid, NULL, 0);
            exit(0);
        }
        else
            exit(1);
    }
}

void exec_calculate_scores(char *dist)
{
    int pfd[2];
    if(pipe(pfd) == -1)
        return;

    pid_t pid = fork();
    if(pid == 0)
    {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        execlp("./scorer", "scorer", dist, NULL);
        exit(1);
    }
    else if(pid > 0)
    {
        close(pfd[1]);
        char buf[512];
        int n;
        while((n = read(pfd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            printf("%s", buf);
        }
        close(pfd[0]);
        waitpid(pid, NULL, 0);
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
        {
            FILE *f_pid = fopen(".monitor_pid", "r");
            if(f_pid != NULL)
            {
                int m_pid;
                if(fscanf(f_pid, "%d", &m_pid) == 1)
                    kill(m_pid, SIGINT);
                fclose(f_pid);
            }
            usleep(10000);
            break;
        }

        else if(strcmp(command, "start_monitor") == 0)
            exec_start_monitor();

        else if(strncmp(command, "calculate_scores", 16) == 0)
        {
            char *token = strtok(command, " ");
            token = strtok(NULL, " ");
            if(token == NULL)
                printf("[EROARE] Trebuie sa specifici un district! \n");
            else
                while(token != NULL)
                {
                    exec_calculate_scores(token);
                    token = strtok(NULL, " ");
                }
        }

        else
            printf("Comanda necunoscuta!\n");
    }
    return 0;
}