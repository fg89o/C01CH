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

const uint32_t SHUNT_MICRO_OHM      = 100000;  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS         = 3;       ///< Max expected amps, values are 1 - clamped to max 1022
const uint16_t INA_AVERAGING        = 64;
const uint16_t INA_CONVERSION_TIME  = 8244;

DomDomChannelClass::DomDomChannelClass(uint8_t INA_address = 0x40, uint8_t channel)
{
    _channel_num = channel;
    tag = String("CHANNEL ");
    tag.concat(_channel_num+1);

    _enabled = true;
    _iniciado = false;

    maximum_mA = 100.0f;
    minimum_mA = 0.0f;
    maximum_V = 0.0f;

    INA_device_index = UINT8_MAX;
    _INA_address = INA_address;
}

bool DomDomChannelClass::begin()
{
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Iniciando canal...");
    if (!_enabled)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "Canal no habilitado");
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, tag.c_str(), "Iniciando canal...ERROR!");
        return false;
    }

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
        this->limitCurrentTask, /* Task function. */
        "Current_Task",         /* String with name of task. */
        10000,                  /* Stack size in bytes. */
        NULL,                   /* Parameter passed as input of the task */
        1,                      /* Priority of the task. */
        NULL                    /* Task handle. */
    );

    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, tag.c_str(), "Iniciando canal...OK!");
    return true;
}

bool DomDomChannelClass::end()
{
    _iniciado = false;
    return true;
}


void DomDomChannelClass::limitCurrentTask(void *parameter)
{
    uint8_t max_dac_pwm = 255;
    uint8_t min_dac_pwm = 0;
    uint8_t curr_pwm = max_dac_pwm;

    int channels_pin[CHANNEL_SIZE] = CHANNEL_CURRENT_PIN;
    int channel = channels_pin[DomDomChannel.getNum()];

    dacWrite(channel, curr_pwm);

    float prev_maxV = -1;
    float prev_targetmA = -999;

    DomDomChannel.is_current_stable = false;
    int8_t pwm_dir = 0;

    while(DomDomChannel.started())
    {
        DomDomChannel.INA.waitForConversion(DomDomChannel.INA_device_index);
            
        // Si los voltios o los miliamperios objetivos varían una vez estabilizado volvemos a estabilizar
        if (DomDomChannel.maximum_V != prev_maxV || DomDomChannel.target_mA != prev_targetmA)
        {
            prev_maxV = DomDomChannel.maximum_V;
            prev_targetmA = DomDomChannel.target_mA;
            DomDomChannel.is_current_stable = false;
            pwm_dir = 0;
        } 

        float volts = DomDomChannel.INA.getBusMilliVolts(DomDomChannel.INA_device_index) / 1000.0f;
        float amps = DomDomChannel.INA.getBusMicroAmps(DomDomChannel.INA_device_index) / 1000.0f;
        float power = amps * volts;
        power = power < 0 ? 0 : power;

        DomDomChannel.lastBusCurrent_mA = amps;
        DomDomChannel.lastBusVoltaje_V = volts;

        // Guardamos los maximos
        DomDomChannel.busPowerPeak_W = DomDomChannel.busPowerPeak_W > power ? DomDomChannel.busPowerPeak_W : power;
        DomDomChannel.busCurrentPeak_mA = DomDomChannel.busCurrentPeak_mA > amps ? DomDomChannel.busCurrentPeak_mA : amps;
        DomDomChannel.busVoltagePeak_V = DomDomChannel.busVoltagePeak_V > volts ? DomDomChannel.busVoltagePeak_V : volts;

        
        // Calculamos la histeresis en funcion del valor maximo para la corriente
        // Establecemos un minimo de 5mA
        float mA_histeresis = (DomDomChannel.maximum_mA * 0.5);
        mA_histeresis = mA_histeresis < 5 ? 5 : mA_histeresis;
        // Comprobamos si la corriente esta en el rango establecido
        bool mAInRange = (amps < (DomDomChannel.target_mA + mA_histeresis) && amps > (DomDomChannel.target_mA - mA_histeresis));
        if (DomDomChannel.is_current_stable && !mAInRange)
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, DomDomChannel.tag.c_str(), "DAC fuera de rango! Se vuelve a calcular");
            DomDomChannel.is_current_stable = false;
            pwm_dir = 0;
        }

        if (!DomDomChannel.is_current_stable)
        {
            // Si no hay direccion establecida la establecemos ahora
            if (pwm_dir == 0)
            {
                pwm_dir = (amps < DomDomChannel.target_mA && volts < DomDomChannel.maximum_V) ? -1 : 1;
                DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, DomDomChannel.tag.c_str(), "Direccion establecida (%d)", pwm_dir);
            }

            // Si estamos aumentando la potencia (pwm_dir == -1), aumentamos hasta que el valor actual sea mayor que el objetivo mas histeresis
            if (pwm_dir == -1 && (amps <= DomDomChannel.target_mA && volts <= DomDomChannel.maximum_V) &&  curr_pwm > min_dac_pwm)
            {
                dacWrite(channel, --curr_pwm);
            }

            // Si estamos aumentando la potencia y nos hemos pasado usamos el valor antiguo
            if (pwm_dir == -1 && (amps > DomDomChannel.target_mA || volts > DomDomChannel.maximum_V || curr_pwm == min_dac_pwm))
            {
                DomDomChannel.is_current_stable = true;
                pwm_dir = 0;
                DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, DomDomChannel.tag.c_str(), "DAC Estabilizado (%d)", curr_pwm);
            }
            
            // Si estamos disminuyendo la potencia (pwm_dir == 1), disminuimos hasta que el valor actual sea menor que el objetivo menos histeresis
            if ((pwm_dir == 1) && (amps > DomDomChannel.target_mA || volts > DomDomChannel.maximum_V) && curr_pwm < max_dac_pwm)
            {
                dacWrite(channel, ++curr_pwm);
            }

            //Si estamos dismnuyendo la potencia y nos hemos pasado usamos el valor actual
            if ((pwm_dir == 1) && ((amps < DomDomChannel.target_mA && volts < DomDomChannel.maximum_V) || curr_pwm == max_dac_pwm))
            {
                DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, DomDomChannel.tag.c_str(), "DAC Estabilizado (%d)", curr_pwm);
                pwm_dir = 0;
                DomDomChannel.is_current_stable = true;
            }

            DomDomChannel.curr_dac_pwm = curr_pwm;
        }
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

