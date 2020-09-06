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

#include "webServer.h"
#include <FS.h>
#include "../../lib/AsyncTCP/AsyncTCP.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "rtc/rtc.h"
#include "configuration.h"
#include "wifi/WiFi.h"
#include "statusLedControl/statusLedControl.h"
#include "channel/ScheduleMgt.h"
#include "channel/channel.h"
#include "EEPROMHelper.h"
#include "Update.h"
#include "fan/fanControl.h"

String GetBodyContent(uint8_t *data, size_t len)
{
  String content = "";
  for (size_t i = 0; i < len; i++) {
    content .concat((char)data[i]);
  }
  return content;
}

void SendResponse(AsyncWebServerRequest *request, AsyncResponseStream *response )
{
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void SendResponse(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    request->send(response);
}

DomDomWebServerClass::DomDomWebServerClass(){}

void DomDomWebServerClass::begin()
{
    _server = new AsyncWebServer (WEBSERVER_HTTP_PORT);

    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    _server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // AJAX para el reloj
    _server->on("/rtc", HTTP_GET, getRTCData);
    _server->on("/rtc", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setRTCData);
    
    // AJAX para el wifi
    _server->on("/red", HTTP_GET, getWifiData);
    _server->on("/red", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setWifiData);

    // AJAX para los canales
    _server->on("/canales", HTTP_GET, getChannelsData);
    _server->on("/canales", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setChannelsData);

    // AJAX para el reset
    _server->on("/reset", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setRestart);

    // AJAX para el restablecer valores de fbrica
    _server->on("/reset", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setFactorySettings);

    // AJAX para los puntos de programacion
    _server->on("/schedule", HTTP_GET, getSchedule);
    _server->on("/schedule", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setSchedule);

    // AJAX para el control de ventilador
    _server->on("/fansettings", HTTP_GET, getFanSettings);
    _server->on("/fansettings", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setFanSettings);

    // AJAX para realizar un test de color
    _server->on("/test", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setTest);

    // AJAX para el restablecer valores de fbrica
    _server->on("/resetMaximos", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setResetMaxValues);

    // AJAX para actualizar el firmware
     _server->on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
        // the request handler is triggered after the upload has finished... 
        // create the response, add header, and send response
        AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        response->addHeader("Connection", "close");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        
        DomDomChannel.end();

        delay(2000);
        
        ESP.restart();

    }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        //Upload handler chunks in data
        if (!index) {
            int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
            if (filename == "firmware.bin")
            {
                cmd = U_FLASH;
            }
            else if (filename == "spiffs.bin")
            {
                cmd = U_SPIFFS;
            }
            else{
                return request->send(400, "text/plain", "Nombre de fichero incorrecto.");
            }
            Serial.printf("%s -> %s\r\n", filename.c_str(), (cmd == U_SPIFFS) ? "U_SPIFFS" : "U_FLASH" );

            if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
                Update.printError(Serial);
                return request->send(400, "text/plain", "OTA could not begin");
            }
        }

        // Write chunked data to the free sketch space
        if(len){
            if (Update.write(data, len) != len) {
                return request->send(400, "text/plain", "OTA could not begin");
            }
        }
            
        if (final) { // if the final flag is set then this is the last frame of data
            if (!Update.end(true)) { //true to set the size to the current progress
                Update.printError(Serial);
                return request->send(400, "text/plain", "Could not end OTA");
            }
        }else{
            return;
        }
    });

    _server->onNotFound([](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html");
    });
    
    _server->begin();

    Serial.println("HTTP server started");
};

void DomDomWebServerClass::getRTCData(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["ready"] = DomDomRTC.ready;
    jsonDoc["enabled"] = DomDomRTC.NTPStarted();
    jsonDoc["servername"] = DomDomRTC.NTPServername();
    jsonDoc["unixtime"] = DomDomRTC.now().unixtime();
    jsonDoc["timezonePosix"] = DomDomRTC.NTPPosixZone();
    jsonDoc["timezone"] = DomDomRTC.NTPTimezone();

    serializeJson(jsonDoc, *response);

    SendResponse(request, response);
}

