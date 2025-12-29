#include "common.h"
#include "device_control.h"

#define TOTAL (sizeof(notes) / sizeof(notes[0]))

int notes[] = {
    392, 523, 659, 784, 698, 659, 523, 440,
    440, 523, 698, 880, 784, 698, 659, 587,
    392, 523, 659, 784, 698, 659, 523, 440,
    440, 523, 698, 880, 784, 659, 587, 523,
    659, 784, 880, 1046, 987, 880, 784,
    784, 698, 659, 698, 659, 587, 523};

void *buzzer_thread_routine(void *arg)
{
    DeviceStatus *status = (DeviceStatus *)arg;

    // 장치 초기화 (wiringPi 핀 모드 설정)
    if (softToneCreate(PIN_BUZZER) != 0) /* 톤 출력을 위한 GPIO 설정 */
    {
        fprintf(stderr, "buzzer init error");
        return NULL;
    }

    while (is_run)
    {
        // 뮤텍스로 상태 확인
        pthread_mutex_lock(&status->lock);
        int play_music = status->buzzer_status;

        // 부저 on || 7세그먼트가 카운트 1
        if (status->buzzer_status == 1 || (status->fnd_value == 0 && status->is_counting == 1))
        {
            play_music = 1;
        }

        pthread_mutex_unlock(&status->lock);

        if (play_music)
        {
            for (int i = 0; i < TOTAL; ++i)
            {
                pthread_mutex_lock(&status->lock);
                if (status->buzzer_status == 0 && status->is_counting == 0)
                {
                    pthread_mutex_unlock(&status->lock);
                    break;
                }
                pthread_mutex_unlock(&status->lock);

                softToneWrite(PIN_BUZZER, notes[i]);

                //  종료 반응성을 위해 딜레이를 잘게 쪼갬
                for (int d = 0; d < 25; d++)
                {
                    if (!is_run)
                        break;
                    delay(10);
                }

                softToneWrite(PIN_BUZZER, 0);
                delay(30);

                if (!is_run)
                    break;
            }
            softToneWrite(PIN_BUZZER, 0); // 연주 후 정지

            // 연주 후 반복 안되게 초기화
            pthread_mutex_lock(&status->lock);
            status->buzzer_status = 0;
            status->is_counting = 0;
            pthread_mutex_unlock(&status->lock);
        }
    }

    // 종료 전 안전하게 부저 끄기
    softToneWrite(PIN_BUZZER, 0);
    printf("Buzzer thread terminated.\n");
    return NULL;
}
