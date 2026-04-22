
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define name_len 50
#define category_len 25
#define description_len 256

typedef struct
{
    int ID;
    char inspectorName[name_len];
    float latitude, longitude;
    char category[category_len];
    int severity;
    time_t timestamp;
    char description[description_len];
} Raport;

void command_add(char *district_id, char *role, char *user)
{
    struct stat st = {0};
    if(stat(district_id, &st) == -1) // verificam daca exista directorul
    {
        if(mkdir(district_id, 0750) == -1) // creez directorul
        {
            fprintf(stderr, "Eroare la crearea directorului!");
            exit(-1);
        }
        if(chmod(district_id, 0750) == -1) // setez permisiunile
        {
            fprintf(stderr, "Eroare la chmod directorului!");
            exit(-1);
        }
        printf("Directorul %s a fost creat cu succes!\n", district_id);
    }
    else
    {
        printf("Directorul %s exista deja!\n", district_id);
    }

    Raport r;
    r.ID = 1;
    strcpy(r.inspectorName, user);
    r.timestamp = time(NULL);

    printf("X (Latitudine): ");
    scanf("%f", &r.latitude);

    printf("Y (Longitudine): ");
    scanf("%f", &r.longitude);

    printf("Category (road/lighting/flooding/other): ");
    scanf("%s", r.category);

    printf("Severity level (1-minor, 2-moderate, 3-critical): ");
    scanf("%d", &r.severity);

    printf("Description: "); getchar();
    fgets(r.description, description_len, stdin);
    r.description[strlen(r.description) - 1] = '\0';

    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // cale spre fisierul reports.dat

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare la deschiderea reports.dat!");
        exit(-1);
    }
    chmod(path, 0664);
    if(write(fd, &r, sizeof(Raport)) != sizeof(Raport))
        fprintf(stderr, "Eroare la scrierea in fisier!");
    else
        printf("Raportul a fost salvat cu succes in %s.\n", path);

    close(fd);
}

int main(int argc, char *argv[])
{
    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    int arg_index;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--role") == 0)
        {
            role = argv[i + 1];
            i++;
        }
        else
            if(strcmp(argv[i], "--user") == 0)
            {
                user = argv[i + 1];
                i++;
            }
            else
                if(command == NULL)
                {
                    command = argv[i];
                    arg_index = i;
                }
    }
    if(role == NULL)
    {
        fprintf(stderr, "Nu s-a specificat rolul!");
        exit(-1);
    }

    if(user == NULL)
    {
        user = "Guest";
    }

    if(command == NULL)
    {
        fprintf(stderr, "Nu s-a dat o comanda!");
        exit(-1);
    }

    if(strcmp(command, "--add") == 0)
    {
        if(arg_index + 1 > argc)
        {
            fprintf(stderr, "Lipseste district ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        command_add(ID, role, user);
    }
    return 0;
}
