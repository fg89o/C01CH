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

#include "ScheduleMgt.h"
#include <EEPROM.h>
#include "configuration.h"
#include "channel.h"
#include "../log/logger.h"

DomDomScheduleMgtClass::DomDomScheduleMgtClass(/* args */)
{
}

DomDomScheduleMgtClass::~DomDomScheduleMgtClass()
{
}

int roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = abs(numToRound) % multiple;
    if (remainder == 0)
        return numToRound;

    if (numToRound < 0)
        return -(abs(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

bool DomDomScheduleMgtClass::getShedulePoint(DateTime &dt, DomDomSchedulePoint *&point, bool previous)
{
    DateTime now = DomDomRTC.now();
    now = now - TimeSpan(now.second());

    DateTime ultimaHora;
    int puntoAnterior = -1;

    for(int i = 0; i < schedulePoints.size(); i++)
    {
        // Buscamos la hora mayor del dia anterior;
        DateTime scheduleDT (now.year(), now.month(), now.day(), schedulePoints[i]->hour, schedulePoints[i]->minute, 0);
        TimeSpan day (1,0,0,0);

        // Si buscamos la fecha anterior y no es menor que la fecha actual, restamos un dia
        if (previous && scheduleDT >= now)
        {
            scheduleDT = scheduleDT - day;
        }

        // Si buscamos la fecha siiguiente y es menor que la actual sumamos un dia
        if (!previous && scheduleDT < now)
        {
            scheduleDT = scheduleDT + day;
        }

        // Si buscamos la fecha anterior
        if (previous)
        {
            if (scheduleDT < now)
            {
                if (puntoAnterior == -1)
                {
                    ultimaHora = scheduleDT;
                    puntoAnterior = i;
                }
                else if (scheduleDT > ultimaHora)
                {
                    ultimaHora = scheduleDT;
                    puntoAnterior = i;
                }
            }
        }

        // Si buscamos la fecha siguiente
        if (!previous)
        {
            if (scheduleDT >= now)
            {
                if (puntoAnterior == -1)
                {
                    ultimaHora = scheduleDT;
                    puntoAnterior = i;
                }
                else if (scheduleDT < ultimaHora)
                {
                    ultimaHora = scheduleDT;
                    puntoAnterior = i;
                }
            }
        }
        
    }

    if (puntoAnterior == -1)
    {
        return false;
    }

    dt = ultimaHora;
    point = schedulePoints[puntoAnterior];

    return true;
}

bool DomDomScheduleMgtClass::save()
{
    int address = EEPROM_SCHEDULE_FIRST_ADDRESS ;
    EEPROM.write(address++, (uint8_t)schedulePoints.size());
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Guardando programacion...");

    for (int i = 0; i < schedulePoints.size(); i++)
    {
        EEPROM.write(address++, schedulePoints[i]->dayOfWeek);
        EEPROM.write(address++, schedulePoints[i]->hour);
        EEPROM.write(address++, schedulePoints[i]->minute);
        EEPROM.write(address++, schedulePoints[i]->fade);
        EEPROM.write(address++, schedulePoints[i]->value);
    };
    
    EEPROM.writeBool(EEPROM_SCHEDULE_STATUS_ADDRESS, _started);

    bool result = EEPROM.commit();

    if (result)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Guardando programacion...OK!");
    }
    else
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error,"SCHEDULE", "Guardando programacion...ERROR!");
    }
        
    return result;
}

bool DomDomScheduleMgtClass::load()
{
    schedulePoints.clear();

    DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Cargando programacion...");

    int address = EEPROM_SCHEDULE_FIRST_ADDRESS ;
    uint8_t count = EEPROM.read(address++);

    if (count > 0 && count <= EEPROM_MAX_SCHEDULE_POINTS)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::debug,"SCHEDULE", "Encontrados %d puntos\n", count);
        for (int i = 0; i < count; i++)
        {
            
            DomDomDayOfWeek day = (DomDomDayOfWeek)EEPROM.read(address++);
            uint8_t hour = EEPROM.read(address++);
            uint8_t minute = EEPROM.read(address++);
            bool fade = EEPROM.read(address++);
            uint8_t value = EEPROM.read(address++);

            addSchedulePoint(day, hour, minute, value, fade);
            
        };

    }

    DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Cargando programacion...OK!");
    return true;
}

void DomDomScheduleMgtClass::addSchedulePoint(DomDomDayOfWeek day, uint8_t hour, uint8_t minute, bool fade)
{
    if (schedulePoints.size() >= EEPROM_MAX_SCHEDULE_POINTS)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn,"SCHEDULE", "Numero maximo de programaciones alcanzado. Se omitira esta inserccion");
        return;
    }

    schedulePoints.push_back(new DomDomSchedulePoint(day, hour, minute, fade));
}

void DomDomScheduleMgtClass::addSchedulePoint(DomDomDayOfWeek day, uint8_t hour, uint8_t minute, uint8_t value, bool fade)
{
    if (schedulePoints.size() >= EEPROM_MAX_SCHEDULE_POINTS)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn,"SCHEDULE", "Numero maximo de programaciones alcanzado. Se omitira esta inserccion");
        return;
    }

    schedulePoints.push_back(new DomDomSchedulePoint(day, hour, minute, value, fade));
}


