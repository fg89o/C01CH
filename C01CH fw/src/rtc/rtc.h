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

#ifndef DOMDOMRTCCLASS_h
#define DOMDOMRTCCLASS_h

#include <Arduino.h>
#include "../lib/RTCLib/RTClib.h"
#include "../lib/NTPClient/NTPClient.h"
#include <WiFiUdp.h>
#include "../configuration.h"

/**
 * Clase encargada de la gestion de la hora.
 * 
 * Al iniciar esta clase se cogera la fecha/hora
 * del modulo externo y se ajustará el RTC interno.
 * En caso de estar habilitado el servicio NTP lo
 * configura y activa. 
 */
class DomDomRTCClass
{
    private:
        /**
         * Indica si el servicio NTP esta activo o no.
         */
        bool _ntpStarted = false;
        /**
         * Puntero a la tarea del NTP
         */
        TaskHandle_t *_ntp_task;
        /**
         * Actualiza la hora por NTP.
         */
        static void NTPTask(void * parameter);
        /**
         * Servidor NTP al que nos conectaremos
         */
        String _ntpServerName = NTP_SERVERNAME;
        /**
         * Formato Posix de la zona horaria
         */
        String _ntpPosixZone = NTP_POSIX_TIMEZONE;
        /**
         * Nombre  de la zona horaria.
         */
        String _ntpTimezone = NTP_TIMEZONE;
        /**
         * Clase UDP para el servicio NTP.
         */
        WiFiUDP _ntpUDP;
        /**
         * Cliente NTP para la actualizacion de la hora por internet
         */
        NTPClient *timeClient;

    public:
        /**
         * Constructor
         */
        DomDomRTCClass();
        /**
         * Indica si la clase se inicio correctamente
         */
        bool ready;
        /**
         * Marca de tiempo con la ultima petición
         */
        unsigned long LastNTPCheck;
        /**
         * Ajusta el RTC interno y externo con la nueva fecha y hora
         */
        void adjust(time_t dt);
        /**
         * Devuelve la fecha y hora actual
         */
        DateTime now();
        /**
         * Inicia el proceso completo para la gestion de la hora
         */
        bool begin();
        /**
         * Inicia el servicio NTP.
         */
        void beginNTP();
        /**
         * Para el servicio NTP.
         */
        void endNTP();
        /**
         * Realiza una peticion de fecha y hora al servidor NTP.
         */
        bool updateFromNTP();
        /**
         * Cambia el servidor para el servicio NTP
         */
        void setNTPServername(String value);
        /**
         * Cambia la zona horaria a utilizar
         */
        void setNTPtimezone(String name, String posix);
        /**
         * Indica si el NTP esta en ejecucion
         */
        bool NTPStarted() const { return _ntpStarted; };
        /**
         * Devuelve la zona horaria en formato POSIX
         */
        String NTPTimezone() const { return _ntpTimezone; };
        /**
         * Devuelve la zona horaria en formato POSIX
         */
        String NTPPosixZone() const { return _ntpPosixZone; };
        /**
         * Devuelve el servidor del NTP
         */
        String NTPServername() const { return _ntpServerName; };
        /**
         * Guarda los datos actuales en memoria
         */
        bool save();
        /**
         * Carga los datos disponibles en memoria
         */
        bool load();
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomRTCClass DomDomRTC;
#endif

#endif /* DOMDOMRTCCLASS_h */