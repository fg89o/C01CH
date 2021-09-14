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
         * Numero del canal. Actua como id del canal.
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
         * Resolución del canal PWM
         */
        uint8_t _resolution;
        /**
         * Valor actual del pwm
         */
        uint16_t _current_pwm;
        /**
         * PWM Pin
         */
        uint8_t _pwm_pin;
        /**
         * Devuelve la primera direccion de memoria para este canal
         */
        int getFirstEEPROMAddress();
        /**
         * Tarea para mantener la limitacion de corriente en el canal
         */
        static void CurrentMonitorTask(void * parameter);

    public:
        /**
         * Constructor
         */
        DomDomChannelClass(uint8_t channel = 0);
        /**
         * Sensor de corriente
         */
        INA_Class INA;
        /**
         * Indice de dispositivo INA correspondiente a este canal
         */
        uint8_t INA_device_index;
        /**
         * Voltaje maximo detectado en el bus
         */
        float busVoltagePeak_V = 0;
        /**
         * Corriente máxima detectada
         */
        float busCurrentPeak_mA = 0;
        /**
         * Potencia máxima detectada
         */
        float busPowerPeak_W = 0;
        /**
         * Ultimo voltaje leido en el bus
         */
        float lastBusVoltaje_V = 0;
        /**
         * Ultima corriente leida en el bbus
         */
        float lastBusCurrent_mA = 0;
        /**
         * Indica si se esta controlando el canal
         */
        bool started();
        /**
         * Configuracion de leds para este canal
         */
        std::vector<DomDomChannelLed *> leds;
        /**
         * Devuelve el numero del canal en solo lectura.
         */
        uint8_t getNum() const { return _channel_num; }
        /**
         * Devuelve si el canal est habilitado o no, valor de solo lectura.
         */
        bool getEnabled() const {return _enabled; };
        /**
         * Devuelve el pwm actual
         */
        uint16_t getPWM() const { return _current_pwm; }
        /**
         * Devuelve el pwm actual
         */
        uint16_t getMinPWM() const { return 0; }
        /**
         * Devuelve el pwm actual
         */
        uint16_t getMaxPWM() const { return pow(2,_resolution); }
        /**
         * Devuelve la resolucion del pwm actual
         */
        uint16_t getPWMResolution() const { return _resolution; }
        /**
         * Configura el canal.
         */
        bool begin(uint8_t INA_address, uint8_t pwm_pin, uint8_t resolution = 12);
        /**
         * Quita la configuracion del canal.
         */
        bool end();
        /**
         * Establece el estado actual del canal.
         */
        bool setEnabled(bool enabled);
        /**
         * Guarda la configuracion actual del canal en memoria.
         */
        bool saveConfig();
        /**
         * Invalida la configuracion actual y carga la configuracion
         * almacenada en memoria para este canal.
         */
        bool loadFromEEPROM();
        /**
         * Devuelve el tag para el log de este canal
         */
        String tag;
        /**
         * Establece el pwm actual del canal
         */
        bool setPWM(uint16_t value, bool guardar = false);
        /**
         * Guarda el valor PWM actual en memoria.
         */
        bool saveCurrentPWM();
        /**
         * Carga el valor PWM en memoria.
         */
        uint16_t loadCurrentPWM();
};

#endif /* DOMDOM_CHANNEL_h */