bool DomDomScheduleMgtClass::begin()
{
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Iniciando programación...");
    if (!_started)
    {

        if (schedulePoints.size() > 0)
        {
            _started = true;

            xTaskCreate(
                this->scheduleTask,            /* Task function. */
                "ScheduleInitTask",     /* String with name of task. */
                10000,                  /* Stack size in bytes. */
                NULL,                   /* Parameter passed as input of the task */
                1,                      /* Priority of the task. */
                taskHandle              /* Task handle. */
            );
        }
        else
        {
            // Si no hay puntos de programación ponemos el valor al 100%
            DomDomLogger.log(DomDomLoggerClass::LogLevel::debug,"SCHEDULE", "No hay puntos de programacion. Cambiado a modo manual.");
            DomDomChannel.setTargetmA(DomDomChannel.maximum_mA);
        }
        
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info,"SCHEDULE", "Iniciando programación...OK!");
    }

    return true;
}

bool DomDomScheduleMgtClass::end()
{
    if (_started)
    {
        _started = false;
    }

    return true;
}

void DomDomScheduleMgtClass::update()
{
    DateTime now = DomDomRTC.now();
    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug,"SCHEDULE", "%d:%d Comprobando programacion", now.hour(), now.minute());

    DateTime horaAnterior;
    DateTime horaSiguiente;
    DomDomSchedulePoint *puntoAnterior = nullptr;
    DomDomSchedulePoint *puntoSiguiente = nullptr;

    bool correct;
    correct = getShedulePoint(horaAnterior, puntoAnterior, true);
    if (!correct)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error,"SCHEDULE", "No se encontra una programacion previa.");
        return;
    }

    correct = getShedulePoint(horaSiguiente, puntoSiguiente, false);
    if (!correct)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error,"SCHEDULE", "No se encontra la programacion siguiente.");
        return;
    }

    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug,"SCHEDULE", "intervalo obtenido: %d:%d - %d:%d", puntoAnterior->hour, puntoAnterior->minute, puntoSiguiente->hour, puntoSiguiente->minute);

    int mA = 0;
    int porcentaje = 0;

    if (!puntoSiguiente->fade || puntoAnterior->value == puntoSiguiente->value)
    {
        porcentaje = puntoSiguiente->value;
    }
    else
    {
        porcentaje = calcFadeValue(puntoAnterior->value, 
                                puntoSiguiente->value,
                                DomDomChannel.minimum_mA,
                                DomDomChannel.maximum_mA,
                                horaAnterior,
                                horaSiguiente);
        porcentaje = roundUp(porcentaje, CHANNEL_PERCENTAGE_MIN_STEP);
    }

    mA = DomDomChannel.minimum_mA + ((DomDomChannel.maximum_mA - DomDomChannel.minimum_mA) * (double)(porcentaje/100.0f));

    if (mA != DomDomChannel.target_mA)
    {
        DomDomChannel.setTargetmA(mA);
    }
}

void DomDomScheduleMgtClass::scheduleTask(void *parameter)
{
    int offset = 5;
    while(DomDomScheduleMgt.isStarted())
    {
        DomDomScheduleMgt.update();
        const int next_ms = (60 - DomDomRTC.now().second() + offset) * 1000;
        vTaskDelay(next_ms / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

int DomDomScheduleMgtClass::calcFadeValue(int prevValue, int nextValue, int min_value, int max_value, DateTime anterior, DateTime siguiente)
{
    int porcentaje_result = 0;
    int minutes_total = (siguiente - anterior).totalseconds() / 60;

    // Si la hora es la misma ajustamos directamente el valor.
    if (minutes_total == 0)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn,"SCHEDULE", "Misma hora de inicio y de final. ¿Error en programacion?");
        
        porcentaje_result = nextValue;
    }
    else
    {
        DateTime now = DomDomRTC.now();
        DateTime now_noseconds (now.year(),now.month(),now.day(),now.hour(), now.minute(), 0);

        int minutes = (now_noseconds - anterior).totalseconds() / 60;
        double minutes_porcentaje = (minutes * 100) / minutes_total / double(100);

        int porcentaje_valor = (nextValue - prevValue) * minutes_porcentaje;

        porcentaje_result = prevValue + porcentaje_valor;
        //int new_pwm = min_value + (max_value - min_value) * ((prevValue + porcentaje_valor) / double(100));
    }   

    return porcentaje_result;
}

void DomDomScheduleMgtClass::startTest(uint16_t value)
{
    if (_testInProgress)
    {
        _testInProgress = false;
        delay(500);
    }

    _testInProgress = true;
    end();

    DomDomChannel.setTargetmA(value);

    xTaskCreate(
        this->testTask,     /* Task function. */
        "testTask",         /* String with name of task. */
        10000,              /* Stack size in bytes. */
        NULL,               /* Parameter passed as input of the task */
        1,                  /* Priority of the task. */
        NULL                /* Task handle. */
    );

}

void DomDomScheduleMgtClass::testTask(void *parameter)
{
    for(int i = 0; i < 300; i++)
    {
        if (!DomDomScheduleMgt.testInProgress())
        {
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    DomDomScheduleMgt.stopTest();
    DomDomScheduleMgt.begin();

    vTaskDelete(NULL);
}

void DomDomScheduleMgtClass::stopTest()
{
    _testInProgress = false;
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomScheduleMgtClass DomDomScheduleMgt;
#endif