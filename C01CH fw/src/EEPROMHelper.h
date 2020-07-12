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
#ifndef DOMDOM_EEPROM_HELPER_h
#define DOMDOM_EEPROM_HELPER_h

#include <Arduino.h>

/**
 * Inicializa la EEPROm a los valores por defecto
 */
void EEPROMInit();

/**
 * Comprueba si es el primer arranque y en caso de serlo inicializa la EEPROM.
 */
void EEPROMCheck();

#endif /* DOMDOM_EEPROM_HELPER_h */