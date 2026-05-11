#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int running = 1;

void handle_sigint(int sig)
{
    char *msg = "\n[STOP]: Am primit SIGINT! Se inchide programul!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    running = 0;
}

void handle_sigusr1(int sig)
{
    char *msg = "\n[REPORT]: Un nou raport a fost adaugat!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main()
{
    struct sigaction sigint_action, sigusr1_action; // structuri de actiune

    //sigint
    sigint_action.sa_handler = handle_sigint;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;

    if(sigaction(SIGINT, &sigint_action, NULL) == -1)
    {
        fprintf(stderr, "[EROARE]: Configurare SIGINT!\n");
        exit(-1);
    }

    //sigusr1
    sigusr1_action.sa_handler = handle_sigusr1;
    sigemptyset(&sigusr1_action.sa_mask);
    sigusr1_action.sa_flags = 0;

    if(sigaction(SIGUSR1, &sigusr1_action, NULL) == -1)
    {
        fprintf(stderr, "[EROARE]: Configurare SIGUSR1!\n");
        exit(-1);
    }

    int my_pid = getpid();

    int fd_check = open(".monitor_pid", O_RDONLY);
    if(fd_check >= 0)
    {
        char buffer[32];
        int bytes = read(fd_check, buffer, sizeof(buffer) - 1);
        if(bytes > 0)
        {
            buffer[bytes] = '\0';
            printf("[EROARE]: Monitor deja pornit cu PID: %s", buffer);
            fflush(stdout);
        }
        close(fd_check);
        exit(1);
    }

    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0)
    {
        fprintf(stderr, "[EROARE]: Creare fisier .monitor_pid!\n");
        exit(-1);
    }

    char pid_str[32]; // transformam pidul in string si il scriem in fisier
    snprintf(pid_str, sizeof(pid_str), "%d\n", my_pid);
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    printf("[START]: monitor_reports pornit, PID: %d\n", my_pid);
    fflush(stdout);

    while(running)
        sleep(1); // asteptam semnalul

    if(unlink(".monitor_pid") == -1)
        fprintf(stderr, "[EROARE]: unlink fisier .monitor_pid!\n");
    else
    {
        printf("[STOP]: Fisierul .monitor_pid a fost sters!\n");
        fflush(stdout);
    }

    return 0;
}
