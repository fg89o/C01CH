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

#include <stdarg.h>
#include "logger.h"
#include "esp_log.h"
#include "../lib/RTCLib/RTClib.h"

DomDomLoggerClass::DomDomLoggerClass(){}

void serial_print(DomDomLoggerClass::LogEntry entry)
{
    String outputstr;

    DateTime dt (millis());
    outputstr.concat(dt.hour());
    outputstr.concat(":");
    outputstr.concat(dt.minute());
    outputstr.concat(":");
    outputstr.concat(dt.second());

    switch (entry.level)
    {
        case DomDomLoggerClass::LogLevel::debug :
            outputstr.concat("\t[DEBUG]");
            break;
        case DomDomLoggerClass::LogLevel::info :
            outputstr.concat("\t[INFO]");
            break;
        case DomDomLoggerClass::LogLevel::warn :
            outputstr.concat("\t[WARN]");
            break;
        case DomDomLoggerClass::LogLevel::error :
            outputstr.concat("\t[ERROR]");
            break;
        default:
            break;
    }

    outputstr.concat("\t[");
    outputstr.concat(entry.tag);
    outputstr.concat("]");

    outputstr.concat("\t" + entry.message);

    Serial.println(outputstr);
}

void DomDomLoggerClass::log(DomDomLoggerClass::LogLevel level, const char *tag, const char *format, ...)
{
    va_list argp;
    DomDomLoggerClass::LogEntry entry;

    entry.time = millis();
    entry.level = level;
    entry.tag = tag;

    va_start(argp, format);
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == '%') {
                entry.message.concat('%');
            } else if (*format == 'c') {
                unsigned char char_to_print = va_arg(argp, int);
                entry.message.concat(char_to_print);
            } else if (*format == 'd') {
                int char_to_print = va_arg(argp, int);
                entry.message.concat(char_to_print);
            } else if (*format == 's') {
                char *char_to_print = va_arg(argp, char *);
                entry.message.concat(char_to_print);
            } else if (*format == 'f') {
                double value = va_arg(argp, double);
                entry.message.concat(value);
            } else {
                entry.message.concat("[Not implemented]");
            }
        } else {
            entry.message.concat(*format);
        }
        format++;
    }
    va_end(argp);

    if (output_ram_enabled)
    {
        if (log_RAM.size() > max_log_entries)
        {
            log_RAM.pop_back();
        }

        log_RAM.push_back(entry);
    }
    
    if (output_serial_enabled)
    {
        serial_print(entry);
    }
    
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomLoggerClass DomDomLogger;
#endif