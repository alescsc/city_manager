#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

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
            if(mode & S_IRUSR) return 1;
            else return 0;
        }
        else if(access_type == 'W')
        {
            if(mode & S_IWUSR) return 1;
            else return 0;
        }
        else if(access_type == 'X')
        {
            if(mode & S_IXUSR) return 1;
            else return 0;
        }
    }
    else if(strcmp(role, "inspector") == 0)
    {
        if(access_type == 'R')
        {
            if(mode & S_IRGRP) return 1;
            else return 0;
        }
        else if(access_type == 'W')
        {
            if(mode & S_IWGRP) return 1;
            else return 0;
        }
        else if(access_type == 'X')
        {
            if(mode & S_IXGRP) return 1;
            else return 0;
        }
    }
    return 0;
}

void log_event(char *district_id, char *user, char *role, char *command)
{
    char log_path[512];
    snprintf(log_path, 512, "%s/logged_district", district_id);

    struct stat log_st = {0};
    if(stat(log_path, &log_st) == 0)
    {
        if(check_permission(log_st.st_mode, role, 'W') == 0)
        {
            fprintf(stderr, "Eroare: Rolul %s nu are drept de a scrie in log!\n", role);
            return;
        }
    }

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if(fd >= 0)
    {
        char log_entry[512];
        snprintf(log_entry, sizeof(log_entry), "%ld\n%s\n%s\n%s\n", (long)time(NULL), user, role, command);
        write(fd, log_entry, strlen(log_entry));
        close(fd);
        chmod(log_path, 0664);
    }
}

void validate_symlink(char *district_id) // verificam daca symlink e corect
{
    struct stat link_info;
    char link_path[512];
    snprintf(link_path, sizeof(link_path), "active_reports-%s", district_id); // aflam pathul
    if(lstat(link_path, &link_info) == 0)
    {
        if(S_ISLNK(link_info.st_mode)) // vedem daca e link
        {
            struct stat target_info;
            if(stat(link_path, &target_info) == -1)
            {
                fprintf(stderr, "WARNING: Linkul simbolic %s e dangling!\n", link_path);
                unlink(link_path);
            }
            else
                printf("Info: Linkul simbolic %s e valid!\n", link_path);
        }
    }
}

void command_add(char *district_id, char *role, char *user)
{
    struct stat st = {0};
    if(stat(district_id, &st) == -1) // verificam daca exista directorul
    {
        if(mkdir(district_id, 0750) == -1) // creez directorul
        {
            fprintf(stderr, "Eroare: Crearea directorului!");
            exit(-1);
        }
        if(chmod(district_id, 0750) == -1) // setez permisiunile
        {
            fprintf(stderr, "Eroare: chmod pe director!");
            exit(-1);
        }
        printf("Info: Directorul %s a fost creat cu succes!\n", district_id);

        char cfg_path[512]; // implementam si district.cfg
        snprintf(cfg_path, 512, "%s/district.cfg", district_id);

        int fd_cfg = open(cfg_path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(fd_cfg >= 0)
        {
            char *default_threshold = "2\n";
            write(fd_cfg, default_threshold, strlen(default_threshold));
            close(fd_cfg);
            chmod(cfg_path, 0640);
            printf("Info: Fisierul district.cfg a fost creat!\n");
        }
    }
    else
    {
        printf("Info: Directorul %s exista deja!\n", district_id);
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
            fprintf(stderr, "Eroare: Rolul %s nu are permisiunea de a scrie in reports.dat!\n", role);
            exit(-1);
        }
        r.ID = (file_st.st_size / sizeof(Raport)) + 1; // daca exista calculam ID automat
    }
    else
        r.ID = 1; // daca nu, il punem pe 1

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }
    chmod(path, 0664);
    if(write(fd, &r, sizeof(Raport)) != sizeof(Raport))
        fprintf(stderr, "Eroare: Scriere in fisier!");
    else
        printf("Info: Raportul a fost salvat cu succes in %s.\n", path);

    close(fd);

    // link simbolic
    char symlink_name[512];
    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district_id);

    if(access(symlink_name, F_OK) == -1) // verificam daca exista fisierul indiferent de ce drepturi avem
    {
        if(symlink(path, symlink_name) == -1) // crearea efectiva a linkului
            fprintf(stderr, "Eroare: Crearea link-ului simbolic a esuat!\n");
        else
            printf("Info: Link-ul simbolic %s a fost creat!\n", symlink_name);
    }

    log_event(district_id, user, role, "add");
}

