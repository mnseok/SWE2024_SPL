#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <locale.h>

#define ERR_EXEC_ABNORMAL -1

int ERR_INVALID_PATH() {
    write(1, "ERROR: invalid path\n", 20);
    return 0;

}

char* readline() {
	char* buf = (char*)malloc(1024);
	int offset = 0;
	while (1) {
		char c;
		int r = read(0, &c, 1);


		if (r <= 0) {
			break;
		}
		if (c == '\\') {
			int r = read(0, &c, 1);
			if (r <= 0) {
				break;
			}
			if (c == 'n') {
				break;
			}
		} else if (c == '\n') {
			break;
		}
		buf[offset++] = c;
	}
	buf[offset] = '\0';
	return buf;
}

char* stringtok(char *str, const char *delim) {
    static char *last_token = NULL;
    char *token_start;

    // If str is NULL, continue from last position
    if (str != NULL) {
        last_token = str;
    } else if (last_token == NULL) {
        return NULL;
    }

    // Skip leading delimiters
    while (*last_token != '\0') {
        const char *d = delim;
        int is_delim = 0;
        while (*d != '\0') {
            if (*last_token == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }
        if (!is_delim) {
            break;
        }
        last_token++;
    }

    if (*last_token == '\0') {
        return NULL;
    }

    token_start = last_token;

    // Find the end of the token
    while (*last_token != '\0') {
        const char *d = delim;
        int is_delim = 0;
        while (*d != '\0') {
            if (*last_token == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }
        if (is_delim) {
            *last_token = '\0';
            last_token++;
            return token_start;
        }
        last_token++;
    }

    // Reached the end of the string
    return token_start;
}

// 문자열 비교 함수
int stringcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int stringlen(const char *s) {
	int len = 0;
	while (*s) {
		len++;
		s++;
	}
	return len;
}

void stringcopy(char *dest, const char *src) {
	while (*src) {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
}

void stringcat(char *dest, const char *src) {
	while (*dest) {
		dest++;
	}
	while (*src) {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
}

void int_to_str(int num, char *str) {
	if (num == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	int i = 0;
	while (num > 0) {
		str[i++] = num % 10 + '0';
		num /= 10;
	}
	str[i] = '\0';

	// 숫자를 뒤집기
	int len = i;
	for (int j = 0; j < len / 2; j++) {
		char temp = str[j];
		str[j] = str[len - j - 1];
		str[len - j - 1] = temp;
	}
}

void stringncopy(char *dest, const char *src, int n) {
    int i = 0;
    while(i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// 옵션인지 확인하는 함수 추가
int isOption(char *arg) {
    if (arg == NULL) {
        return 0;
    }
    return arg[0] == '-';
}

int ls(char *dir_path, char *option);
int head(char *file_path, char *line);
int tail(char *file_path, char *line);
int mv(char *file_path1, char *file_path2);
int cp(char *file_path1, char *file_path2);
int pwd();

int main(){
	while(1){
		int i, cmdrlt;
		char *cmd;
		char *argument[10];
	    // size_t size;

        /* Input commands */
		cmd = readline();

        /* ============== */
        
        /* Tokenize commands (Use  function) */
		argument[0] = stringtok(cmd, " ");
		for (i = 1; i < 10; i++) {
			argument[i] = stringtok(NULL, " ");
			if (argument[i] == NULL) {
				break;
			}
		}

        /* ======================================= */


        /* Command */

		// 아래의 함수들에서 인수가 비어있는 경우에 대해 에러처리하지 않도록 미리 처리
        if (stringcmp("ls", argument[0]) == 0){
            // 인수 개수 확인
            if (argument[1] == NULL) {
                // 인수 없음: 에러
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else if (argument[2] == NULL) {
                // 인수 1개
				if (isOption(argument[1])) { // 첫번째 인수가 옵션인 경우 정상적인 명령어 호출이 아니기 때문에 에러
					cmdrlt = ERR_EXEC_ABNORMAL; 
				} else {
					cmdrlt = ls(argument[1], NULL);
				}
            } else if (argument[3] == NULL) {
                // 인수 2개
				if (isOption(argument[1]) || !isOption(argument[2])) { // 첫번째 인수가 옵션이거나 두번째 인수가 옵션이 아닌 경우 에러
					cmdrlt = ERR_EXEC_ABNORMAL;
				} else {
					cmdrlt = ls(argument[1], argument[2]);            
				}
            } else {
                // 인수 초과
                cmdrlt = ERR_EXEC_ABNORMAL;
            }
        }
		else if (stringcmp("head", argument[0]) == 0){
			// 인수 개수 확인
			// 첫 번째 인수가 -n인지 확인
			if (argument[1] == NULL || stringcmp(argument[1], "-n") != 0 || argument[2] == NULL || argument[3] == NULL || argument[4] != NULL) {
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else {
				// 인수 2개: argument[2]는 라인 수, argument[3]는 파일 경로
				cmdrlt = head(argument[3], argument[2]);
			}
		}
		else if (stringcmp("tail", argument[0]) == 0){
            // 인수 개수 확인
			// 첫 번째 인수가 -n인지 확인
			if (argument[1] == NULL || stringcmp(argument[1], "-n") != 0 || argument[2] == NULL || argument[3] == NULL || argument[4] != NULL) {
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else {
				// 인수 2개: argument[2]는 라인 수, argument[3]는 파일 경로
				cmdrlt = tail(argument[3], argument[2]);
			}
		}
		else if (stringcmp("mv", argument[0]) == 0){
            // 인수 개수 확인
            if (argument[1] == NULL || argument[2] == NULL || argument[3] != NULL) {
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else {
				cmdrlt = mv(argument[1], argument[2]);
            }
		}
		else if (stringcmp("cp", argument[0]) == 0){
			if (argument[1] == NULL || argument[2] == NULL || argument[3] != NULL) {
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else {
				cmdrlt = cp(argument[1], argument[2]);
            }
		}
		else if (stringcmp("pwd", argument[0]) == 0){
            if (argument[1] != NULL) {
                cmdrlt = ERR_EXEC_ABNORMAL;
            } else {
                cmdrlt = pwd();
            }
		}
		else if (stringcmp("quit", argument[0]) == 0){
			break;
		}
		else{
			/* Print "ERROR: invalid command" */
			write(1, "ERROR: invalid command\n", 23);
		}

		if (cmdrlt == ERR_EXEC_ABNORMAL){
            /* Print "ERROR: The command is executed abnormally" */
			write(1, "ERROR: The command is executed abnormally\n", 42);
		} 

		printf("\n");
		free(cmd);
	}
	return 0;
}

int ls(char *dir_path, char *option) {
	// 인수 검증 추가
    if (option != NULL && stringcmp(option, "-al") != 0) {
        return ERR_EXEC_ABNORMAL; // 잘못된 옵션
    }

    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    dp = opendir(dir_path);
    if(dp == NULL){
        return ERR_INVALID_PATH(); // path가 존재하지 않거나, 권한 문제
    }

	// total_blocks 변수 선언 및 초기화
    char *file_names[1024];
    int file_count = 0;
    long total_blocks = 0;

    while((entry = readdir(dp)) != NULL){
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        if(stat(fullpath, &statbuf) == -1){
            continue;
        }

        total_blocks += statbuf.st_blocks;

        int len = stringlen(entry->d_name);
        file_names[file_count] = malloc(len + 1);
        stringcopy(file_names[file_count], entry->d_name);
        file_count++;
    }
    closedir(dp);

	// 파일 이름 정렬	
    int cmpfunc(const void *a, const void *b){
        return stringcmp(*(const char **)a, *(const char **)b);
    }
    qsort(file_names, file_count, sizeof(char *), cmpfunc);

    if(option != NULL){ // 위에서 option은 항상 al임을 검증
		// total_blocks 출력
        printf("total %ld\n", total_blocks / 2);

        for(int i = 0; i < file_count; i++){
            char *filename = file_names[i];
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, filename);

            if(stat(fullpath, &statbuf) == -1){
                continue;
            }

			// 파일의 권한, 링크 수, 소유자, 그룹, 파일 크기, 수정 시간 출력
            char perms[11] = "----------";
            mode_t mode = statbuf.st_mode;

            // File type
            if(S_ISREG(mode)) perms[0] = '-';
            else if(S_ISDIR(mode)) perms[0] = 'd';
            else if(S_ISCHR(mode)) perms[0] = 'c';
            else if(S_ISBLK(mode)) perms[0] = 'b';
            else if(S_ISFIFO(mode)) perms[0] = 'p';
            else if(S_ISLNK(mode)) perms[0] = 'l';
            else if(S_ISSOCK(mode)) perms[0] = 's';

            // Owner permissions
            if(mode & S_IRUSR) perms[1] = 'r';
            if(mode & S_IWUSR) perms[2] = 'w';
            if(mode & S_IXUSR) perms[3] = 'x';

            // Group permissions
            if(mode & S_IRGRP) perms[4] = 'r';
            if(mode & S_IWGRP) perms[5] = 'w';
            if(mode & S_IXGRP) perms[6] = 'x';

            // Other permissions
            if(mode & S_IROTH) perms[7] = 'r';
            if(mode & S_IWOTH) perms[8] = 'w';
            if(mode & S_IXOTH) perms[9] = 'x';

			// 링크 수, 소유자, 그룹, 파일 크기, 수정 시간 출력
			// Number of links
            nlink_t nlink = statbuf.st_nlink;

            // User ID
			struct passwd *pwd = getpwuid(statbuf.st_uid);
			char *uid_str;
			if (pwd) {
				uid_str = pwd->pw_name;
			} else {
				uid_str = (char *)malloc(12);
				int_to_str(statbuf.st_uid, uid_str);
			}

            // File group
            struct group *grp = getgrgid(statbuf.st_gid);
			char *group;
			if (grp) {
				group = grp->gr_name;
			} else {
				group = (char *)malloc(12);
				int_to_str(statbuf.st_gid, group);
			}

            // char *group = grp ? grp->gr_name : "";

            // File size
            off_t size = statbuf.st_size;

            // Last modification time
            char timebuf[80];
            struct tm *tm = localtime(&statbuf.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

            // Adjust field widths to match expected output
            printf("%s %1ld %4s %-5s %5ld %s %s\n",
                perms, nlink, uid_str, group, (long)size, timebuf, filename);

			// free used pointers
			free(uid_str);
        }
    } else {
        // No option or option is not -al
        for(int i = 0; i < file_count; i++){
            char *filename = file_names[i];
            if(filename[0] != '.'){ // Skip hidden files
                printf("%s  ", filename);
            }
        }
        printf("\n");
    }

    // Free 
    for(int i = 0; i < file_count; i++){
        free(file_names[i]);
    }

    return 0;
}

int head(char *file_path, char *line){
	int K;
	K = atoi(line);
	if ((line[0] != '0' && K == 0) || K < 0) { // 음수 표기이거나, 0이 아닌데 0으로 변환되는 경우 비정상적인 것으로 간주하여 에러 처리
		return ERR_EXEC_ABNORMAL;
	}

	if (K == 0) { // 0인 경우 아무것도 출력하지 않음
		return 0;
	}

    int fd = open(file_path, O_RDONLY);
    if(fd == -1){
		return ERR_INVALID_PATH(); // 정상적인 경로가 아니거나, 권한 문제 등으로 인한 에러
    }

    char buf[1];
    int lines_read = 0;
    ssize_t bytes_read;
    char line_buffer[1024];
    int line_index = 0;

    while(lines_read < K && (bytes_read = read(fd, buf, 1)) > 0){
		if (bytes_read == -1) {
			close(fd);
			return ERR_EXEC_ABNORMAL;
		}

        line_buffer[line_index++] = buf[0];
        if(buf[0] == '\n'){
            line_buffer[line_index] = '\0';
            printf("%s", line_buffer);
            line_index = 0;
            lines_read++;
        } else if(line_index >= 1023){
            line_buffer[line_index] = '\0';
            printf("%s", line_buffer);
            line_index = 0;
        }
    }

    // If the file ends without a newline, print the last line
    if(line_index > 0){
        line_buffer[line_index] = '\0';
        printf("%s", line_buffer);
    }

	if (close(fd) == -1) {
		return ERR_EXEC_ABNORMAL;
	}
    return 0;
}

int tail(char *file_path, char *line){
	int K;
	K = atoi(line);
	if ((line[0] != '0' && K == 0)|| K < 0) { // 음수 표기이거나, 0이 아닌데 0으로 변환되는 경우 비정상적인 것으로 간주하여 에러 처리
		return ERR_EXEC_ABNORMAL;
	}

	if (K == 0) { // 0인 경우 아무것도 출력하지 않음
		return 0;
	}

    int fd = open(file_path, O_RDONLY);
    if(fd == -1){
        return ERR_INVALID_PATH();
    }

    if(K > 1024){
        K = 1024; // Set a maximum number of lines
    }

    char *lines[1024];
    char buf[1];
    ssize_t bytes_read;
    char line_buffer[1024];
    int line_index = 0;
    int total_lines = 0;

    while((bytes_read = read(fd, buf, 1)) > 0){
		if (bytes_read == -1) {
			close(fd);
			return ERR_EXEC_ABNORMAL;
		}
        if(line_index >= 1023){
            line_index = 0;
        }
        line_buffer[line_index++] = buf[0];
        if(buf[0] == '\n'){
            line_buffer[line_index] = '\0';
            lines[total_lines % K] = malloc(line_index + 1);
            stringcopy(lines[total_lines % K], line_buffer);
            total_lines++;
            line_index = 0;
        }
    }

    // If the file ends without a newline
    if(line_index > 0){
        line_buffer[line_index] = '\0';
        lines[total_lines % K] = malloc(line_index + 1);
        stringcopy(lines[total_lines % K], line_buffer);
        total_lines++;
    }

    int start = total_lines > K ? total_lines % K : 0;
    int count = total_lines > K ? K : total_lines;
    for(int i = 0; i < count; i++){
        int index = (start + i) % K;
        printf("%s", lines[index]);
        free(lines[index]);
    }

    if (close(fd) == -1) {
		return ERR_EXEC_ABNORMAL;
	}
    return 0;
}

int mv(char *file_path1, char *file_path2){
    int result = rename(file_path1, file_path2);
    if(result == -1){
        return ERR_INVALID_PATH();
    }
    return 0;
}

int cp(char *file_path1, char *file_path2){
    int fd_in = open(file_path1, O_RDONLY);
    if(fd_in == -1){
        return ERR_INVALID_PATH();
    }

    int fd_out = open(file_path2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd_out == -1){
        close(fd_in);
		return ERR_INVALID_PATH();
    }
    char buffer[4096];
    ssize_t bytes_read;
    while((bytes_read = read(fd_in, buffer, sizeof(buffer))) > 0){
        ssize_t bytes_written = write(fd_out, buffer, bytes_read);
        if(bytes_written != bytes_read){
            close(fd_in);
            close(fd_out);
            return -1;
        }
    }
    close(fd_in);
    close(fd_out);
    return 0;
}

int pwd(){
	char *buffer;

    // 버퍼 할당
	buffer = getcwd(NULL, 0); // NULL과 0을 전달하면 필요 크기만큼 자동 할당
	if (buffer == NULL) {
		return ERR_EXEC_ABNORMAL;
	}

    // 현재 작업 디렉토리 출력
	printf("%s\n", buffer);

    // 할당한 버퍼 해제
    free(buffer);
    return 0;
}

