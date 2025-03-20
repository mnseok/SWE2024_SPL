#include<stdio.h>
#include<syslog.h>
#include <unistd.h>
#include<stdlib.h>
#include<fcntl.h>

int main() {
    unsigned int pid;
    int fd0, fd1, fd2;
    if ((pid=fork())!=0) exit(0);
    if (setsid() < 0) exit(0);
    if (chdir("/") < 0) exit(0);

    umask(0);

    close(0); close(1); close(2);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = open("/dev/null", O_RDWR);
    fd2 = open("/dev/null", O_RDWR);

    setlogmask(LOG_MASK(LOG_INFO));
    openlog("vsd", LOG_PID, LOG_USER);
    int i = 0;
    while (1) {
        syslog(LOG_INFO, "Very Simple Daemon %d", i++);
        sleep(10);
    }
    return 0;

}
