#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

// use stdio.h only for debugging

int itoa(int cur, int fd){
    int tmp = cur, i=0, j=0;
    
    char tmpStr[8]={}, str[8];
    while (tmp>0) {
        tmpStr[i++]=(char) (((int)'0') + tmp%10);
        tmp/=10;
    }
    i--; 

    while(i>=0) {
        str[j++]=tmpStr[i--];
    }
    str[j++] = ' ';
    str[j++] = '|';
    str[j++] = ' ';
    str[j]='\0';

    write(fd, str, j);
    return cur+1;
	/* Implement itoa function here  */

}

int main (int argc, char **argv){
	int fd = open("Aladdin.txt", O_RDONLY);
	int fd2 = open("numbered.txt", O_WRONLY | O_CREAT, 0644);
	int linenum = 1;
	char c;

	/* Read one character from fd(Aladdin.txt) */ 
	/* Write one character to fd2(numbered.txt) */
    /* if character is newline, write linenumber after newline using itoa */	
	linenum=itoa(linenum, fd2);
    while (read(fd, &c, 1)==1) {
        write(fd2, &c, 1);
        if (c == '\n') {
            linenum = itoa(linenum, fd2);
        }
    }

	close(fd);
	close(fd2);
}
