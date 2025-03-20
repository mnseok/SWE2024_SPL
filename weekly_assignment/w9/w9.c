#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

int main(){
	char *cmd = NULL;
	size_t size = 0;
	char *arg[11];

	while(1){
		int i  = 0;
		getline(&cmd, &size, stdin);
		cmd[strlen(cmd)-1] = '\0';
		if(strlen(cmd) == 0) continue;

		char* ptr = strtok(cmd, " ");
		while(ptr != NULL){
			arg[i++] = ptr;
			ptr = strtok(NULL, " ");
		}
		arg[i] = NULL;
		if(strcmp("quit", arg[0]) == 0)	break;
				
		char path[100];
		sprintf(path, "/bin/%s", arg[0]);
	
		int target_idx = -1;
		for(int x = 0;x < i; x++){
			if(strcmp("<", arg[x]) == 0 || strcmp(">", arg[x]) == 0 || strcmp("|", arg[x]) == 0)
				target_idx = x;
		}


		if(target_idx == -1){
			// target_idx == -1 는 실행 할 명령어가 1개인 경우
			// 자식 프로세스 execv()해서 실행하면 끝
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				exit(1);
			}

			if (pid == 0) { // 자식 프로세스
				execv(path, arg);
				perror("execv failed"); // execv 실패 시 오류 메시지 출력
				exit(1);
			}
			wait(NULL); // 부모 프로세스는 자식의 종료를 기다림

		} else if( strcmp(">",  arg[target_idx]) == 0){
			// 실행 할 명령어가 2개이며, ">" redirection으로 연결되어 있는 경우
			// file을 write로 open하고 자식 프로세스에서 execv를 실행하고 dup2를 stdout을 open한 파일로 대체

			arg[target_idx] = NULL; // 리다이렉션 기호 뒤의 인자 제거
			pid_t pid = fork();
			if (pid == 0) { // 자식 프로세스
				int fd = open(arg[target_idx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (fd < 0) {
					perror("open failed");
					exit(1);
				}
				dup2(fd, STDOUT_FILENO); // 표준 출력을 파일로 대체
				close(fd);
				execv(path, arg);
				perror("execv failed");
				exit(1);
			}
			wait(NULL);
		}else if( strcmp("<", arg[target_idx]) == 0){
			// 실행 할 명령어가 2개이며, "<" redirection으로 연결되어 있는 경우
			// file을 read로 open하고 자식 프로세스에서 execv를 실행하고 dup2를 stdin을 open한 파일로 대체
			arg[target_idx] = NULL;
			pid_t pid = fork();
			if (pid == 0) { // 자식 프로세스
				int fd = open(arg[target_idx + 1], O_RDONLY);
				if (fd < 0) {
					perror("open failed");
					exit(1);
				}
				dup2(fd, STDIN_FILENO); // 표준 입력을 파일로 대체
				close(fd);
				execv(path, arg);
				perror("execv failed");
				exit(1);
			}
			wait(NULL);
		}else if( strcmp("|", arg[target_idx]) == 0){
			// 실행 할 명령어가 2개이며, "|" redirection으로 연결되어 있는 경우
			// 자식 프로세스를 2개 생성하여 각각 execv를 실행하고 첫 번째 자식의 stdout과 두 번째 자식의 stdin을 pipe로 연결
			arg[target_idx] = NULL;
			int fd[2]; // 말하는 쪽이 1, 듣는 쪽이 0
			if (pipe(fd) < 0) {
				perror("pipe failed");
				exit(1);
			}

			pid_t pid1 = fork();
			if (pid1 < 0) {
				perror("fork failed");
				exit(1);
			}
			if (pid1 == 0) { // 첫번째 자식 프로세스 (말하는 쪽)
				close(fd[0]); // 듣는 쪽은 닫음
				dup2(fd[1], STDOUT_FILENO); // 표준 출력을 파이프로 대체
				close(fd[1]);
				execv(path, arg);
				perror("execv failed");
				exit(1);
			} 

			pid_t pid2 = fork();
			if (pid2 < 0) {
				perror("fork failed");
				exit(1);
			}
			if (pid2 == 0) { // 두 번째 자식 프로세스
				close(fd[1]);
				char path2[100];
				sprintf(path2, "/bin/%s", arg[target_idx + 1]); // 두 번째 명령어 경로
				dup2(fd[0], STDIN_FILENO); // stdin을 파이프의 읽기 끝으로 설정
				close(fd[0]);
				execv(path2, &arg[target_idx + 1]); // 두 번째 명령어 실행
				perror("execv failed");
				exit(1);
			}
			close(fd[0]);
			close(fd[1]);
			wait(NULL); // 부모 프로세스는 자식의 종료를 기다림
			wait(NULL);

		}	
	}
}


