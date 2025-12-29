#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "device_control.h"

volatile int is_run = 1;

DeviceStatus global_status;

DeviceStatus *get_device_status()
{
    return &global_status;
}
void make_daemon();

int main(int argc, char **argv)
{

    void *handle;

    typedef void *(*thread_func_t)(void *);
    thread_func_t led_f, buzzer_f, sensor_f, fnd_f, server_f;

    // 라이브러리 파일 열기
    handle = dlopen("./lib/libdevice_control.so", RTLD_LAZY);
    if (!handle)
    {
        // syslog 로그 -> 에러시 발생
        syslog(LOG_ERR, "DEBUG: dlopen failed: %s\n", dlerror());
        return -1;
    }

    // 라이브러리에서 주소 찾기
    led_f = (thread_func_t)dlsym(handle, "led_thread_routine");
    buzzer_f = (thread_func_t)dlsym(handle, "buzzer_thread_routine");
    sensor_f = (thread_func_t)dlsym(handle, "sensor_thread_routine");
    fnd_f = (thread_func_t)dlsym(handle, "fnd_thread_routine");
    server_f = (thread_func_t)dlsym(handle, "server_thread");

    // 심볼 확인 (하나라도 없으면 종료)
    if (!led_f || !buzzer_f || !sensor_f || !fnd_f || !server_f)
    {
        // syslog 로그 -> 에러시 발생
        syslog(LOG_ERR, "Symbol lookup failed: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    // 데몬 프로세스로 실행해라
    make_daemon();
    // syslog 로그 출력
    syslog(LOG_INFO, "Device Daemon Started");

    // 2. 하드웨어 및 뮤텍스 설정
    if (wiringPiSetup() == -1)
    {
        // syslog 로그 -> 에러시 발생
        syslog(LOG_ERR, "wiringPiSetup Failed");
        return -1;
    }
    pthread_mutex_init(&global_status.lock, NULL);

    global_status.led_brightness = 0;
    global_status.fnd_value = 0;
    global_status.is_counting = 0;
    global_status.light_level = 0;

    // 3. 스레드 생성
    pthread_t t_led, t_buzzer, t_sensor, t_fnd, t_server;

    pthread_create(&t_led, NULL, led_f, &global_status); // 찾아낸 포인터 사용
    pthread_create(&t_buzzer, NULL, buzzer_f, &global_status);
    pthread_create(&t_sensor, NULL, sensor_f, &global_status);
    pthread_create(&t_fnd, NULL, fnd_f, &global_status);
    pthread_create(&t_server, NULL, server_f, &global_status);

    // syslog 출력
    syslog(LOG_INFO, "All threads created successfully");

    pthread_join(t_led, NULL);
    pthread_join(t_buzzer, NULL);
    pthread_join(t_sensor, NULL);
    pthread_join(t_fnd, NULL);
    pthread_join(t_server, NULL);

    pthread_mutex_destroy(&global_status.lock);
    // syslog 출력
    syslog(LOG_INFO, "Device Daemon Terminated");
    closelog();

    return 0;
}

// 데몬 프로세스를 만드는 함수
void make_daemon()
{
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        perror("getrlimit");

    if ((pid = fork()) < 0)
        exit(1);
    else if (pid != 0)
        exit(0); // 부모 종료

    // 4. 새로운 세션 생성
    if (setsid() < 0)
    {
        syslog(LOG_ERR, "daemon setsid failed: %m");
        exit(1);
    }

    // 터미널 관련 시그널 무시
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("sigaction() : Can't ignore SIGHUP");
    }

    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "daemon chdir to / failed: %m");
        exit(1);
    }

    // 모든 파일 디스크립터 닫기
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i < rl.rlim_max; i++)
        close(i);

    // 표준 입출력을 /dev/null로 리다이렉트
    int fd0 = open("/dev/null", O_RDWR); // fd 0
    int fd1 = dup(0);                    // fd 1
    int fd2 = dup(0);                    // fd 2

    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        return;
    }

    openlog("device_daemon", LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "daemon initialized successfully");

    return;
}