void DomDomWebServerClass::setRTCData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);
    if (err) { 
        switch (err.code()) {
            case DeserializationError::Ok:
                Serial.print(F("Deserialization succeeded"));
                break;
            case DeserializationError::InvalidInput:
                Serial.print(F("Invalid input!"));
                break;
            case DeserializationError::NoMemory:
                Serial.print(F("Not enough memory"));
                break;
            default:
                Serial.print(F("Deserialization failed"));
                break;
        };
        request->send(400); 
        return;
    }

    if (!doc.containsKey("enabled"))
    {
        Serial.println("No se recibidio un JSON valido");
        request->send(400); 
        return;
    }
    
    if (doc["enabled"])
    {
        Serial.printf("Habilitado servicio NTP\n");
        DomDomRTC.setNTPServername(doc["servername"]);
        DomDomRTC.setNTPtimezone(
            doc["timezone"],
            doc["timezonePosix"]
        );

        DomDomRTC.endNTP();
        delay(500);
        DomDomRTC.beginNTP();
    }

    DomDomRTC.save();
    
    Serial.printf("Nueva fecha %s\n",DomDomRTC.now().timestamp().c_str());

    SendResponse(request);
}

void DomDomWebServerClass::getWifiData(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
        
    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["mode"] = DomDomWifi.getMode() == WIFI_MODE_AP ? "AP" : "STA";
    jsonDoc["sta_enabled"] = "true";
    jsonDoc["rssi"] = DomDomWifi.RSSI();
    jsonDoc["current_channel"] = WiFi.channel();
    if (DomDomWifi.getMode() == WIFI_MODE_STA)
    {
        jsonDoc["ssid"] = DomDomWifi.ssid;
        jsonDoc["current_gateway"] = WiFi.gatewayIP().toString();
        jsonDoc["current_ip"] = WiFi.localIP().toString();
        jsonDoc["current_dns"] = WiFi.dnsIP().toString();
    }
    else
    {
        jsonDoc["ssid"] = "";
        jsonDoc["current_gateway"] = WiFi.softAPIP().toString();
        jsonDoc["current_ip"] = WiFi.softAPIP().toString();
        jsonDoc["current_dns"] = "0.0.0.0";
    }
    
    jsonDoc["mdns_enabled"] = DomDomWifi.mDNS_enabled;
    jsonDoc["mdns_hostname"] = DomDomWifi.mDNS_hostname;

    serializeJson(jsonDoc, *response);
    
    SendResponse(request,response);
}

void DomDomWebServerClass::setWifiData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);
    if (err) { 
        request->send(400); 
        return;
    }

    if (doc.containsKey("ssid"))
    {
        String recv_ssid = doc["ssid"];
        DomDomWifi.saveSTASSID(recv_ssid);
    }

    if (doc.containsKey("pwd"))
    {
        String recv_pwd = doc["pwd"];
        DomDomWifi.saveSTAPass(recv_pwd);
    }
    
    if (doc.containsKey("mdns_enabled"))
    {
        DomDomWifi.mDNS_enabled = doc["mdns_enabled"];
    }

    if (doc.containsKey("mdns_hostname"))
    {
        String name = doc["mdns_hostname"];
        DomDomWifi.mDNS_hostname = name;
    }
    
    DomDomWifi.saveMDNSSettings();

    SendResponse(request);
}

