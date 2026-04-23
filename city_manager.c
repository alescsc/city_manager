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

void get_permissions_string(mode_t mode, char *str)
{
    strcpy(str, "---------");
    // permisiuni owner
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';

    // permisiuni grup
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';

    // others
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';

    str[9] = '\0';
}

int check_permission(mode_t mode, char *role, char access_type) // functie pt a verifica accesul de scriere/citire
{
    if(strcmp(role, "manager") == 0)
    {
        if(access_type == 'R')
        {
            if(mode & S_IRUSR)
                return 1;
            else
                return 0;
        }
        else if(access_type == 'W')
        {
            if(mode & S_IWUSR)
                return 1;
            else
                return 0;
        }
        else if(access_type == 'X')
        {
            if(mode & S_IXUSR)
                return 1;
            else
                return 0;
        }
    }
    else if(strcmp(role, "inspector") == 0)
    {
        if(access_type == 'R')
        {
            if(mode & S_IRGRP)
                return 1;
            else
                return 0;
        }
        else if(access_type == 'W')
        {
            if(mode & S_IWGRP)
                return 1;
            else
                return 0;
        }
        else if(access_type == 'X')
        {
            if(mode & S_IXGRP)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

void command_add(char *district_id, char *role, char *user)
{
    struct stat st = {0};
    if(stat(district_id, &st) == -1) // verificam daca exista directorul
    {
        if(strcmp(role, "manager") != 0)
        {
            fprintf(stderr, "Eroare: Cartierul %s nu exista! Doar un manager poate crea fisierele .cfg si .log!!!!", district_id);
            exit(-1);
        }
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

        char cfg_path[512]; // implementam si district.cfg
        snprintf(cfg_path, 512, "%s/district.cfg", district_id);

        int fd_cfg = open(cfg_path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(fd_cfg >= 0)
        {
            char *default_threshold = "2\n";
            write(fd_cfg, default_threshold, strlen(default_threshold));
            close(fd_cfg);
            chmod(cfg_path, 0640);
            printf("Fisierul district.cfg a fost creat!\n");
        }
    }
    else
    {
        printf("Directorul %s exista deja!\n", district_id);
    }

    Raport r;
    strcpy(r.inspectorName, user);
    r.timestamp = time(NULL);

    r.latitude = (float)(rand() % 90);
    r.longitude = (float)(rand() % 180);

    char *categories[] = {"road", "lighting", "flooding", "other"};
    strcpy(r.category, categories[rand() % 4]);

    r.severity = (rand() % 3) + 1;

    printf("Description: ");
    fgets(r.description, description_len, stdin);
    r.description[strlen(r.description) - 1] = '\0';

    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // cale spre fisierul reports.dat

    struct stat file_st = {0};
    if(stat(path, &file_st) == 0)
    {
        if(check_permission(file_st.st_mode, role, 'W') == 0)
        {
            fprintf(stderr, "Rolul %s nu are permisiunea de a scrie in reports.dat!\n", role);
            exit(-1);
        }
        r.ID = (file_st.st_size / sizeof(Raport)) + 1; // daca exista calculam ID automat
    }
    else
        r.ID = 1; // daca nu, il punem pe 1

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

    // creere si logged_district
    char log_path[512];
    snprintf(log_path, 512, "%s/logged_district", district_id);

    struct stat log_st = {0};
    if(stat(log_path, &log_st) == 0) // verificam daca deja exiata logul
    {
        if(check_permission(log_st.st_mode, role, 'W') == 0) // verificam permisiunea de scriere
        {
            fprintf(stderr, "Rolul %s nu are permisiune de a scrie in fisierul logged_district!", role);
            return;
        }
    }
    else
    {    //daca nu exista fisierul log, doar un manager il poate crea
        if(strcmp(role, "manager") != 0)
        {
            fprintf(stderr, "Rolul %s nu are permisiune de a scrie in fisierul logged_district!", role);
            return;
        }
    }
    int fd_log = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_log >= 0)
    {
        char log_entry[256];
        snprintf(log_entry, sizeof(log_entry), "%ld\n%s\n%s add\n", (long)time(NULL), user, role);
        write(fd_log, log_entry, strlen(log_entry));
        close(fd_log);
        chmod(log_path, 0644);
    }
}

void command_list(char *district_id, char *role)
{
    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // cale spre fisierul reports.dat

    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare la deschiderea reports.dat!");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'R') == 0)
    {
        fprintf(stderr, "Rolul %s nu are dreptul de a citi rapoartele (--list)!\n", role);
        exit(-1);
    }
    char perm_str[11];
    get_permissions_string(st.st_mode, perm_str);

    printf("\n");
    printf("> File information for: %s < \n", path);
    printf("Permissions: %s | Size: %ld bytes | Last modified: %s", perm_str, (long)st.st_size, ctime(&st.st_mtime));
    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare la deschiderea fisierului!");
        exit(-1);
    }
    Raport r;
    while(read(fd, &r, sizeof(Raport)) == sizeof(Raport))
    {
        printf("ID: %d | Inspector: %s | Cat: %s | Sev: %d\n", r.ID, r.inspectorName, r.category, r.severity);
        printf("Coord: (%.2f, %.2f) | Time: %s", r.latitude, r.longitude, ctime(&r.timestamp));
        printf("Description: %s\n", r.description);
        printf("\n");
    }

    close(fd);
}

void command_view(char *district_id, int report_id, char *role)
{
    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // creem path
    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare la deschiderea reports.dat!");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'R') == 0) // verificam daca are permisiune
    {
        fprintf(stderr, "Rolul %s nu are dreptul de a citi rapoarte! (--view)!\n", role);
        exit(-1);
    }

    if(report_id <= 0 || (report_id * sizeof(Raport)) > st.st_size) // verificam daca raportul exista
    {
        fprintf(stderr, "Eroare, raportul cu ID %d nu exista in districtul %s\n", report_id, district_id);
        exit(-1);
    }
    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare la deschiderea reports.dat!");
        exit(-1);
    }

    off_t offset = (report_id - 1) * sizeof(Raport); // calculam offest in fisier
    if(lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        fprintf(stderr, "Eroare la lseek!\n");
        close(fd);
        exit(-1);
    }

    Raport r;
    if(read(fd, &r, sizeof(Raport)) == sizeof(Raport))
    {
        printf(" > DETALII PENTRU RAPORTUL: %d <\n", r.ID);
        printf("Inspector: %s\n", r.inspectorName);
        printf("Cat: %s\n", r.category);
        printf("Sev: %d\n", r.severity);
        printf("Coordonate: (%.2f, %.2f)\n", r.latitude, r.longitude);
        printf("Timestamp: %s\n", ctime(&r.timestamp));
        printf("Description: %s\n", r.description);
        printf("\n\n");
    }
    else
        fprintf(stderr, "Eroare la citirea raportului!\n");
    close(fd);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
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
        if(arg_index + 1 >= argc)
        {
            fprintf(stderr, "Lipseste district ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        command_add(ID, role, user);
    }
    else if(strcmp(command, "--list") == 0)
    {
        if(arg_index + 1 >= argc)
        {
            fprintf(stderr, "Lipseste district ID!");
            exit(-1);
        }
        command_list(argv[arg_index + 1], role);
    }
    else if(strcmp(command, "--view") == 0)
    {
        if(arg_index + 2 >= argc)
        {
            fprintf(stderr, "Lipseste district ID si report ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        int report_id = atoi(argv[arg_index + 2]);
        command_view(ID, report_id, role);
    }
    return 0;
}
