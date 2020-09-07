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

#include "fanControl.h"
#include "configuration.h"
#include <EEPROM.h>
#include "../channel/channel.h"

DomDomFanControlClass::DomDomFanControlClass(){}

void DomDomFanControlClass::begin()
{
    if (!_started)
    {
        _started = true;

        pinMode(FAN_PWM_PIN, OUTPUT);

        min_pwm = 0;
        curr_pwm = 0;
        max_pwm = pow(2, FAN_PWM_RESOLUTION);

        min_channel_value = 0;
        max_channel_value = 100;

        // configure LED PWM functionalitites
        ledcSetup(FAN_PWM_CHANNEL, 50000, FAN_PWM_RESOLUTION);

        // attach the channel to be controlled
        ledcAttachPin(FAN_PWM_PIN, FAN_PWM_CHANNEL);

        // init PWM
        ledcWrite(FAN_PWM_CHANNEL, min_pwm); 

        xTaskCreate(
            this->fanTask,          /* Task function. */
            "fantTask",             /* String with name of task. */
            10000,                  /* Stack size in bytes. */
            NULL,                   /* Parameter passed as input of the task */
            1,                      /* Priority of the task. */
            NULL                    /* Task handle. */
        );
    }
}

void DomDomFanControlClass::end()
{
    if (_started)
    {
        _started = false;
    }
}

void DomDomFanControlClass::fanTask(void * parameter)
{
    while(DomDomFanControl.isStarted())
    {
        DomDomFanControl.update();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void DomDomFanControlClass::update()
{
    if (DomDomChannel.started())
    {
        int rango_canal = DomDomChannel.maximum_mA - DomDomChannel.minimum_mA;
        int valor_canal = DomDomChannel.target_mA - DomDomChannel.minimum_mA;
        int porcentaje_canal = valor_canal * 100 / rango_canal;

        int rango_fan = max_channel_value - min_channel_value;
        int valor_fan = porcentaje_canal - min_channel_value;
        int porcentaje_fan = valor_fan * 100 / rango_fan;

        potencia = porcentaje_fan;

        int new_pwm = (max_pwm - min_pwm) * (porcentaje_fan / 100);
        if (new_pwm != curr_pwm)
        {
            Serial.printf("[FAN] Nuevo PWM: %d\r\n", new_pwm);
            setCurrentPWM(new_pwm);
        }
    }
    else
    {
       setCurrentPWM(min_pwm); 
    }
    
    
}

void DomDomFanControlClass::setCurrentPWM(uint16_t pwm)
{
    uint16_t max = pow(2,FAN_PWM_RESOLUTION);
    if (pwm > max)
    {
        Serial.printf("[FAN] ERROR: valor %hu pwm mayor que el maximo. Valor cambiado a %hu\n", pwm, max);
        pwm =  max;
    }

    if (pwm < 0)
    {
        Serial.printf("[FAN] ERROR: valor %d pwm menor que 0. Valor cambiado a 0\n", pwm);
        pwm = 0;
    }

    ledcWrite(FAN_PWM_CHANNEL, pwm);

    curr_pwm = pwm;
}

bool DomDomFanControlClass::save()
{
    int address = EEPROM_FAN_ENABLED_ADDRESS;
    EEPROM.writeBool(address, _started);
    address += 1;
    EEPROM.writeUShort(address, max_pwm);
    address += 2;
    EEPROM.writeUShort(address, min_pwm);
    address += 2;
    EEPROM.writeUShort(address, max_channel_value);
    address += 2;
    EEPROM.writeUShort(address, min_channel_value);
    address += 2;
    EEPROM.writeUShort(address, curr_pwm);

    return EEPROM.commit();
}

bool DomDomFanControlClass::load()
{
    if (isStarted())
    {
        end();
    }

    int address = EEPROM_FAN_ENABLED_ADDRESS;
    address+=1;
    max_pwm = EEPROM.readUShort(address);
    address += 2;
    min_pwm = EEPROM.readUShort(address);
    address += 2;
    max_channel_value = EEPROM.readUShort(address);
    address += 2;
    min_channel_value = EEPROM.readUShort(address);
    address += 2;
    setCurrentPWM(EEPROM.readUShort(address));

    if (EEPROM.readBool(EEPROM_FAN_ENABLED_ADDRESS))
    {
        begin();
    }

    return true;
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomFanControlClass DomDomFanControl;
#endif