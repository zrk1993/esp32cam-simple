#ifndef BLink_h
#define BLink_h

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include <Arduino.h>

void blink(int times, int dl)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    bool led_on = false;
    for (int i = 0; i <= times * 2; i++)
    {
        delay(dl);
        led_on = !led_on;
        digitalWrite(LED_BUILTIN, led_on ? LOW : HIGH);
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void blink(int times)
{
    blink(times, 300);
}

void blink()
{
    blink(100, 300);
}

void blink_err()
{
    blink(5, 100);
}

void blink_ok()
{
    blink(2, 300);
}

#endif