#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("\nПередано некорректное количество аргументов!\n");
        printf("\nВведите struct_id и PID!\n");
        printf("struct_id:\n");
        printf("\t1: task_cputime\n\t2: vm_area_struct\n\n");
        return 0;
    }

    printf("Цыпандин Николай lab2: procfs: task_cputime, vm_area_struct\n");

    int struct_id = atoi(argv[1]);
    pid_t console_pid = atoi(argv[2]);

    int fd = open("/proc/Lab2/my_driver", O_RDWR);

    char input_buf[BUF_SIZE];
    char output_buf[BUF_SIZE];

    sprintf(input_buf, "%d", console_pid);

    write(fd, input_buf, strlen(input_buf));

    lseek(fd, 0, SEEK_SET);

    sprintf(output_buf, "%d", struct_id);
    read(fd, output_buf, strlen(output_buf));

    puts(output_buf);
    return 0;
}
