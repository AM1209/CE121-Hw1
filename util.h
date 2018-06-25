#define MAX_READ 512

int exists (int fd, char *name);  //if found returns start of object, if not found returns -1
int find_end(int fd, char *name, int start);
void copy(int target, int source, int start, int end);