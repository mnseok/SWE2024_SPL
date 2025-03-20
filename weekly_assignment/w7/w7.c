#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handle_sigint(int sig) {
    for (int i = 0; i < 5; i++) {
        printf("BEEP\n");
        sleep(1);
    }
    printf("I'm Alive!\n");
}

int main() {
    signal(SIGINT, handle_sigint);
    
    while (1) {
        sleep(1);
    }
    
    return 0;
}

