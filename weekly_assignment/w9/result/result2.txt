#include <stdio.h>
#include <string.h>

void replaceSpaceWithNewline(char *str) {
    int i;
    for(i = 0; str[i] != '\0'; i++) {
        if(str[i] == ' ') {
            str[i] = '\n';
        }
    }
}

int main() {
    char str[1000];  // 충분한 크기의 버퍼 할당
    
    fgets(str, sizeof(str), stdin);
    
    str[strcspn(str, "\n")] = '\0';
    
    replaceSpaceWithNewline(str);
    printf("%s\n", str);
    
    return 0;
}