bool DomDomChannelClass::setTargetmA(float value)
{
    if (!_enabled && value != 0)
    {
        return false;
    }

    if (value > maximum_mA)
    {
        value =  maximum_mA;
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, tag.c_str(), "El valor PWM es mayor que el maximo. Valor cambiado a %f", value);
    }

    if (value < minimum_mA)
    {
        value = minimum_mA;
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, tag.c_str(), "El valor PWM es mayor que el maximo. Valor cambiado a %f", value);
    }

    target_mA = value;

    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, tag.c_str(), "Target mA: %f", value);

    return true;
}

bool DomDomChannelClass::started()
{
    return _iniciado;
}

bool DomDomChannelClass::save()
{
    int address = getFirstEEPROMAddress();

    EEPROM.write(address, (_channel_num+1));
    address += 1;
    EEPROM.writeBool(address, _enabled);
    address += 1;
    EEPROM.writeUShort(address, _INA_address);
    address += 2;
    EEPROM.writeFloat(address, maximum_V);
    address += 4;
    EEPROM.writeFloat(address, maximum_mA);
    address += 4;
    EEPROM.writeFloat(address, minimum_mA);
    address += 4;
    EEPROM.writeFloat(address, target_mA);
    address += 4;

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
    bool started = DomDomChannel.started();
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
        _INA_address = EEPROM.readUShort(address);
        address += 2;
        maximum_V = EEPROM.readFloat(address);
        address += 4;
        maximum_mA = EEPROM.readFloat(address);
        address += 4;
        minimum_mA = EEPROM.readFloat(address);
        address += 4;
        target_mA = EEPROM.readFloat(address);
        address += 4;

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
        save();
        return false;
    }

    if (started)
    {
        begin();
    }

    return true;
}

int DomDomChannelClass::getFirstEEPROMAddress()
{
    return EEPROM_CHANNEL_FIRST_ADDRESS;
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomChannelClass DomDomChannel;
#endif