/**
 * Óscar Fernández
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

#include "EEPROMHelper.h"
#include "configuration.h"
#include <EEPROM.h>

void EEPROMInit()
{
    EEPROM.write(1,1);

    /** DIRECCION EEPROM DEL WIFI */
    EEPROM.writeString(EEPROM_STA_SSID_NAME_ADDRESS, WIFI_STA_SSID_NAME);
    EEPROM.writeString(EEPROM_STA_PASSWORD_ADDRESS, WIFI_STA_PASSWORD);
    /************************************************************/
 
    /** DIRECCION EEPROM DEL SERVICIO MDNS*/
    EEPROM.write(EEPROM_MDNS_ENABLED_ADDRESS, MDNS_ENABLED);
    EEPROM.writeString(EEPROM_MDNS_HOSTNAME_ADDRESS, MDNS_HOSTNAME);
    /************************************************************/

    /** DIRECCIONES EEPROM DEL CANAL */
    int address = EEPROM_CHANNEL_FIRST_ADDRESS;
    // canal
    EEPROM.write(address,0);
    address += 1;
    // enabled
    EEPROM.write(address,0);
    address += 1;
    // MAX PWM
    EEPROM.writeUShort(address,0);
    address += 2;
    // MIN PWM
    EEPROM.writeUShort(address,0);
    address += 2;
    // MAX miliamps
    EEPROM.writeFloat(address,0);
    address += 4;
    // MIN voltage
    EEPROM.writeFloat(address,0);
    address += 4;
    // CURRENT PWM
    EEPROM.writeUShort(address,0);
    address += 2;
    // LEDS
    EEPROM.write(address,0);
    /************************************************************/

    /** DIRECCION EEPROM DE LA AGENDA */
    EEPROM.writeBool(EEPROM_SCHEDULE_STATUS_ADDRESS, true);
    /************************************************************/

    /** DIRECCIONES EEPROM DEL SERVICIO NTP */
    EEPROM.writeBool(EEPROM_NTP_ENABLED_ADDRESS, NTP_ENABLED);
    EEPROM.writeString(EEPROM_NTP_SERVERNAME_ADDRESS, NTP_SERVERNAME);
    EEPROM.writeString(EEPROM_NTP_TIMEZONENAME_ADDRESS, NTP_TIMEZONE);
    EEPROM.writeString(EEPROM_NTP_TIMEZONEPOSIX_ADDRESS, NTP_POSIX_TIMEZONE);
    /************************************************************/

    /** Confirmamos cambios */
    EEPROM.commit();
}

void EEPROMCheck()
{
    if (EEPROM.read(1) != 1)
    {
        Serial.print("Detectado primer arranque. Inicializando EEPROM\n");
        EEPROMInit();
    }
}
