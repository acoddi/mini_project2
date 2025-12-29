#ifndef __DEVICE_CONTROL_H__
#define __DEVICE_CONTROL_H__

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <softTone.h>

#define PIN_BUZZER 6 /* GPIO25 */
#define PIN_SW 5     /* GPIO24 */
#define PIN_CDS 0    /* GPIO17 */
#define PIN_LED 29   /* GPIO29 */

// 장치 제어 스레드 함수.
void *led_thread_routine(void *arg);    // LED 제어 루틴
void *buzzer_thread_routine(void *arg); // 부저 제어 루틴
void *sensor_thread_routine(void *arg); // 조도센서 모니터링 루틴
void *fnd_thread_routine(void *arg);    // 7세그먼트 카운트다운 루틴

#endif