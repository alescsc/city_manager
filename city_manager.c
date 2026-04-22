#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

int main(int argc, char *argv[])
{
    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    int arg_index;

    for(int i = 0; i < argc; i++)
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
    }
    return 0;
}
