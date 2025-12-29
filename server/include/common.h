#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>

#define PORT 8080
#define BUF_SIZE 1024

extern volatile int is_run;      // 프로그램 종료 플래그
extern volatile int is_counting; // 카운트다운 진행 중 플래그

// 장치 상태를 담는 구조체 (스레드 간 공유 자원)
typedef struct
{
    int led_brightness;
    int buzzer_status;
    int light_level;
    int fnd_value;
    int is_counting;
    int keep_running;     // SIGINT 시 0으로 변경하여 스레드 종료 유도
    pthread_mutex_t lock; // 공유 자원 보호를 위한 뮤텍스
} DeviceStatus;

// 스레드 함수에 전달할 인자 구조체
typedef struct
{
    int client_sock;
    DeviceStatus *status;
} ThreadArgs;

void *server_thread(void *arg);

// 클라이언트 요청 처리용 스레드 함수
void *client_connection(void *arg);

DeviceStatus *get_device_status();

#endif
