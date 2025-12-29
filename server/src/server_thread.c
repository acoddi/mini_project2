#include "common.h"
#include "device_control.h"

void *server_thread(void *arg)
{
    // tcp ipv4 소켓 생성
    int ssock = socket(AF_INET, SOCK_STREAM, 0);

    // 주소 구조체 선언
    struct sockaddr_in servaddr, cliaddr;

    // 주소 구조체 초기화
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // 바인딩 부분
    int opt = 1;
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // 리슨 부분 백로그 10개 생성
    if (listen(ssock, 10) == -1)
    {
        perror("listen");
        exit(1);
    }
    while (is_run)
    {
        unsigned int len = sizeof(cliaddr);
        int csock = accept(ssock, (struct sockaddr *)&cliaddr, &len);
        if (csock < 0)
            continue;

        int *new_sock = malloc(sizeof(int));
        *new_sock = csock;
        pthread_t thread;
        pthread_create(&thread, NULL, client_connection, new_sock);
        pthread_detach(thread);
    }

    close(ssock);
    return NULL;
}

// 클라이언트에서 받은 문자 파싱 하는 함수
void *client_connection(void *arg)
{
    int csock = *((int *)arg);
    free(arg);
    DeviceStatus *status = get_device_status();
    char buf[BUFSIZ];
    int n;

    while ((n = read(csock, buf, sizeof(buf) - 1)) > 0)
    {
        buf[n] = '\0';
        char cmd;
        int val;
        if (sscanf(buf, " %c %d", &cmd, &val) == 2)
        {
            pthread_mutex_lock(&status->lock);

            // led on/off
            if (cmd == 'L' || cmd == 'l')
                status->led_brightness = val;
            // segment
            else if (cmd == 'F' || cmd == 'f')
            {
                status->fnd_value = val;
                status->is_counting = 1;
            }
            // buzzer
            else if (cmd == 'B' || cmd == 'b')
                status->buzzer_status = val;

            pthread_mutex_unlock(&status->lock);
        }
        write(csock, "ACK\n", 4);
    }
    close(csock);
    return NULL;
}
