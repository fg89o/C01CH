/**
 * Oscar Fernández
 * Copyright (c) 2020 
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

#include "channel.h"
#include <EEPROM.h>
#include "configuration.h"
#include "../log/logger.h"
#include "channelMgt.h"

const uint32_t SHUNT_MICRO_OHM      = 100000;  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS         = 3;       ///< Max expected amps, values are 1 - clamped to max 1022
const uint16_t INA_AVERAGING        = 64;
const uint16_t INA_CONVERSION_TIME  = 8244;

DomDomChannelClass::DomDomChannelClass(uint8_t channel)
{
    _channel_num = channel;
    tag = String("CHANNEL ");
    tag.concat(_channel_num+1);

    _enabled = true;
    _iniciado = false;
    
    _current_pwm = 0;
}

bool DomDomChannelClass::begin(uint8_t INA_address, uint8_t pwm_pin, uint8_t resolution)
{
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Iniciando canal...");
    if(started())
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, tag.c_str(), "Canal ya iniciado...ERROR!");
        return false;
    }

    INA_device_index = UINT8_MAX;
    _INA_address = INA_address;

    _pwm_pin = pwm_pin;
    _resolution = resolution;

    if (!_enabled)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "Canal no habilitado");
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "Iniciando canal...ERROR!");
        return false;
    }

    // configure LED PWM functionalitites
    ledcSetup(_channel_num, 5000, _resolution);
    
    // attach the channel to be controlled
    ledcAttachPin(_pwm_pin, _channel_num);

    // init PWM
    ledcWrite(_channel_num, _current_pwm); 

    // Iniciamos el INA
    INA._EEPROM_offset = EEPROM_SIZE;
    INA._EEPROM_size = EEPROM_INA_SIZE;
    uint8_t devicesFound = 0;
    uint8_t max_retries = 3;
    uint8_t retry = 0;

    do
    {
        devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);
        for (uint8_t i = 0; i < devicesFound; i++)  // Loop through all devices
        {
            if (INA.getDeviceAddress(i) == _INA_address )
            {
                INA_device_index = i;
            }
        } 

        retry++;

    }while (INA_device_index == UINT8_MAX && retry < max_retries);

    if (INA_device_index == UINT8_MAX)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "INA no encontrado");
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "Iniciando canal...ERROR!");

        _enabled = false;

        return false;
    }

    INA.setAveraging(INA_AVERAGING, INA_device_index);                         // Average each reading n-times
    INA.setBusConversion(INA_CONVERSION_TIME,INA_device_index);                // Maximum conversion time 8.244ms
    INA.setShuntConversion(INA_CONVERSION_TIME,INA_device_index);              // Maximum conversion time 8.244ms
    INA.setMode(INA_MODE_CONTINUOUS_BOTH,INA_device_index);                    // Bus/shunt measured continuously

    _iniciado = true;
    xTaskCreate(
        this->CurrentMonitorTask, /* Task function. */
        "Current_Monitor_Task",   /* String with name of task. */
        10000,                    /* Stack size in bytes. */
        (void *)_channel_num,     /* Parameter passed as input of the task */
        1,                        /* Priority of the task. */
        NULL                      /* Task handle. */
    );

    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Iniciando canal...OK!");
    return true;
}

bool DomDomChannelClass::end()
{
    _iniciado = false;
    return true;
}


