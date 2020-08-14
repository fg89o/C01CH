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
#include "log/logger.h"

static String TAG = "WIFI";

DomDomWifiClass::DomDomWifiClass()
{
    _connected = false;
};

bool DomDomWifiClass::begin()
{
    DomDomLogger.logI(TAG, "Iniciando WiFi\r\n");

    ssid = DomDomWifi.readSTASSID();
    pwd = DomDomWifi.readSTAPass();
    if (!(ssid.length() > 1 && ssid.length() < EEPROM_SSID_NAME_LENGTH))
    {
        DomDomLogger.logD(TAG,"Valores en EEPROM no validos. Usando valores wifi por defecto\r\n");
        ssid = WIFI_STA_SSID_NAME;
        pwd = WIFI_STA_PASSWORD;
    }

    mDNS_enabled = EEPROM.read(EEPROM_MDNS_ENABLED_ADDRESS);
    mDNS_hostname = EEPROM.readString(EEPROM_MDNS_HOSTNAME_ADDRESS);

    connect();

    return true;
}

// Inicia la tarea del proceso de conexion
void DomDomWifiClass::connect()
{
    // Si tenemos red WIFI a la que conectarnos lo intentamos
    if (ssid.length() > 0)
    {
        DomDomLogger.logI(TAG, "Conectando Wifi a %s\r\n", ssid.c_str());
        for(int i = 0; i < WIFI_NUM_RETRIES; i++)
        {
            connectSTAWifi();
            int prev_ms = millis();
            while(WiFi.status() != WL_CONNECTED && millis() - prev_ms < WIFI_CONNECTION_LATENCY)
            {
                if ((millis() - prev_ms) % 1000 == 0)
                {
                    DomDomLogger.logV(TAG, "_");
                }
            }
            
            _connected = WiFi.status() == WL_CONNECTED;

            if (!_connected)
            {
                DomDomLogger.logV(TAG, ".");
                delay(1000);
            }else{
                DomDomLogger.logV(TAG, "\r\n");
                printWifiInfo();
                break;
            }
        }

        if (!_connected)
        {
            DomDomLogger.logE(TAG, "Agotados intentos de conexion!\r\n");
        }
    }

    // Si no nos hemos conectado creamos el punto de acceso
    if (!_connected)
    {
        DomDomLogger.logI(TAG, "Creando AP\r\n");
        for(int i = 0; i < WIFI_NUM_RETRIES; i++)
        {
            _connected = createOwnAPWifi();

            if (_connected)
            {
                printWifiInfo();
                break;
            }
            
            delay(2000);
        }
    }
    
    if (!_connected)
    {
        DomDomLogger.logE(TAG, "No se puedo crear el punto de acceso!\r\n");
    }
    
    if (_connected && mDNS_enabled)
    {
        DomDomLogger.logI(TAG, "Configurando mDNS");
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

    mDNS_hostname = EEPROM.readString(EEPROM_MDNS_HOSTNAME_ADDRESS);
    if (mDNS_hostname.length() <= 0)
    {
        mDNS_hostname = MDNS_HOSTNAME;
    }

    if (!MDNS.begin(mDNS_hostname.c_str())) {
        DomDomLogger.logE(TAG, "Error al configurar el servicio MDNS!\r\n");
        return false;
    }

    DomDomLogger.logI(TAG, "Configurado mDNS con hostname %s.local\r\n", mDNS_hostname.c_str());
    return true;
}

bool DomDomWifiClass::isConnected(){
    return _connected;
}

int DomDomWifiClass::getMode()
{
    return WiFi.getMode();
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

    DomDomLogger.logI(TAG, "=== Wifi Info ===\r\n");
    
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        DomDomLogger.logI(TAG, "· Modo: \tSTA\r\n");
        DomDomLogger.logI(TAG, "· Estado: \t%s\r\n", (char *)(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado"));
        DomDomLogger.logI(TAG, "· SSID: \t%s\r\n", WIFI_STA_SSID_NAME);
        DomDomLogger.logI(TAG, "· IP Local: \t%s\r\n", WiFi.localIP().toString().c_str());
        DomDomLogger.logI(TAG, "· Gateway: \t%s\r\n", WiFi.gatewayIP().toString().c_str());
        DomDomLogger.logI(TAG, "· DNS: \t%s\r\n", WiFi.dnsIP().toString().c_str());
    }

    if (WiFi.getMode() == WIFI_MODE_AP)
    {
        DomDomLogger.logI(TAG, "· Modo: \tAP\r\n");
        DomDomLogger.logI(TAG, "· SSID: \t%s\r\n", WIFI_AP_SSID_NAME);
        DomDomLogger.logI(TAG, "· IP Local: \t%s\r\n", WiFi.softAPIP().toString().c_str());
        DomDomLogger.logI(TAG, "· Network ID: \t%s\r\n",WiFi.softAPNetworkID().toString().c_str());
        DomDomLogger.logI(TAG, "· MAC: \t%s\r\n", WiFi.softAPmacAddress().c_str());
    }
}

bool DomDomWifiClass::saveSTASSID(String str)
{
    if (strlen(str.c_str()) > EEPROM_SSID_NAME_LENGTH)
    {
        DomDomLogger.logE(TAG, "ERROR: SSID name too long!\r\n");
        return false;
    }
    
    EEPROM.writeString(EEPROM_STA_SSID_NAME_ADDRESS, str);
    bool result = EEPROM.commit();
    if (!result)
    {
        DomDomLogger.logE(TAG, "Error al guardar en la EEPROM!!\r\n");
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
       DomDomLogger.logE(TAG, "ERROR: Password too long!\r\n");
        return false;
    }

    EEPROM.writeString(EEPROM_STA_PASSWORD_ADDRESS, password);
    bool result = EEPROM.commit();
    if (!result)
    {
        DomDomLogger.logE(TAG, "Error al guardar en la EEPROM!!\r\n");
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