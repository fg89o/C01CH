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

#include "wifi.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include "configuration.h"
#include "../log/logger.h"

DomDomWifiClass::DomDomWifiClass()
{
    _connected = false;
};

bool DomDomWifiClass::begin()
{
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Iniciando WiFi...");

    ssid = DomDomWifi.readSTASSID();
    pwd = DomDomWifi.readSTAPass();
    if (!(ssid.length() > 1 && ssid.length() < EEPROM_SSID_NAME_LENGTH))
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "WIFI", "Usando valores wifi por defecto");
        ssid = WIFI_STA_SSID_NAME;
        pwd = WIFI_STA_PASSWORD;
    }

    mDNS_enabled = EEPROM.read(EEPROM_MDNS_ENABLED_ADDRESS);
    mDNS_hostname = EEPROM.readString(EEPROM_MDNS_HOSTNAME_ADDRESS);

    connect();

    if (!_connected)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "Iniciando WiFi...ERROR!", ssid.c_str());
    }else{
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Iniciando WiFi...OK!", ssid.c_str());
    }

    return true;
}

// Inicia la tarea del proceso de conexion
void DomDomWifiClass::connect()
{
    // Si tenemos red WIFI a la que conectarnos lo intentamos
    if (ssid.length() > 0)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Conectando a %s...", ssid.c_str());
        for(int i = 0; i < WIFI_NUM_RETRIES; i++)
        {
            connectSTAWifi();
            delay(WIFI_CONNECTION_LATENCY);
            
            _connected = WiFi.status() == WL_CONNECTED;

            if (!_connected)
            {
                DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "WIFI", "Intento %d de %d fallido", (i+1), WIFI_NUM_RETRIES);
            }else{
                DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "IP: %s", WiFi.localIP().toString().c_str());
                DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Conectando a %s...OK!", ssid.c_str());
                break;
            }
        }

        if (!_connected)
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "WIFI", "Conectando a %s...ERROR!", ssid.c_str());
        }
    }

    // Si no nos hemos conectado creamos el punto de acceso
    if (!_connected)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Creando AP...");
        for(int i = 0; i < WIFI_NUM_RETRIES; i++)
        {
            _connected = createOwnAPWifi();

            if (_connected)
            {
                DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Creando AP...OK!", ssid.c_str());
                break;
            }
            
            DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "WIFI", "Intento %d de %d fallido", (i+1), WIFI_NUM_RETRIES);
            delay(2000);
        }

        if (!_connected)
        {
            DomDomLogger.log(DomDomLoggerClass::LogLevel::warn, "WIFI", "Creando AP...ERROR!", ssid.c_str());
        }
    }
    
    if (_connected && mDNS_enabled)
    {
        beginmDNS();
    }
}

bool DomDomWifiClass::createOwnAPWifi()
{
    WiFi.mode(WIFI_AP);

    return WiFi.softAP(
        WIFI_AP_SSID_NAME, 
        WIFI_AP_PASSWORD, 
        WIFI_AP_CHANNEL,
        WIFI_AP_HIDDEN,
        WIFI_AP_MAX_CONNECTIONS
    );
}

int DomDomWifiClass::connectSTAWifi()
{
    WiFi.mode(WIFI_STA);

    return WiFi.begin(ssid.c_str(), pwd.c_str());
}

bool DomDomWifiClass::beginmDNS()
{
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Iniciando mDNS...");
    mDNS_hostname = EEPROM.readString(EEPROM_MDNS_HOSTNAME_ADDRESS);
    if (mDNS_hostname.length() <= 0)
    {
        mDNS_hostname = MDNS_HOSTNAME;
    }

    if (!MDNS.begin(mDNS_hostname.c_str())) {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "Iniciando mDNS...ERROR!");
        return false;
    }

    DomDomLogger.log(DomDomLoggerClass::LogLevel::debug, "WIFI", "mDNS hostname: %s.local", mDNS_hostname.c_str());
    DomDomLogger.log(DomDomLoggerClass::LogLevel::info, "WIFI", "Iniciando mDNS...OK!");

    return true;
}

bool DomDomWifiClass::isConnected(){
    if (getMode() == 1)
    {
        return (WiFi.status() == WL_CONNECTED);
    }

    return _connected;
}

wifi_mode_t DomDomWifiClass::getMode()
{
    return WiFi.getMode();
}

uint8_t DomDomWifiClass::getAPClients()
{
    if ( getMode() == WIFI_MODE_STA ){
        return 0;
    }

    return WiFi.softAPgetStationNum();
}

int8_t DomDomWifiClass::RSSI()
{
    return WiFi.RSSI();
}

void DomDomWifiClass::printWifiInfo()
{
    String modo;
    switch(WiFi.getMode())
    {
        case WIFI_MODE_AP:
            modo = "AP";
            break;
        case WIFI_MODE_STA:
            modo = "STA";
            break;
        case WIFI_MODE_APSTA:
            modo = "STA+AP";
            break;
        case WIFI_MODE_NULL:
            modo = "No inicializado";
            break;
        case WIFI_MODE_MAX:
            modo = "MAX";
            break;
    };

    Serial.println("=== Wifi Info ===");
    
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        Serial.printf("· Modo: STA\n");
        Serial.printf("· Estado: %s\n", (char *)(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado"));
        Serial.printf("· SSID: %s\n", WIFI_STA_SSID_NAME);
        Serial.printf("· IP Local: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("· Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("· DNS: %s\n", WiFi.dnsIP().toString().c_str());
    }

    if (WiFi.getMode() == WIFI_MODE_AP)
    {
        Serial.print("· Modo: AP\n");
        Serial.printf("· SSID: %s\n", WIFI_AP_SSID_NAME);
        Serial.printf("· IP Local: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("· Network ID: %s\n",WiFi.softAPNetworkID().toString().c_str());
        Serial.printf("· MAC: %s\n", WiFi.softAPmacAddress().c_str());
    }
}

bool DomDomWifiClass::saveSTASSID(String str)
{
    if (strlen(str.c_str()) > EEPROM_SSID_NAME_LENGTH)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "SSID name too long!");
        return false;
    }
    
    EEPROM.writeString(EEPROM_STA_SSID_NAME_ADDRESS, str);
    bool result = EEPROM.commit();
    if (!result)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "Error al guardar en la EEPROM!!");
    }
    return result;
}

String DomDomWifiClass::readSTASSID()
{
    return EEPROM.readString(EEPROM_STA_SSID_NAME_ADDRESS);
}

bool DomDomWifiClass::saveSTAPass(String password)
{
    if (strlen(password.c_str()) > EEPROM_STA_PASSWORD_LENGTH)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "Password too long!");
        return false;
    }

    EEPROM.writeString(EEPROM_STA_PASSWORD_ADDRESS, password);
    bool result = EEPROM.commit();
    if (!result)
    {
        DomDomLogger.log(DomDomLoggerClass::LogLevel::error, "WIFI", "Error al guardar en la EEPROM!!");
    }
    return result;
}

String DomDomWifiClass::readSTAPass()
{
    return EEPROM.readString(EEPROM_STA_PASSWORD_ADDRESS);
}

bool DomDomWifiClass::saveMDNSSettings()
{
    EEPROM.writeBool(EEPROM_MDNS_ENABLED_ADDRESS, mDNS_enabled);
    EEPROM.writeString(EEPROM_MDNS_HOSTNAME_ADDRESS, mDNS_hostname);
    return EEPROM.commit();
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomWifiClass DomDomWifi;
#endif