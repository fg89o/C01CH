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
#ifndef DOMDOM_STATUSLEDCONTROL_h
#define DOMDOM_STATUSLEDCONTROL_h

#include <Arduino.h>
#include "../configuration.h"

/**
 * Clase para el control del led
 * 
 * Esta clase sera la encargada de controlar el led que nos 
 * marcará el estado del equipo.
 */
class DomDomStatusLedControlClass
{
    private:
        /**
         * Varianle para controlar si actualmente esta parpadeando el led
         */
        bool blinking;
        /**
         * Nivel PWM para cuando lo encendemos
         */
        int max_pwm;
        /**
         * nivel PWM para cuando lo apagamos
         */
        int min_pwm;
        /**
         * Modo de la conexion Wifi.
         */
        int wifi_mode;
        /**
         * Mutex para evitar que varias tareas controlen el led a la vez
         */
        SemaphoreHandle_t xMutex;
        /**
         * Actualiza la hora por NTP.
         */
        static void blinkTask(void * parameter);

    public:
        /**
         * Constructor.
         * */
        DomDomStatusLedControlClass();
        /**
         * Retraso entre el parpadeo
         */
        int blinkDelay = LED_STATUS_BLINK_DELAY;
        /**
         * Configura el led de status y empieza a monitorizar el
         * estado para variar el led cuando proceda.
         */
        void begin();
        /**
         * Realiza un parpadeo del led activo.
         */
        void blink(int times);
        /**
         * Toma el control del led
         */
        void block();
        /**
         * Libera el led
         */
        void release();
        /**
         * Enciende el led
         */
        void on();
        /**
         * Apaga el led
         */
        void off();
        /**
         * Devuelve un bool indicando si el led esta parpadeando
         */
        bool isBlinking();
        /**
         * Devuelve en tiempo entre parpadeos
         */
        bool getDelay();
        /**
         * Inicia un parpadeo infinito del led
         */
        void blinkInfinite();
};


#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomStatusLedControlClass DomDomStatusLedControl;
#endif

#endif /* DOMDOM_SCHEDULEPOINT_h */