void DomDomWebServerClass::getChannelsData(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
        
    StaticJsonDocument<2048> jsonDoc;
    
    jsonDoc["modo_programado"] = DomDomScheduleMgt.isStarted();

    DomDomSchedulePoint * point = nullptr;
    DateTime dt;
    if (DomDomScheduleMgt.getShedulePoint(dt, point, false))
    {
        jsonDoc["siguiente_punto_hora"] = point->hour;
        jsonDoc["siguiente_punto_minuto"] = point->minute;
    }
    
    JsonArray ports = jsonDoc.createNestedArray("canales");

    JsonObject obj = ports.createNestedObject();
    obj["enabled"] = DomDomChannel.getEnabled();
    obj["channel_num"] = DomDomChannel.getNum();
    obj["target_mA"] = DomDomChannel.target_mA;
    obj["max_mA"] = DomDomChannel.maximum_mA;
    obj["min_mA"] = DomDomChannel.minimum_mA;
    obj["max_volts"] = DomDomChannel.target_V;
    obj["dac_pwm"] = DomDomChannel.curr_dac_pwm;
    obj["max_leds"] = CHANNEL_MAX_LEDS_CONFIG;

    String str_lvolts = String(DomDomChannel.lastBusVoltaje_V,2);
    String str_lamps = String(DomDomChannel.lastBusCurrent_mA,3);
    String str_pamps = String(DomDomChannel.busCurrentPeak_mA,3);
    String str_pvolts = String(DomDomChannel.busVoltagePeak_V,2);
    String str_ppower = String(DomDomChannel.busPowerPeak_W,2);

    obj["bus_volts"] = serialized(str_lvolts);
    obj["bus_miliamps"] = serialized(str_lamps);
    obj["bus_volts_peak"] = serialized(str_pvolts);
    obj["bus_miliamps_peak"] = serialized(str_pamps);
    obj["bus_power_peak"] = serialized(str_ppower);
    
    JsonArray leds = obj.createNestedArray("leds");
    for (int j = 0; j < DomDomChannel.leds.size(); j++)
    {
        JsonObject led = leds.createNestedObject();
        led["K"] = DomDomChannel.leds[j]->K;
        led["nm"] = DomDomChannel.leds[j]->nm;
        led["W"] = DomDomChannel.leds[j]->W;
    }

    serializeJson(jsonDoc, *response);
    
    SendResponse(request,response);
}

void DomDomWebServerClass::setChannelsData(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(2048);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    if (err) { 
        request->send(400); 
        return;
    }

    if (doc.containsKey("modo_programado"))
    {
        if (doc["modo_programado"])
        {
            Serial.printf("[Schedule] Programacion iniciada.\n");
            DomDomScheduleMgt.begin();
            Serial.println("2");
        }
        else
        {
            Serial.printf("[Schedule] Programacion parada.\n");
            DomDomScheduleMgt.end();
            Serial.println("3");
        }

        DomDomScheduleMgt.save();
    }

    if (doc.containsKey("canales"))
    {
        JsonArray canales = doc["canales"].as<JsonArray>();
        for(JsonObject canal : canales)
        {
            DomDomChannel.setEnabled(canal["enabled"]);
            DomDomChannel.target_V = canal["max_volts"];
            DomDomChannel.maximum_mA = canal["max_mA"];
            DomDomChannel.minimum_mA = canal["min_mA"];

            if (!DomDomScheduleMgt.isStarted())
            {
                // DomDomStatusLedControl.blink(1);
                DomDomChannel.setTargetmA(canal["target_mA"]);
            }

            if (canal.containsKey("leds"))
            {
                DomDomChannel.leds.clear();
                JsonArray leds = canal["leds"].as<JsonArray>();
                for(JsonObject led : leds)
                {
                    DomDomChannelLed *obj = new DomDomChannelLed();
                    obj->K = led["K"];
                    obj->nm = led["nm"];
                    obj->W = led["W"];
                    
                    if (obj->K > 0 || obj->nm > 0 || obj->W > 0)
                    {
                        DomDomChannel.leds.push_back(obj);
                    }
                    
                }
            }

            DomDomChannel.save();
        }

    }

    SendResponse(request);
}

void DomDomWebServerClass::setRestart(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");

    if (err) { 
        request->send(400); 
        return;
    }

    if (doc.containsKey("reset"))
    {
        if (doc["reset"])
        {
            request->send(response);
            
            DomDomStatusLedControl.blink(10);

            ESP.restart();
        }
    }

    request->send(400);
}

void DomDomWebServerClass::setResetMaxValues(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");

    if (err) { 
        request->send(400); 
        return;
    }

    if (doc.containsKey("reset"))
    {
        if (doc["reset"])
        {
            
            DomDomChannel.busPowerPeak_W = 0;
            DomDomChannel.busCurrentPeak_mA = 0;
            DomDomChannel.busVoltagePeak_V = 0;

            request->send(response);
        }
    }

    request->send(400);
}

void DomDomWebServerClass::setFactorySettings(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");

    if (err) { 
        request->send(400); 
        return;
    }

    if (doc.containsKey("factorysettings"))
    {
        if (doc["factorysettings"])
        {
            request->send(response);

            EEPROMInit();
            ESP.restart();
        }
    }

    request->send(400);
}

