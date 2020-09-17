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
#include <stdarg.h>
#include <Arduino.h>



class DomDomLoggerClass
{
    public:
        /**
         * Enumerado para los distintos tipos de nivel de alerta
         */
        enum LogLevel
        {
            debug = 1,
            info = 2,
            warn = 4,
            error = 8
        };

        /**
         * Estructura para las entradas de log
         */
        struct LogEntry 
        {
            String                          message;
            DomDomLoggerClass::LogLevel     level;
            long int                        time;
            const char*                     tag;
        };

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
        int max_log_entries = 500;
        /**
         * Crea una nueva entrada
         */
        void log(DomDomLoggerClass::LogLevel level, const char *tag, const char *format, ...);
        /**
         * Array con los mensajes en RAM
         */
        std::vector<DomDomLoggerClass::LogEntry> log_RAM;
};

#if !defined(NO_GLOBAL_INSTANCES)
extern DomDomLoggerClass DomDomLogger;
#endif

#endif /* DOMDOM_LOGGER_h */