void DomDomChannelClass::CurrentMonitorTask(void *parameter)
{
    int channel_num = (int)parameter;
    DomDomChannelClass * channel = DomDomChannelMgt.channels[channel_num];

    while(channel->started())
    {
        channel->INA.waitForConversion(channel->INA_device_index);
        
        float volts = channel->INA.getBusMilliVolts(channel->INA_device_index) / 1000.0f;
        float amps = channel->INA.getBusMicroAmps(channel->INA_device_index) / 1000.0f;
        float power = amps * volts;
        power = power < 0 ? 0 : power;

        // Guardamos los valores actuales
        channel->lastBusVoltaje_V = volts;
        channel->lastBusCurrent_mA = amps;

        // Guardamos los maximos
        channel->busPowerPeak_W = channel->busPowerPeak_W > power ? channel->busPowerPeak_W : power;
        channel->busCurrentPeak_mA = channel->busCurrentPeak_mA > amps ? channel->busCurrentPeak_mA : amps;
        channel->busVoltagePeak_V = channel->busVoltagePeak_V > volts ? channel->busVoltagePeak_V : volts;

        vTaskDelay(CHANNEL_BUS_REFRESH_INTERVAL / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

bool DomDomChannelClass::setEnabled(bool enabled)
{
    if (_enabled != enabled)
    {
        _enabled = enabled;

        if (_iniciado && !enabled)
        {
            end();
        }
    }

    return true;
}

bool DomDomChannelClass::setPWM(uint16_t value, bool guardar)
{
    if (!_enabled && value != 0)
    {
        return false;
    }

    if (value > pow(2,_resolution))
    {
        value =  pow(2,_resolution);
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, tag.c_str(), "El valor PWM es mayor que el maximo. Valor cambiado a %f", value);
    }

    if (value < 0)
    {
        value = 0;
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, tag.c_str(), "El valor PWM es mayor que el maximo. Valor cambiado a %f", value);
    }

    _current_pwm = value;
    ledcWrite(_channel_num, value); 

    if (guardar)
    {
        saveCurrentPWM();
    }

    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, tag.c_str(), "PWM: %d", value);

    return true;
}

bool DomDomChannelClass::started()
{
    return _iniciado;
}

bool DomDomChannelClass::saveCurrentPWM()
{
    int address = getFirstEEPROMAddress();
    address += 2;

    EEPROM.writeUShort(address, _current_pwm);
    return EEPROM.commit();
}

uint16_t DomDomChannelClass::loadCurrentPWM()
{
    int address = getFirstEEPROMAddress();
    address += 2;

    uint16_t pwm = EEPROM.readUShort(address);
    return pwm;
}


bool DomDomChannelClass::saveConfig()
{
    int address = getFirstEEPROMAddress();

    EEPROM.write(address, (_channel_num+1));
    address += 1;
    EEPROM.writeBool(address, _enabled);
    address += 1;
    // hueco para el pwm
    address += 2;
    if (leds.size() > 0)
    {
        EEPROM.write(address, leds.size());
        address++;

        for (int i = 0; i < leds.size(); i++)
        {
            EEPROM.writeUShort(address, leds[i]->K);
            address += 2;
            EEPROM.writeUShort(address, leds[i]->nm);
            address += 2;
            EEPROM.writeUShort(address, leds[i]->W);
            address += 2;
            EEPROM.write(address, leds[i]->type);
        }
    }

    bool result = EEPROM.commit();

    if (result)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Guardado en EEPROM...OK!");
    }

    return result;
};

bool DomDomChannelClass::loadFromEEPROM()
{
    bool started = _iniciado;
    if (started)
    {
        end();
    }

    int address = getFirstEEPROMAddress();

    if (EEPROM.read(address) == (_channel_num+1))
    {
        address += 1;
        setEnabled(EEPROM.readBool(address));
        address += 1;
        
        // hueco del pwm
        address += 2;

        int leds_count = EEPROM.read(address);
        address++;

        leds.clear();
        for (int i=0; i < leds_count; i++)
        {
            DomDomChannelLed* led = new DomDomChannelLed();
            led->K = EEPROM.readUShort(address);
            address += 2;
            led->nm = EEPROM.readUShort(address);
            address += 2;
            led->W = EEPROM.readUShort(address);
            address += 2;
            led->type = (LedType)EEPROM.read(address);

            leds.push_back(led);
        }

        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Cargando configuracion desde EEPROM...OK!");
        
    } else {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "No se encontro informacion en la EEPROM");
        return false;
    }

    if (started)
    {
        int pins[CHANNEL_SIZE] = CHANNEL_PWM_PINS;
        int resolution[CHANNEL_SIZE] = CHANNEL_RESOLUTION;
        int ina[CHANNEL_SIZE] = CHANNEL_INA_ADDRESS;

        begin(ina[_channel_num], pins[_channel_num], resolution[_channel_num]);
    }

    return true;
}

int DomDomChannelClass::getFirstEEPROMAddress()
{
    return EEPROM_CHANNEL_FIRST_ADDRESS;
}