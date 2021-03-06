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
#ifndef DOMDOM_WEBSERVERCLASS_h
#define DOMDOM_WEBSERVERCLASS_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define WEBSERVER_HTTP_PORT 80

class DomDomWebServerClass
{
    private:
        /**
         * Instacia del servidor web
         */
        AsyncWebServer *_server;

    public:
        /**
         * Constructor
         */
        DomDomWebServerClass();
        /**
         * Inicia el servidor web.
         */
        void begin();
        /**
         * Devuelve un JSON con la informacion RTC del equipo.
         */
        static void getRTCData(AsyncWebServerRequest *request);
        /**
         * Acepta un JSON con la nueva informacion RTC.
         */
        static void setRTCData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Devuelve un JSON con la informacion de red.
         */
        static void getWifiData(AsyncWebServerRequest *request);
        /**
         * Acepta un JSON con la nueva configuracion de red.
         */
        static void setWifiData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Devuelve un JSON con la informacion de los canales.
         */
        static void getChannelsData(AsyncWebServerRequest *request);
        /**
         * Acepta un JSON con la nueva configuracion de los canales.
         */
        static void setChannelsData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Acepta un JSON que con la estructura correcta provoca un reinicio en equipo.
         */
        static void setRestart(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Acepta un JSON que con la estructura correcta provoca que la EEPROM se reestablezca
         */
        static void setFactorySettings(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Devuelve un JSON con la programacion
         */
        static void getSchedule(AsyncWebServerRequest *request);
        /**
         * Acepta un JSON para configurar un punto de programacion
         */
        static void setSchedule(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Acepta un JSON para configurar un test de color
         */
        static void setTest(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Devuelve un JSON con la configuracion para el ventilador
         */
        static void getFanSettings(AsyncWebServerRequest *request);
        /**
         * Acepta un JSON para configurar el ventilador
         */
        static void setFanSettings(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Acepta un JSON que con la estructura correcta provoca un reinicio en equipo.
         */
        static void setResetMaxValues(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
        /**
         * Devuelve un JSON con la informacion de los canales.
         */
        static void getLog(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total);
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomWebServerClass DomDomWebServer;
#endif

#endif /* DOMDOM_WEBSERVERCLASS_h */