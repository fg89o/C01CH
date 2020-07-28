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
const uint16_t INA_AVERAGING        = 32;
const uint16_t INA_CONVERSION_TIME  = 8244;

DomDomChannelClass::DomDomChannelClass()
{
    _channel_num = 0;
    _enabled = true;
    _iniciado = false;

    maximum_mA = 0;
    maximum_voltage = 0;

    xMutex = xSemaphoreCreateMutex();

    setPWMResolution(12);
}

bool DomDomChannelClass::begin()
{
    if (pwm_pin == 0 || !_enabled)
    {
        return false;
    }

    // configure LED PWM functionalitites
    ledcSetup(_channel_num, 5000, _resolution);
    
    // attach the channel to be controlled
    ledcAttachPin(pwm_pin, _channel_num);

    // Iniciamos el INA
    currentSensor._EEPROM_offset = EEPROM_SIZE;
    currentSensor._EEPROM_size = EEPROM_INA_SIZE;
    if (!currentSensor.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM))
    {
        Serial.printf("Error! INA no encontrado. El canal %d se deshabilitara.\r\n", _channel_num);
        return false;
    }
    
    currentSensor.setAveraging(INA_AVERAGING,0);                          // Average each reading n-times
    currentSensor.setBusConversion(INA_CONVERSION_TIME,0);                // Maximum conversion time 8.244ms
    currentSensor.setShuntConversion(INA_CONVERSION_TIME,0);              // Maximum conversion time 8.244ms
    currentSensor.setMode(INA_MODE_CONTINUOUS_BOTH,0);                    // Bus/shunt measured continuously

    _iniciado = true;

    // init PWM
    ledcWrite(_channel_num, _min_pwm); 

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
    ledcDetachPin(pwm_pin);
    _iniciado = false;

    return true;
}


