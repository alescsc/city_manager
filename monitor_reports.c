#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int running = 1;

void handle_sigint(int sig)
{
    char *msg = "\nMONITOR: Am primit SIGINT! Se inchide programul!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    running = 0;
}

void handle_sigusr1(int sig)
{
    char *msg = "\nMONITOR: Un nou raport a fost adaugat!\n";
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
        fprintf(stderr, "Eroare: Configurare SIGINT!\n");
        exit(-1);
    }

    //sigusr1
    sigusr1_action.sa_handler = handle_sigusr1;
    sigemptyset(&sigusr1_action.sa_mask);
    sigusr1_action.sa_flags = 0;

    if(sigaction(SIGUSR1, &sigusr1_action, NULL) == -1)
    {
        fprintf(stderr, "Eroare: Configurare SIGUSR1!\n");
        exit(-1);
    }

    int my_pid = getpid();

    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare: Creare fisier .monitor_pid!\n");
        exit(-1);
    }

    char pid_str[32]; // transformam pidul in string si il scriem in fisier
    snprintf(pid_str, sizeof(pid_str), "%d\n", my_pid);
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    printf("Info: monitor_reports pornit, PID: %d\n", my_pid);

    while(running)
        sleep(1); // asteptam semnalul

    if(unlink(".monitor_pid") == -1)
        fprintf(stderr, "Eroare: unlink fisier .monitor_pid!\n");
    else
        printf("Info: Fisierul .monitor_pid a fost sters!\n");

    return 0;
}
