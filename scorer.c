#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define name_len 50
#define category_len 25
#define description_len 256
#define MAX_NR 100
#define MAX_DIST 128

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

typedef struct
{
    char nume[name_len];
    int scor_total;
} Scor;

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "[EROARE] Lipsteste districtul!\n");
        exit(-1);
    }

    char dist[MAX_DIST];
    strncpy(dist, argv[1], MAX_DIST - 1);

    //construim calea
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", dist);

    FILE *f = fopen(path, "rb");
    if(f == NULL)
    {
        printf("[INFO] Nu exista rapoarte in districtul %s!\n", dist);
        return 0;
    }

    Scor array_scor[MAX_NR];
    int nr_inspectori = 0;
    Raport rep;

    while(fread(&rep, sizeof(Raport), 1, f) == 1)
    {
        int gasit = 0;
        for(int i = 0; i < nr_inspectori; i++)
            if(strcmp(array_scor[i].nume, rep.inspectorName) == 0)
            {
                array_scor[i].scor_total = array_scor[i].scor_total + rep.severity;
                gasit = 1;
                break;
            }
        if(!gasit && nr_inspectori < MAX_NR)
        {
            strncpy(array_scor[nr_inspectori].nume, rep.inspectorName, name_len - 1);
            array_scor[nr_inspectori].scor_total = rep.severity;
            nr_inspectori++;
        }
    }

    fclose(f);
    printf("-- Scoruri pentru district: %s --\n", dist);
    for(int i = 0; i < nr_inspectori; i++)
        printf("Inspector %s: Scor %d\n", array_scor[i].nume, array_scor[i].scor_total);
    return 0;
}