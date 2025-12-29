#include "common.h"
#include "device_control.h"

// BCD 입력을 위한 4개 핀 (A, B, C, D)
static const int gpiopins[4] = {4, 1, 16, 15};

// 0~9까지의 2진수(BCD) 데이터 패턴
static const int bcd_number[10][4] = {
    {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 1, 0, 0}, {0, 1, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 0, 0}, {1, 0, 0, 1}};

// 숫자를 디코더 칩으로 전송하는 함수
void display_fnd(int num)
{
    if (num < 0 || num > 9)
        return;
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(gpiopins[i], bcd_number[num][i] ? HIGH : LOW);
    }
}

void *fnd_thread_routine(void *arg)
{
    DeviceStatus *status = (DeviceStatus *)arg;

    // 1. 핀 초기화
    for (int i = 0; i < 4; i++)
    {
        pinMode(gpiopins[i], OUTPUT);
        digitalWrite(gpiopins[i], HIGH); // 칩 특성에 따라 초기값 설정
    }

    while (is_run)
    {
        pthread_mutex_lock(&status->lock);

        // 표시할 숫자 출력
        display_fnd(status->fnd_value);

        // 카운트다운 로직
        if (status->is_counting)
        {
            pthread_mutex_unlock(&status->lock);

            sleep(1); // 1초 대기

            pthread_mutex_lock(&status->lock);

            // 1s씩 감소
            if (status->fnd_value > 0)
            {
                status->fnd_value--;
            }
            else
            {
                // 0초가 되면 카운팅 상태 해제
                status->is_counting = 0;
            }
        }

        pthread_mutex_unlock(&status->lock);
    }

    // 종료 시 초기화
    for (int i = 0; i < 4; i++)
        digitalWrite(gpiopins[i], HIGH);
    printf("FND thread terminated.\n");
    return NULL;
}