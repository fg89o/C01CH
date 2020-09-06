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

const uint32_t SHUNT_MICRO_OHM      = 100000;  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS         = 3;       ///< Max expected amps, values are 1 - clamped to max 1022
const uint16_t INA_AVERAGING        = 64;
const uint16_t INA_CONVERSION_TIME  = 8244;

DomDomChannelClass::DomDomChannelClass(uint8_t INA_address = 0x40)
{
    _channel_num = 0;
    _enabled = true;
    _iniciado = false;

    maximum_mA = 100.0f;
    minimum_mA = 0.0f;
    target_V = 0.0f;

    INA_device_index = UINT8_MAX;
    _INA_address = INA_address;
}

bool DomDomChannelClass::begin()
{
    if (!_enabled)
    {
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
                Serial.printf("[CHANNEL %d] Buscando INA . . . OK!\r\n", _channel_num);
                INA_device_index = i;
            }
        } 

        retry++;

    }while (INA_device_index == UINT8_MAX && retry < max_retries);

    if (INA_device_index == UINT8_MAX)
    {
        Serial.printf("[CHANNEL %d] Error! INA no encontrado.\r\n", _channel_num);
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

    float v_histeresis = 0.10;
    float mA_histeresis = 5;

    unsigned long prev_millis_info = 0;

    dacWrite(channel, curr_pwm);

    float prev_targetVolts = -1;
    float prev_targetmA = -1;
    DomDomChannel.is_current_stable = false;

    while(DomDomChannel.started())
    {
        if (DomDomChannel.INA.conversionFinished(0))
        {
            // Si los voltios  o los miliamperios objetivos varían una vez estabilizado volvemos a estabilizar
            if (DomDomChannel.target_V != prev_targetVolts || DomDomChannel.target_mA != prev_targetmA)
            {
                prev_targetVolts = DomDomChannel.target_V;
                prev_targetmA = DomDomChannel.target_mA;
                DomDomChannel.is_current_stable = false;
            } 

            float volts = DomDomChannel.INA.getBusMilliVolts(DomDomChannel.INA_device_index) / 1000.0f;
            float amps = DomDomChannel.INA.getBusMicroAmps(DomDomChannel.INA_device_index) / 1000.0f;
            
            // Si el objetivo es menor o igual  0 voltios quitamos la histeresis
            if (DomDomChannel.target_V <= 0)
            {
                v_histeresis = 0;
            }

            // si el objetivo es menor o igual 0 mA quitamos la histeresis
            if (DomDomChannel.target_mA <= 0)
            {
                mA_histeresis = 0;
            }
            
            bool voltsInRange = (volts < (DomDomChannel.target_V + v_histeresis) && volts > (DomDomChannel.target_V - v_histeresis));
            bool mAInRange = (amps < (DomDomChannel.target_mA + mA_histeresis) && amps > (DomDomChannel.target_mA - mA_histeresis));

            DomDomChannel.lastBusCurrent_mA = amps;
            DomDomChannel.lastBusVoltaje_V = volts;

            float power = amps * volts;
            power = power < 0 ? 0 : power;

            if (DomDomChannel.busPowerPeak_W < power)
            {
                DomDomChannel.busPowerPeak_W = power;   
            }

            if (DomDomChannel.busCurrentPeak_mA < amps)
            {
                DomDomChannel.busCurrentPeak_mA = amps;
            }

            if (DomDomChannel.busVoltagePeak_V < volts)
            {
                DomDomChannel.busVoltagePeak_V = volts;
            }

            if (voltsInRange && amps < DomDomChannel.target_mA + mA_histeresis)
            {
                DomDomChannel.is_current_stable = true;
                Serial.printf("[CHANNEL %d] Salida estabilizada por voltios\r\n", DomDomChannel.getNum());
            }
            else if (mAInRange && volts < DomDomChannel.target_V + v_histeresis )
            {
                DomDomChannel.is_current_stable = true;
                Serial.printf("[CHANNEL %d] Salida estabilizada por amperios\r\n", DomDomChannel.getNum());
            }

            if (!DomDomChannel.is_current_stable)
            {
                if ((amps > DomDomChannel.target_mA || volts > DomDomChannel.target_V) && curr_pwm < max_dac_pwm)
                {
                    dacWrite(channel, ++curr_pwm);
                }
                if ((amps < DomDomChannel.target_mA && volts < DomDomChannel.target_V) &&  curr_pwm > min_dac_pwm)
                {
                    dacWrite(channel, --curr_pwm);
                }

                DomDomChannel.curr_dac_pwm = curr_pwm;
            }

            if (millis() - prev_millis_info > CHANNEL_BUS_REFRESH_INTERVAL )
            {
                Serial.printf("Bus voltage max: %fV\r\n", DomDomChannel.target_V);
                Serial.printf("Bus miliamps max: %fmA\r\n", DomDomChannel.target_mA);
                Serial.printf("Bus voltage: %fV\r\n", volts);
                Serial.printf("Bus miliamps: %fmA\r\n", amps);
                Serial.printf("DAC: %d\r\n", curr_pwm);
                prev_millis_info = millis();
            }
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
        Serial.printf("[CHANNEL %d] ERROR: valor pwm mayor que el maximo. Valor cambiado a %f\r\n", _channel_num, value);
    }

    if (value < minimum_mA)
    {
        Serial.printf("[CHANNEL %d] WARN: el valor %f menor que el minimo (%f).\r\n", _channel_num, value, minimum_mA);
    }

    target_mA = value;
    is_current_stable = false;

    Serial.printf("[Channel %d] Target mA: %f\n", _channel_num, target_mA);
    
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
    EEPROM.writeFloat(address, target_V);
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
        Serial.printf("[Channel %d] Guardado en EEPROM\n", _channel_num);
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
        target_V = EEPROM.readFloat(address);
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

        Serial.printf("[CHANNEL %d] Configuracion cargada correctamente.\n", _channel_num);
        
    } else {
        Serial.printf("[CHANNEL %d] No se encontro informacion guardada del canal\n", _channel_num);
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