void command_list(char *district_id, char *role, char *user)
{
    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // cale spre fisierul reports.dat

    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'R') == 0)
    {
        fprintf(stderr, "Eroare: Rolul %s nu are dreptul de a citi rapoartele (--list)!\n", role);
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
        fprintf(stderr, "Eroare: Deschiderea fisierului!");
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
    log_event(district_id, user, role, "list");
}

void command_view(char *district_id, int report_id, char *role, char *user)
{
    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id); // creem path
    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'R') == 0) // verificam daca are permisiune
    {
        fprintf(stderr, "Eroare: Rolul %s nu are dreptul de a citi rapoarte! (--view)!\n", role);
        exit(-1);
    }

    if(report_id <= 0 || (report_id * sizeof(Raport)) > st.st_size) // verificam daca raportul exista
    {
        fprintf(stderr, "Eroare: Raportul cu ID %d nu exista in districtul %s\n", report_id, district_id);
        exit(-1);
    }
    int fd = open(path, O_RDONLY); // verificam daca s a deschis cum trebuie
    if(fd < 0)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }

    off_t offset = (report_id - 1) * sizeof(Raport); // calculam offest in fisier
    if(lseek(fd, offset, SEEK_SET) == (off_t)-1) // verificam daca s a pus cursorul bine
    {
        fprintf(stderr, "Eroare: lseek!\n");
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
        fprintf(stderr, "Eroare: Citirea raportului!\n");
    close(fd);
    log_event(district_id, user, role, "view");
}

void command_remove_report(char *district_id, int report_id, char *role, char *user)
{
    if(strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Eroare: Doar managerul poate sa stearga rapoarte!\n");
        exit(-1);
    }

    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id);
    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'W') == 0)
    {
        fprintf(stderr, "Eroare: Rolul %s nu are permisiune de a scrie in reports.dat!\n", role);
        exit(-1);
    }

    if(report_id <= 0 || (report_id * sizeof(Raport)) > st.st_size)
    {
        fprintf(stderr, "Eroare: Raportul cu ID %d nu exista!\n", report_id);
        exit(-1);
    }

    int fd = open(path, O_RDWR);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare: Deschiderea reports.dat!");
        exit(-1);
    }

    int total_reports = st.st_size / sizeof(Raport);
    Raport r;
    for(int i = report_id; i < total_reports; i++)
    {
        lseek(fd, i * sizeof(Raport), SEEK_SET); // trecem la urmatorul raport
        read(fd, &r, sizeof(Raport)); // citim datele din el si le salvam in r
        r.ID--; // ii scadem indexul
        lseek(fd, (i-1) * sizeof(Raport), SEEK_SET); // trecem la raportul pe care il stergem
        write(fd, &r, sizeof(Raport)); // suprascriem
    }
    off_t new_size = (total_reports - 1) * sizeof(Raport);
    if(ftruncate(fd, new_size) == -1)
    {
        fprintf(stderr, "Eroare: Trunchierea fisierului!\n");
        close(fd);
        exit(-1);
    }
    printf("Info: Raportul %d a fost sters cu succes din %s!\n", report_id, district_id);
    close(fd);

    log_event(district_id, user, role, "remove_report");
}

