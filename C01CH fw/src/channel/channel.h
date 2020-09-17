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

#pragma once
#ifndef DOMDOM_CHANNEL_h
#define DOMDOM_CHANNEL_h

#include <Arduino.h>
#include "channelLed.h"
#include "../../lib/INA/INA.h"

/**
 * Representa un canal.
 * 
 * A traves de esta clase tendremos accesso a la configuracion
 * del canal y los metodos para poder controlar
 * su estado, configuracion, luminosidad, etc.
 */
class DomDomChannelClass
{
    private:
        /**
         * Numero del canal. Actual como id del canal.
         */
        uint8_t _channel_num;
        /**
         * Indica si este canal esta habilitado para su uso.
         * Si no esta habilitado el valor PWM sera 0.
         */
        bool _enabled;
        /**
         * Indica si el canal se esta controlando
         */
        bool _iniciado;
        /**
         * Direccion I2C del dispositivo INA asociado a este canal
         */
        uint8_t _INA_address;
        /**
         * Guarda el valor PWM actual en memoria.
         */
        bool saveCurrentPWM();
        /**
         * Devuelve la primera direccion de memoria para este canal
         */
        int getFirstEEPROMAddress();
        /**
         * Tarea para mantener la limitacion de corriente en el canal
         */
        static void limitCurrentTask(void * parameter);

    public:
        /**
         * Constructor
         */
        DomDomChannelClass(uint8_t INA_address, uint8_t channel = 0);
        /**
         * Sensor de corriente
         */
        INA_Class INA;
        /**
         * Indice de dispositivo INA correspondiente a este canal
         */
        uint8_t INA_device_index;
        /**
         * Ultimo voltaje leido en el bus
         */
        float lastBusVoltaje_V = 0;
        /**
         * Voltaje maximo detectado en el bus
         */
        float busVoltagePeak_V = 0;
        /**
         * Ultima corriente leida en el bbus
         */
        float lastBusCurrent_mA = 0;
        /**
         * Corriente máxima detectada
         */
        float busCurrentPeak_mA = 0;
        /**
         * Potencia máxima detectada
         */
        float busPowerPeak_W = 0;
        /**
         * Indica si se esta controlando el canal
         */
        bool started();
        /**
         * Configuracion de leds para este canal
         */
        std::vector<DomDomChannelLed *> leds;
        /**
         * Limite máximo para miliamperios en este canal
         */
        float maximum_mA;
        /**
         * Límite mínimo para los miliamperios en este canal
         */
        float minimum_mA;
        /**
         * Voltaje maximo para el canal
         */
        float maximum_V;
        /**
         * Corriente objetivo
         */
        float target_mA;
        /**
         * Pin de salida de este canal
         */
        uint8_t dac_pwm_pin;
        /**
         * PWM actual para la salida DAC
         */
        uint8_t curr_dac_pwm;
        /**
         * Indica si la corriente de salida se encuentra estable
         */
        bool is_current_stable;
        /**
         * Devuelve el numero del canal en solo lectura.
         */
        uint8_t getNum() const { return _channel_num; }
        /**
         * Devuelve si el canal est habilitado o no, valor de solo lectura.
         */
        bool getEnabled() const {return _enabled; };
        /**
         * Configura el canal.
         */
        bool begin();
        /**
         * Quita la configuracion del canal.
         */
        bool end();
        /**
         * Establece el estado actual del canal.
         */
        bool setEnabled(bool enabled);
        /**
         * Establece los mA objetivo
         */
        bool setTargetmA(float value);
        /**
         * Guarda la configuracion actual del canal en memoria.
         */
        bool save();
        /**
         * Invalida la configuracion actual y carga la configuracion
         * almacenada en memoria para este canal.
         */
        bool loadFromEEPROM();
        /**
         * Devuelve el tag para el log de este canal
         */
        String tag;
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomChannelClass DomDomChannel;
#endif

#endif /* DOMDOM_CHANNEL_h */