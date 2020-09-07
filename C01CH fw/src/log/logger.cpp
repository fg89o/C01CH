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

DomDomLoggerClass::DomDomLoggerClass(){}

void DomDomLoggerClass::logToRAM(const char *tag, char *message)
{
    sprintf(message, "[%s]\t%s", tag, message);
    if (log_RAM.size() >= max_log_entries)
    {
        log_RAM.pop_back();
    }

    log_RAM.push_back(message);
}

void DomDomLoggerClass::log(int level, const char *tag, const char *format, va_list args)
{
    char *message = nullptr;
    sprintf(message, format, args);

    switch (level)
    {
        case 1:
            sprintf(message, "[%s]\t[ERROR]\t%s", tag, message);
            break;  
        case 2:
            sprintf(message, "[%s]\t[WARN]\t%s", tag, message);
            break;   
        case 3:
            sprintf(message, "[%s]\t[INFO]\t%s", tag, message);
            break;   
        case 4:
            sprintf(message, "[%s]\t[DEBUG]\t%s", tag, message);
            break;   
        case 5:
            sprintf(message, "[%s]\t[VERBOSE]\t%s", tag, message);
            break;   
    }

    if (output_serial_enabled) Serial.print(message);
    if (output_ram_enabled) logToRAM(tag,message);
}


void DomDomLoggerClass::logV(String tag, String format, ...)
{
    va_list args;
    va_start(args, format);
    log(5,tag.c_str(), format.c_str(), args);
    va_end(args);
}

void DomDomLoggerClass::logD(String tag, String format, ...)
{
    va_list args;
    va_start(args, format);
    log(4,tag.c_str(), format.c_str(), args);
    va_end(args);
}

void DomDomLoggerClass::logI(String tag,String format, ...)
{
    va_list args;
    va_start(args, format);
    log(3,tag.c_str(), format.c_str(), args);
    va_end(args);
}

void DomDomLoggerClass::logW(String tag, String format, ...)
{
    va_list args;
    va_start(args, format);
    log(2,tag.c_str(), format.c_str(), args);
    va_end(args);
}

void DomDomLoggerClass::logE(String tag, String format, ...)
{
    va_list args;
    va_start(args, format);
    log(1,tag.c_str(), format.c_str(), args);
    va_end(args);
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomLoggerClass DomDomLogger;
#endif