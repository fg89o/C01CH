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

#pragma once
#ifndef DOMDOM_LOGGER_h
#define DOMDOM_LOGGER_h

#include <stdio.h>
#include <Arduino.h>

class DomDomLoggerClass
{
    private:
        /**
         * Array con los mensajes en RAM
         */
        std::vector<String> log_RAM;
        /**
         * Metodo que crea una entrada en la RAM
         */
        void logToRAM(const char *tag, char *message);
        /**
         * Realiza una entrada del log en formato INFO
         */
        void log(int level, const char *tag, const char *format, va_list args);

    public:
        /**
         * Constructor
         */
        DomDomLoggerClass();
        /**
         * Indica si la salida se realizara por el puerto serie
         */
        bool output_serial_enabled = true;
        /**
         * Indica si la salida del log se realizara en RAM
         */
        bool output_ram_enabled = true;
        /**
         * Numero de entradas máximas para el log
         */
        int max_log_entries = 100;
        /**
         * Realiza una entrada del log en formato VERBOSE
         */
        void logV(String tag, String format, ...);
         /**
         * Realiza una entrada del log en formato DEBUG
         */
        void logD(String tag, String format, ...);
         /**
         * Realiza una entrada del log en formato INFO
         */
        void logI(String tag, String format, ...);
        /**
         * Realiza una entrada del log en formato ERROR
         */
        void logE(String tag, String format, ...);
        /**
         * Realiza una entrada del log en formato WARNING
         */
        void logW(String tag, String format, ...);
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomLoggerClass DomDomLogger;
#endif

#endif /* DOMDOM_LOGGER_h */