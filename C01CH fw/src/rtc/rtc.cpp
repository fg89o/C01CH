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

#include "rtc.h"
#include "time.h"
#include "sys/time.h"
#include <EEPROM.h>
#include <Wire.h>
#include "../wifi/WiFi.h"
#include "configuration.h"
#include "../log/logger.h"

//#include "zones.h"

DomDomRTCClass::DomDomRTCClass(){
    _ntpStarted = false;
}

bool DomDomRTCClass::begin()
{
    if (!_ntpStarted)
    {
        ready = false;
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "RTC", "Inicializando RTC...");
    
        timeClient = new NTPClient(_ntpUDP, _ntpServerName.c_str(), 0, 0);
        timeClient->begin();

        if (DomDomWifi.getMode() == WIFI_MODE_STA)
        {
            ready = updateFromNTP();
            beginNTP();
        }else
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "RTC", "Conexión WIFI no disponible");
        }
        
        if (ready)
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "RTC", "Inicializando RTC...OK!");
        }else
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "RTC", "Inicializando RTC...ERROR!");
        }
    }

    return ready;
}

bool DomDomRTCClass::updateFromNTP()
{
    bool result = false;
    LastNTPCheck = millis();
    if (DomDomWifi.getMode() == WIFI_MODE_STA && DomDomWifi.isConnected())
    {
        result = timeClient->forceUpdate();
        if (result)
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, "RTC", "NTP recibido: %d", timeClient->getEpochTime());
            adjust(timeClient->getEpochTime());
        }
    }
    
    return result;
}

void DomDomRTCClass::beginNTP()
{
    if (!_ntpStarted)
    {
        _ntpStarted = true;

        xTaskCreate(
            this->NTPTask,        /* Task function. */
            "NTP_Task",             /* String with name of task. */
            10000,                  /* Stack size in bytes. */
            NULL,                   /* Parameter passed as input of the task */
            1,                      /* Priority of the task. */
            this->_ntp_task         /* Task handle. */
        );
    }
}

void DomDomRTCClass::endNTP()
{
    if (_ntpStarted)
    {
        _ntpStarted = false;
        timeClient->end();
    }
}

void DomDomRTCClass::NTPTask(void * parameter)
{
    while(DomDomRTC.NTPStarted())
    {
        if (DomDomRTC.updateFromNTP())
        {
            vTaskDelay(NTP_DELAY_ON_SUCCESS / portTICK_PERIOD_MS);
        }
        else
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::error,"RTC", "No se recibio respuesta del NTP");
            vTaskDelay(NTP_DELAY_ON_FAILURE / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete(NULL);
}

DateTime DomDomRTCClass::now()
{
    time_t now;
    time(&now);
    
    DateTime dt (now);
    return dt;
}

void DomDomRTCClass::adjust(time_t dt)
{
    // Reloj interno
    tm timeinfo;
    const char* zone = _ntpPosixZone.c_str();
    setenv("TZ", zone, 1);
    tzset();

    localtime_r(&dt, &timeinfo);

    DateTime nDate = DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );

    timeval epoch;
    epoch.tv_sec = nDate.unixtime();
    epoch.tv_usec = 0;

    settimeofday((const timeval*)&epoch, 0);

    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug,"RTC", "RTC interno ajustado a %s",nDate.timestamp().c_str());
}

bool DomDomRTCClass::save()
{
    EEPROM.writeString(EEPROM_NTP_SERVERNAME_ADDRESS, _ntpServerName);
    EEPROM.writeString(EEPROM_NTP_TIMEZONENAME_ADDRESS, _ntpTimezone);
    EEPROM.writeString(EEPROM_NTP_TIMEZONEPOSIX_ADDRESS, _ntpPosixZone);

    return EEPROM.commit();
}

bool DomDomRTCClass::load()
{
    setNTPServername(EEPROM.readString(EEPROM_NTP_SERVERNAME_ADDRESS));
    setNTPtimezone(
        EEPROM.readString(EEPROM_NTP_TIMEZONENAME_ADDRESS),
        EEPROM.readString(EEPROM_NTP_TIMEZONEPOSIX_ADDRESS)
    );

    return true;
}

void DomDomRTCClass::setNTPServername(String value)
{
    if (value.length() > EEPROM_NTP_SERVERNAME_ADDRESS)
    {
        Serial.printf("[ERROR] Longitud maxima superada para el nombre del servidor NTP");
        return;
    }

    _ntpServerName = value;
}

void DomDomRTCClass::setNTPtimezone(String timezone, String posix)
{
    if (timezone.length() > EEPROM_NTP_TIMEZONENAME_LENGTH)
    {
        Serial.printf("[ERROR] Longitud maxima superada para la zona horaria");
        return;
    }

    if (posix.length() > EEPROM_NTP_TIMEZONEPOSIX_LENGTH)
    {
        Serial.printf("[ERROR] Longitud maxima superada para el formato POSIX de la zona horaria");
        return;
    }

    _ntpTimezone = timezone;
    _ntpPosixZone = posix;
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomRTCClass DomDomRTC;
#endif