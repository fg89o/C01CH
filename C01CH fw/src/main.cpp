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

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "configuration.h"
#include "statusLedControl/statusLedControl.h"
#include "channel/channelMgt.h"
#include "wifi/WiFi.h"
#include "rtc/rtc.h"
#include "webServer/webServer.h"
#include "EEPROMHelper.h"
#include "channel/ScheduleMgt.h"
// #include "fan/fanControl.h"
#include "log/logger.h"

void initEEPROM()
{
  
  DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "MAIN", "Iniciando EEPROM con %d bytes", EEPROM_SIZE+EEPROM_INA_SIZE);
  bool init = false;
  int err_count = 0;
  while(err_count<EEPROM_INIT_RETRIES && !init)
  {
      init = EEPROM.begin(EEPROM_SIZE+EEPROM_INA_SIZE);
      if (!init)
      {
          err_count++;
          Serial.print(".");
          delay(200);
      }
  }

  EEPROMCheck();

}

void setup()
{
  Serial.begin(BAUDRATE);

  DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "MAIN", "C01CH Firmware | Copyright (c) 2020 Óscar Fernández");

  // Inicializamos la eeprom
  initEEPROM();

  // Inicializamos el control del led de estado
  DomDomStatusLedControl.begin();
  DomDomStatusLedControl.blinkDelay = 2000;
  DomDomStatusLedControl.blinkInfinite();
  DomDomStatusLedControl.blinkDelay = 1000;

  // Inicializa el wifi
  DomDomWifi.begin();
  // Si llegamos aqui y no estamos conectados resetamos en 5 segundos
  if (!DomDomWifi.isConnected())
  {
    int seg = 5;
    DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "MAIN", "Equipo sin conexion. Se reiniciará en %d segundos!", seg);
    delay(seg * 1000);
    ESP.restart();
  }

  // Configuramos el RTC
  DomDomRTC.load();
  // Si tenemos conexion a internet inicamos todos los servicios
  if (DomDomWifi.getMode() == 1)
  {
    // Inicializacion del RTC
    DomDomRTC.begin();
  } else {
    DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "MAIN", "Equipo sin conexion no se establecera la hora RTC");
  }

  // Iniciamos el servidor web
  DomDomWebServer.begin();

  // Configuramos los canales
  DomDomChannelMgt.loadAll();
  DomDomChannelMgt.begin();

  // Puntos de programacion
  DomDomScheduleMgt.load();
  
  // Iniciamos la programacion
  if (EEPROM.readBool(EEPROM_SCHEDULE_STATUS_ADDRESS))
  {
      DomDomScheduleMgt.begin();
  }else{
      // DomDomChannel.setPWM(DomDomChannel.loadCurrentPWM());
  }

  // Ventilador
  // DomDomFanControl.begin();
}

unsigned long prev_millis = millis();

void loop()
{

  // Si la conexion a internet no esta establecida volvemos a probar
  if ( (DomDomWifi.getMode() == WIFI_MODE_STA && !DomDomWifi.isConnected()) || DomDomWifi.getMode() == WIFI_MODE_AP )
  {
    // Solamente comprobamos una vez por minuto
    if ( millis() - prev_millis > WIFI_RECONNECTION_PERIOD )
    {
      prev_millis = millis();

      // Si estamos en modo AP y tenemos clientes conectados salimos
      if ( DomDomWifi.getMode() == WIFI_MODE_AP && DomDomWifi.getAPClients() > 0)
      {
        return;
      }

      DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "MAIN", "Reintentando conexion wifi...");
      DomDomWifi.begin();
      
      if ( DomDomWifi.getMode() == WIFI_MODE_STA )
      {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "MAIN", "Reintentando conexion wifi...OK!");
        DomDomRTC.begin();
      } else {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "MAIN", "Reintentando conexion wifi...ERROR!");
      }
    }
  }

}