void DomDomWebServerClass::getSchedule(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
        
    DynamicJsonDocument jsonDoc(6000);
    
    jsonDoc["max_schedule_points"] = EEPROM_MAX_SCHEDULE_POINTS;
    jsonDoc["channel_size"] = 1;
    JsonArray points = jsonDoc.createNestedArray("schedule");

    for(int i = 0; i < DomDomScheduleMgt.schedulePoints.size(); i++)
    {
        JsonObject obj = points.createNestedObject();
        obj["hour"] = DomDomScheduleMgt.schedulePoints[i]->hour;
        obj["minute"] = DomDomScheduleMgt.schedulePoints[i]->minute;
        obj["fade"] = DomDomScheduleMgt.schedulePoints[i]->fade;
        
        JsonArray values = obj.createNestedArray("values");
        values.add(DomDomScheduleMgt.schedulePoints[i]->value);
    }

    serializeJson(jsonDoc, *response);
    
    SendResponse(request,response);
}

void DomDomWebServerClass::setSchedule(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(6000);
    DeserializationError err = deserializeJson(doc, bodyContent);

    if (err) { 
        request->send(400); 
        return;
    }

    DomDomScheduleMgt.schedulePoints.clear();
    JsonArray points = doc.as<JsonArray>();
    Serial.printf("[Schedule] Recibidos %d puntos\n", points.size());
    for(int i = 0; i < points.size(); i++)
    {
        DomDomSchedulePoint *p = new DomDomSchedulePoint(ALL, points[i]["hour"], points[i]["minute"], points[i]["values"][0], (bool)points[i]["fade"]);
        DomDomScheduleMgt.schedulePoints.push_back(p);
    }

    Serial.printf("[Schedule] Guardando...\n");
    DomDomScheduleMgt.save();
    Serial.printf("[Schedule] Guardado\n");

    SendResponse(request);
}

void DomDomWebServerClass::setTest(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(2048);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    if (err) { 
        request->send(400); 
        return;
    }
    
    if (doc.containsKey("canales"))
    {
        JsonArray canales = doc["canales"].as<JsonArray>();
        uint16_t pwm[canales.size()];

        for(int i = 0; i < canales.size(); i++)
        {
            pwm[i] = canales[i]["current_pwm"];
        }
        
        DomDomScheduleMgt.startTest(pwm[0]);
    }

    SendResponse(request);
}

void DomDomWebServerClass::getFanSettings(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
        
    StaticJsonDocument<1024> jsonDoc;
    
    jsonDoc["enabled"] = DomDomFanControl.isStarted();
    jsonDoc["max_pwm"] = DomDomFanControl.max_pwm;
    jsonDoc["min_pwm"] = DomDomFanControl.min_pwm;
    jsonDoc["curr_pwm"] = DomDomFanControl.curr_pwm;
    jsonDoc["max_channel_value"] = DomDomFanControl.max_channel_value;
    jsonDoc["min_channel_value"] = DomDomFanControl.min_channel_value;

    serializeJson(jsonDoc, *response);
    
    SendResponse(request,response);
}

void DomDomWebServerClass::setFanSettings(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
     String bodyContent = GetBodyContent(data, len);
    
    DynamicJsonDocument doc(1024);;
    DeserializationError err = deserializeJson(doc, bodyContent);

    if (err) { 
        request->send(400); 
        return;
    }
    
    if (doc.containsKey("enabled"))
    {
        if (doc["enabled"])
        {
            Serial.printf("[FAN] Control automático iniciado.");
            DomDomFanControl.begin();
        }
        else
        {
            Serial.printf("[FAN] Control automático parado.");
            DomDomFanControl.end();
        }
    }

    if (DomDomFanControl.isStarted())
    {
        DomDomFanControl.max_channel_value = doc["max_channel_value"];
        DomDomFanControl.min_channel_value = doc["min_channel_value"];
        DomDomFanControl.max_pwm = doc["max_pwm"];
        DomDomFanControl.min_pwm = doc["min_pwm"];
    }
    else
    {
        if (doc.containsKey("curr_pwm"))
        {
            DomDomFanControl.setCurrentPWM(doc["curr_pwm"]);
        }
    }

    DomDomFanControl.save();
    
    SendResponse(request);
}

#if !defined(NO_GLOBAL_INSTANCES)
DomDomWebServerClass DomDomWebServer;
#endif