void command_update_threshold(char *district_id, int new_threshold, char *role, char *user)
{
    if(strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Eroare: Doar managerul poate sa actualizeze pragul de threshold!\n");
        exit(-1);
    }

    if(new_threshold < 1 || new_threshold > 3)
    {
        fprintf(stderr, "Eroare: Pragul de threshold trebuie sa fie intre 1 si 3!\n");
        exit(-1);
    }

    char cfg_path[512];
    snprintf(cfg_path, 512, "%s/district.cfg", district_id);

    struct stat st = {0};
    if(stat(cfg_path, &st) == -1)
    {
        fprintf(stderr, "Eroare: Fisierul district.cfg nu exista in districtul %s!\n", district_id);
        exit(-1);
    }

    if((st.st_mode & 0777) != 0640)
    {
        fprintf(stderr, "Eroare: Permisiunile fisierului district.cfg au fost modificate (nu mai sunt 640)!\n");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'W') == 0)
    {
        fprintf(stderr, "Eroare: Rolul %s nu are permisiune de a scrie in district.cfg!\n", role);
        exit(-1);
    }

    int fd_cfg = open(cfg_path, O_WRONLY | O_TRUNC);
    if(fd_cfg < 0)
    {
        fprintf(stderr, "Eroare: Deschiderea fiserului district.cfg a esuat!\n");
        exit(-1);
    }

    char threshold_str[10];
    snprintf(threshold_str, sizeof(threshold_str), "%d\n", new_threshold);
    if(write(fd_cfg, threshold_str, strlen(threshold_str)) == - 1)
        fprintf(stderr, "Eroare: Scrierea noului prag a esuat!\n");
    else
        printf("Info: Pragul pentru districtul %s a fost actualizat la %d!\n", district_id, new_threshold);

    close(fd_cfg);
    log_event(district_id, user, role, "update_threshold");
}

int parse_condition(char *input, char *field, char *op, char *value) // functia pentru citirea conditiei de la filter generata cu AI
{
    if(sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3)
        return 1;
    return 0;
}

int cmp_numeric(long long val_raport, long long val_filtru, char *op)
{
    if(strcmp(op, "==") == 0) return val_raport == val_filtru;
    if(strcmp(op, "!=") == 0) return val_raport != val_filtru;
    if(strcmp(op, "<") == 0) return val_raport < val_filtru;
    if(strcmp(op, "<=") == 0) return val_raport <= val_filtru;
    if(strcmp(op, ">") == 0) return val_raport > val_filtru;
    if(strcmp(op, ">=") == 0) return val_raport >= val_filtru;

    return 0;
}

int match_condition(Raport *r, char *field, char *op, char *value)
{
    if(strcmp(field, "severity") == 0)
        return cmp_numeric(r->severity, atol(value), op);
    else if(strcmp(field, "timestamp") == 0)
        return cmp_numeric(r->timestamp, atol(value), op);
    else if(strcmp(field, "category") == 0)
    {
        if(strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if(strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
    }
    else if(strcmp(field, "inspector") == 0)
    {
        if(strcmp(op, "==") == 0) return strcmp(r->inspectorName, value) == 0;
        if(strcmp(op, "!=") == 0) return strcmp(r->inspectorName, value) != 0;
    }

    return 0;
}

void command_filter(char *district_id, char *condition, char *role, char *user)
{
    char field[50], op[10], value[50];
    if(parse_condition(condition, field, op, value) == 0)
    {
        fprintf(stderr, "Eroare: Formatul conditiei este invalid! (--filter)\n");
        exit(-1);
    }

    char path[512];
    snprintf(path, 512, "%s/reports.dat", district_id);

    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        fprintf(stderr, "Eroare: Deschiderea raports.dat!\n");
        exit(-1);
    }

    if(check_permission(st.st_mode, role, 'R') == 0)
    {
        fprintf(stderr, "Eroare: Rolul %s nu are dreptul de a citi rapoarte! (--filter)\n", role);
        exit(-1);
    }

    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr, "Eroare: Deschiderea raports.dat!\n");
        exit(-1);
    }

    Raport r;
    int gasit = 0;
    printf("> Rezultate filtrate pentru conditia %s in districtul %s <\n", condition, district_id);
    while(read(fd, &r, sizeof(Raport)) == sizeof(Raport))
    {
        if(match_condition(&r, field, op, value) == 1)
        {
            gasit = 1;
            printf("ID: %d | Inspector: %s | Cat: %s | Sev: %d\n", r.ID, r.inspectorName, r.category, r.severity);
            printf("Coord: (%.2f, %.2f) | Time: %s", r.latitude, r.longitude, ctime(&r.timestamp));
            printf("Description: %s\n", r.description);
            printf("\n");
        }
    }
    if(gasit == 0)
           printf("Info: Nu exista niciun raport care indeplineste conditia specificata!\n");
    close(fd);
    log_event(district_id, user, role, "filter");
}

