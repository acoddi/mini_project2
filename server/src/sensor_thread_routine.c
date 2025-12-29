#include "common.h"
#include "device_control.h"

void *sensor_thread_routine(void *arg)
{
    DeviceStatus *status = (DeviceStatus *)arg;

    while (is_run)
    {
        // 1. 센서 값 읽기
        int current_light = digitalRead(PIN_CDS);

        // 2. flag 업데이트
        pthread_mutex_lock(&status->lock);
        status->light_level = current_light; // flag 역할
        pthread_mutex_unlock(&status->lock);

        delay(100);
    }
    return NULL;
}