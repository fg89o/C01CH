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
#include "../INA/INA.h"

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
         * Valor actual del PWM en el pin de salida.
         */
        uint16_t _current_pwm;
        /**
         * Numero del canal. Actual como id del canal.
         */
        uint8_t _channel_num;
        /**
         * Resolucion del PWM.
         */
        uint8_t _resolution;
        /**
         * Valor minimo para el PWM.
         * 
         * Este valor nunca deberia ser inferior a 0
         * ni superior al valor maximo.
         */
        uint16_t _min_pwm;
        /**
         * Valor maximo para el PWM.
         * 
         * Este valor nunca deberia ser superior a
         * pow(2, resolucion) ni inferior al valor minimo.
         */
        uint16_t _max_pwn;
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
         *  create Mutex 
         * */
        SemaphoreHandle_t  xMutex;
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
         * Constructor.
         */
        DomDomChannelClass();
        /**
         * Constructor con mas parametros.
         */
        DomDomChannelClass(char pwm_pin, char resolution);
        /**
         * Sensor de corriente
         */
        INA_Class currentSensor;
        /**
         * Indica si se esta controlando el canal
         */
        bool started();
        /**
         * Configuracion de leds para este canal
         */
        std::vector<DomDomChannelLed *> leds;
        /**
         * Valor maximo para el PWM configurado por el usuario.
         */
        uint16_t max_limit_pwm;
        /**
         * Valor minimo para el PWM configurado por el usuario.
         */
        uint16_t min_limit_pwm;
        /**
         * Limite máximo para miliamperios en este canal
         */
        float maximum_mA;
        /**
         * Limite máximo para el voltaje
         */
        float maximum_voltage;
        /**
         * Pin de salida de este canal
         */
        uint8_t pwm_pin;
        /**
         * PWM actual para la salida DAC
         */
        uint8_t curr_dac_pwm;
        /**
         * Devuelve el valor PWM actual en solo lectura.
         */
        uint16_t current_pwm() const {return _current_pwm; }
        /**
         * Devuelve el numero del canal en solo lectura.
         */
        uint8_t getNum() const { return _channel_num; }
        /**
         * Devuelve la resolucion del PWM en solo lectura.
         */
        uint8_t getResolution() const { return _resolution; }
        /**
         * Devuelve si el canal est habilitado o no, valor de solo lectura.
         */
        bool getEnabled() const {return _enabled; };
        /**
         * Devuelve el voltaje actual en el bus
         */
        const float getBusVoltage_V();
        /**
         * Devuelve el consumo actual del bus en mA
         */
        const float getCurrent_mA();
        /**
         * Configura el canal.
         */
        bool begin();
        /**
         * Quita la configuracion del canal.
         */
        bool end();
        /**
         * Establece un nuevo valor para el PWM.
         */
        bool setPWMValue(uint16_t value);
        /**
         * Establece una nueva resolucion PWM.
         */
        bool setPWMResolution(uint8_t value);
        /**
         * Establece el estado actual del canal.
         */
        bool setEnabled(bool enabled);
        /**
         * Guarda la configuracion actual del canal en memoria.
         */
        bool save();
        /**
         * Invalida la configuracion actual y carga la configuracion
         * almacenada en memoria para este canal.
         */
        bool loadFromEEPROM();
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomChannelClass DomDomChannel;
#endif

#endif /* DOMDOM_CHANNEL_h */