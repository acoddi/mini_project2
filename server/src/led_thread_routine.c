#include "common.h"
#include "device_control.h"

#define BRIGHT_HIGH 255
#define BRIGHT_MED 128
#define BRIGHT_LOW 30
#define BRIGHT_OFF 0

void *led_thread_routine(void *arg)
{
    DeviceStatus *status = (DeviceStatus *)arg;
    int target_pwm = 0;

    // 초기화
    pinMode(PIN_LED, OUTPUT);
    softPwmCreate(PIN_LED, 0, 255);

    while (is_run)
    {
        pthread_mutex_lock(&status->lock);

        // 밝기 조절
        if (status->led_brightness > 0)
        {
            switch (status->led_brightness)
            {
            case 1:
                target_pwm = BRIGHT_LOW;
                break;
            case 2:
                target_pwm = BRIGHT_MED;
                break;
            case 3:
                target_pwm = BRIGHT_HIGH;
                break;
            default:
                target_pwm = BRIGHT_OFF;
            }
        }
        else
        {
            target_pwm = (status->light_level == HIGH) ? BRIGHT_OFF : BRIGHT_MED;
        }
        pthread_mutex_unlock(&status->lock);

        softPwmWrite(PIN_LED, target_pwm);

        delay(100);
    }

    softPwmWrite(PIN_LED, 0);
    return NULL;
}
