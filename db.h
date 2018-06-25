#define MAX_READ 512

void input(int fd, char *path);
void find(int fd, char *name);
void export(int fd, char *command);
void delete(int fd, char *path, char *name);