void command_remove_district(char *district_id, char *role, char *user)
{
    if(strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Eroare: Doar managerul poate sa stearga un district!\n");
        exit(-1);
    }

    log_event(district_id, user, role, "remove_district");

    char symlink_name[512];
    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district_id);

    pid_t pid = fork(); // creem proces nou
    if(pid == -1)
    {
        fprintf(stderr, "Eroare: Crearea procesului (fork) a esuat!\n");
        exit(-1);
    }
    else if(pid == 0)
    {
        unlink(symlink_name); // stergem legatura
        execlp("rm", "rm", "-rf", district_id, NULL); // stergem directorul
        fprintf(stderr, "Eroare: Eroare la rm!\n");
        exit(-1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0); // punem procesul parinte sa astepte
        if(WIFEXITED(status) && WEXITSTATUS(status) == 0) // verificam daca procesul copil s a terminat fara erori
            printf("Info: Districtul %s a fost sters cu succes!\n", district_id);
        else
            fprintf(stderr, "Eroare: Eroare la stergerea districtului!\n");
    }
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
        fprintf(stderr, "Eroare: Nu s-a specificat rolul!");
        exit(-1);
    }

    if(user == NULL)
    {
        user = "Guest";
    }

    if(command == NULL)
    {
        fprintf(stderr, "Eroare: Nu s-a dat o comanda!");
        exit(-1);
    }

    if(strcmp(command, "--add") == 0)
    {
        if(arg_index + 1 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        command_add(ID, role, user);
    }
    else if(strcmp(command, "--list") == 0)
    {
        if(arg_index + 1 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID!");
            exit(-1);
        }
        validate_symlink(argv[arg_index + 1]);
        command_list(argv[arg_index + 1], role, user);
    }
    else if(strcmp(command, "--view") == 0)
    {
        if(arg_index + 2 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID si report ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        int report_id = atoi(argv[arg_index + 2]);
        validate_symlink(ID);
        command_view(ID, report_id, role, user);
    }
    else if(strcmp(command, "--remove_report") == 0)
    {
        if(arg_index + 2 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID si report ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        int report_id = atoi(argv[arg_index + 2]);
        validate_symlink(ID);
        command_remove_report(ID, report_id, role, user);
    }
    else if(strcmp(command, "--update_threshold") == 0)
    {
        if(arg_index + 2 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID si noul prag!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        int valoare = atoi(argv[arg_index + 2]);
        validate_symlink(ID);
        command_update_threshold(ID, valoare, role, user);
    }
    else if(strcmp(command, "--filter") == 0)
    {
        if(arg_index + 2 >= argc)
        {
            fprintf(stderr, "Eroare: Lipsesc district ID si conditia!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        char *conditie = argv[arg_index + 2];
        validate_symlink(ID);
        command_filter(ID, conditie, role, user);
    }
    else if(strcmp(command, "--remove_district") == 0)
    {
        if(arg_index + 1 >= argc)
        {
            fprintf(stderr, "Eroare: Lipseste district ID!");
            exit(-1);
        }
        char *ID = argv[arg_index + 1];
        command_remove_district(ID, role, user);
    }
    return 0;
}