void DomDomChannelClass::limitCurrentTask(void *parameter)
{
    uint8_t max_dac_pwm = 255;
    uint8_t min_dac_pwm = 0;
    uint8_t curr_pwm = max_dac_pwm;

    int channel = CHANNEL_CURRENT_PIN;

    float v_histeresis = 0.10;
    float mA_histeresis = 5;

    unsigned long prev_millis_info = 0;

    dacWrite(channel, curr_pwm);

    while(DomDomChannel.started())
    {
        if (DomDomChannel.currentSensor.conversionFinished(0))
        {
            DomDomChannel.waitForConversion();
            float volts = DomDomChannel.getBusVoltage_V();
            float amps = DomDomChannel.getCurrent_mA();

            bool voltsInRange = (volts < (DomDomChannel.maximum_voltage + v_histeresis) && volts > (DomDomChannel.maximum_voltage-v_histeresis));
            bool mAInRange = (amps < (DomDomChannel.maximum_mA + mA_histeresis) && amps > (DomDomChannel.maximum_mA-mA_histeresis));

            bool skip = false;

            if (voltsInRange && amps < DomDomChannel.maximum_mA + mA_histeresis)
            {
                skip = true;
            }
            else if (mAInRange && volts < DomDomChannel.maximum_voltage + v_histeresis )
            {
                skip = true;
            }

            if (!skip)
            {
                if ((amps > DomDomChannel.maximum_mA || volts > DomDomChannel.maximum_voltage) && curr_pwm < max_dac_pwm)
                {
                    dacWrite(channel, ++curr_pwm);
                }

                if ((amps < DomDomChannel.maximum_mA && volts < DomDomChannel.maximum_voltage) &&  curr_pwm > min_dac_pwm)
                {
                    dacWrite(channel, --curr_pwm);
                }

                DomDomChannel.curr_dac_pwm = curr_pwm;
            }

            if (millis() - prev_millis_info > CHANNEL_BUS_REFRESH_INTERVAL )
            {
                Serial.printf("Bus voltage max: %fV\r\n", DomDomChannel.maximum_voltage);
                Serial.printf("Bus miliamps max: %fmA\r\n", DomDomChannel.maximum_mA);
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

bool DomDomChannelClass::setPWMValue(uint16_t value)
{
    if (!_iniciado || (!_enabled && value != 0))
    {
        return false;
    }

    if (value > max_limit_pwm)
    {
        value =  max_limit_pwm;
        Serial.printf("ERROR: valor pwm mayor que el maximo. Valor cambiado a %d\n", value);
    }

    if (value < min_limit_pwm)
    {
        value = min_limit_pwm;
        Serial.printf("ERROR: valor pwm menor que el minimo. Valor cambiado a %d\n", value);
    }

    ledcWrite(_channel_num, value);

    _current_pwm = value;

    Serial.printf("[Channel %d] PWM: %d\n", _channel_num, _current_pwm);
    
    return true;
}

bool DomDomChannelClass::setPWMResolution(uint8_t value)
{
    if (value != 8 && value != 10 && value != 12 && value != 15)
    {
        Serial.printf("ERROR: El valor de resolucion no se encuentra entre los permitidos (%d)\n",value);
        return false;
    }

    _resolution = value;
    _min_pwm = 0;
    _max_pwn = (int)pow(2,value);
    max_limit_pwm = _max_pwn;
    min_limit_pwm = _min_pwm;

    ledcSetup(_channel_num, 5000, _resolution);

    return true;
}

bool DomDomChannelClass::saveCurrentPWM()
{
    int address = getFirstEEPROMAddress();
    address += 14;

    EEPROM.writeUShort(address, _current_pwm);
    return EEPROM.commit();
}

bool DomDomChannelClass::started()
{
    return _iniciado;
}

void DomDomChannelClass::waitForConversion()
{
    currentSensor.waitForConversion(0);
}

float DomDomChannelClass::getBusVoltage_V()
{
    xSemaphoreTake( xMutex, portMAX_DELAY );
    // unsigned long prev_millis = millis();
    // uint32_t interval = INA_AVERAGING * INA_CONVERSION_TIME / 1000;
    float v = currentSensor.getBusMilliVolts(0) / 1000.0;
    // while (millis() - prev_millis < interval )
    // {
    //     delay(1);
    // }
    xSemaphoreGive( xMutex );
    return v; 
}

float DomDomChannelClass::getCurrent_mA()
{
    xSemaphoreTake( xMutex, portMAX_DELAY );
    // unsigned long prev_millis = millis();
    // uint32_t interval = INA_AVERAGING * INA_CONVERSION_TIME / 1000;
    float mA = currentSensor.getBusMicroAmps(0) / 1000.0;
    // while (millis() - prev_millis < interval )
    // {
    //     delay(1);
    // }
    xSemaphoreGive( xMutex );
    return mA; 
}

bool DomDomChannelClass::save()
{
    int address = getFirstEEPROMAddress();

    EEPROM.write(address, (_channel_num+1));
    address += 1;
    EEPROM.writeBool(address, _enabled);
    address += 1;
    EEPROM.writeUShort(address, max_limit_pwm);
    address += 2;
    EEPROM.writeUShort(address, min_limit_pwm);
    address += 2;
    EEPROM.writeFloat(address, maximum_mA);
    address += 4;
    EEPROM.writeFloat(address, maximum_voltage);
    address += 4;
    EEPROM.writeUShort(address, _current_pwm);
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
        Serial.printf("[Channel %d] Guardado en EEPROM\n", _channel_num);
    }

    return result;
};

bool DomDomChannelClass::loadFromEEPROM()
{
    int address = getFirstEEPROMAddress();

    if (EEPROM.read(address) == (_channel_num+1))
    {
        address += 1;
        setEnabled(EEPROM.readBool(address));
        address += 1;
        max_limit_pwm = EEPROM.readUShort(address);
        if (max_limit_pwm > _max_pwn)
        {
            max_limit_pwm = _max_pwn;
        }
        address += 2;
        min_limit_pwm = EEPROM.readUShort(address);
        address += 2;
        maximum_mA = EEPROM.readFloat(address);
        address += 4;
        maximum_voltage = EEPROM.readFloat(address);
        address += 4;
        _current_pwm = EEPROM.readUShort(address);
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

        Serial.printf("[CHANNEL %d] Configuracion cargada correctamente.\n", _channel_num);
        
    } else {
        Serial.printf("[CHANNEL %d] No se encontro informacion guardada del canal\n", _channel_num);
        save();
        return false;
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