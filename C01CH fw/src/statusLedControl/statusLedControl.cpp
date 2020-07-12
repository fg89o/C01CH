/**
 * DomDom Firmware
 * Copyright (c) 2020 DomDomFirmware
 *

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "statusLedControl.h"
#include "configuration.h"

DomDomStatusLedControlClass::DomDomStatusLedControlClass()
{
    xMutex = xSemaphoreCreateMutex();
};

void DomDomStatusLedControlClass::begin()
{
    pinMode(LED_STATUS_PIN, OUTPUT);

    min_pwm = 0;
    max_pwm = pow(2, LED_STATUS_RESOLUTION);
    blinking = false;

    // configure LED PWM functionalitites
    ledcSetup(LED_STATUS_CHANNEL, 5000, LED_STATUS_RESOLUTION);
    
    // attach the channel to be controlled
    ledcAttachPin(LED_STATUS_PIN, LED_STATUS_CHANNEL);

    // init PWM
    ledcWrite(LED_STATUS_CHANNEL, max_pwm); 

}

void DomDomStatusLedControlClass::blinkInfinite()
{
    if (!isBlinking())
    {
        blinking = true;

        xTaskCreate(
            this->blinkTask,        /* Task function. */
            "Blink_Task",             /* String with name of task. */
            10000,                  /* Stack size in bytes. */
            NULL,                   /* Parameter passed as input of the task */
            1,                      /* Priority of the task. */
            NULL                    /* Task handle. */
        );
    }
    
}

void DomDomStatusLedControlClass::blink( int times)
{
    if (!isBlinking())
    {
        block();
        for (int i = 0; i < times; i++)
        {
            off();
            delay(LED_STATUS_BLINK_DELAY);
            on();
            delay(LED_STATUS_BLINK_DELAY);
            off();

            if (i < times)
            {
                delay(getDelay() / portTICK_PERIOD_MS);
            }
        }
        release();
    }
}

void DomDomStatusLedControlClass::blinkTask(void * parameter)
{
    DomDomStatusLedControl.block();
    while (DomDomStatusLedControl.isBlinking())
    {
        ledcWrite(LED_STATUS_CHANNEL, DomDomStatusLedControl.min_pwm);
        vTaskDelay(LED_STATUS_BLINK_DELAY / portTICK_PERIOD_MS);
        ledcWrite(LED_STATUS_CHANNEL, DomDomStatusLedControl.max_pwm);
        vTaskDelay(LED_STATUS_BLINK_DELAY / portTICK_PERIOD_MS);
        ledcWrite(LED_STATUS_CHANNEL, DomDomStatusLedControl.min_pwm);
        vTaskDelay(DomDomStatusLedControl.getDelay() / portTICK_PERIOD_MS);
    }
    DomDomStatusLedControl.release();

    vTaskDelete(NULL);
}

void DomDomStatusLedControlClass::block()
{
    xSemaphoreTake( xMutex, portMAX_DELAY );
}

void DomDomStatusLedControlClass::release()
{
    xSemaphoreGive( xMutex );
}

void DomDomStatusLedControlClass::on()
{
    blinking = false;
    block();
    Serial.println("Status led on");
    ledcWrite(LED_STATUS_CHANNEL, max_pwm);
    release();
}

void DomDomStatusLedControlClass::off()
{
    blinking = false;
    block();
    Serial.println("Status led off");
    ledcWrite(LED_STATUS_CHANNEL, min_pwm);
    release();
}

bool DomDomStatusLedControlClass::isBlinking()
{
    return blinking;
}

bool DomDomStatusLedControlClass::getDelay()
{
    return blinkDelay;
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomStatusLedControlClass DomDomStatusLedControl;
#endif