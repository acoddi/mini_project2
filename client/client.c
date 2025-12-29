#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUF_SIZE 1024

int sock;

// SIGINT 핸들러
void handle_signal(int sig)
{
    printf("\n종료 신호(%d) 수신. 서버 연결을 해제하고 종료합니다.\n", sig);
    if (sock != -1)
    {
        close(sock);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // 시그널 설정 (sigaction)
    struct sigaction sa;
    sa.sa_handler = handle_signal; // 핸들러 함수 등록
    sigemptyset(&sa.sa_mask);      // 마스크 초기화
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction error");
        exit(1);
    }

    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // 3. 서버 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }

    printf("서버에 연결되었습니다!\n");
    printf("L: 0 ~ 3, B 0 ~ 1, F 1 ~ 9\n");
    printf("명령어 예시: L 1 (LED), B 1 (부저), F 5 (FND)\n");
    printf("조도센서 활성화 조건: L 0 입력으로 LED 끄기\n");
    printf("종료하려면 Ctrl+C를 누르세요.\n");

    while (1)
    {
        printf(">> ");
        if (fgets(message, BUF_SIZE, stdin) == NULL)
            break;

        // 서버로 전송
        write(sock, message, strlen(message));

        // 5. 서버로부터 ACK 수신
        str_len = read(sock, message, BUF_SIZE - 1);
        if (str_len <= 0)
        {
            printf("서버와의 연결이 끊겼습니다.\n");
            break;
        }

        message[str_len] = '\0';
        printf("Server: %s", message);
    }

    close(sock);
    return 0;
}