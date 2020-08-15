
#pragma once

#ifndef DOMDOM_GLOBAL_CONFIGURACION_h
#define DOMDOM_GLOBAL_CONFIGURACION_h

#define DEBUG

// Datos del proyecto
#define PROJECT_VER     1.1
#define PROJECT_MODEL   "C01CH"

//===========================================================================
//============================ Serial Settings  =============================
//===========================================================================
// Define la velocidad de comunicación del puerto COM
#define BAUDRATE 9600

//===========================================================================
//============================ STATUS LED CONTROL  ==========================
//===========================================================================

#define LED_STATUS_RESOLUTION   10
#define LED_STATUS_CHANNEL      10
#define LED_STATUS_PIN          2
#define LED_STATUS_BLINK_DELAY  100

//===========================================================================
//============================ WIFI Settings  ===============================
//===========================================================================

// Define la red wifi a la que intetara conectase por defecto
#define WIFI_STA_SSID_NAME "MIWIFI_2G_QQPJ"

// Define el password de la red wifi por defecto
#define WIFI_STA_PASSWORD "aFG6nxrr"

// Numero de intentos para conectar el wifi
#define WIFI_NUM_RETRIES 3

// Tiempo de espera entre intentos
#define WIFI_CONNECTION_LATENCY 5000

// Nombre del AP que se creara para conectarse al equipo (32 digitos max.)
#define WIFI_AP_SSID_NAME "C01CH"

// Contraseña para el AP (entre 8 y 63 max.)
#define WIFI_AP_PASSWORD ""

// Numero maximo de conexiones al punto de acceso ( 4 max. )
#define WIFI_AP_MAX_CONNECTIONS 4

// Canal (entre 1 y 13)
#define WIFI_AP_CHANNEL 6

// Estado del ssid. 1 = HIDDEN ; 0 = BROADCAST
#define WIFI_AP_HIDDEN 0

//===========================================================================
//============================ mDNS SECTION =================================
//===========================================================================

#define MDNS_ENABLED 1

// Nombre mDNS para localizar el equipo
#define MDNS_HOSTNAME "C01CHDEV"

//===========================================================================
//============================ CHANNELS SECTION =============================
//===========================================================================

#define CHANNEL_SIZE                    1
#define CHANNEL_RESOLUTION              8
#define CHANNEL_MAX_LEDS_CONFIG         10
#define CHANNEL_CURRENT_PIN             { 25 }
#define CHANNEL_BUS_REFRESH_INTERVAL    10000
#define CHANNEL_PERCENTAGE_MIN_STEP     5

//===========================================================================
//============================ FAN SECTION =============================
//===========================================================================
#define FAN_PWM_PIN                     17
#define FAN_PWM_RESOLUTION              10
#define FAN_PWM_CHANNEL                 12

//===========================================================================
//===================== RTC Y  NTP SECTION ==================================
//===========================================================================

#define RTC_BEGIN_ATTEMPS       3
#define RTC_BEGIN_ATTEMPS_DELAY 200

#define NTP_ENABLED             1
#define NTP_SERVERNAME          "pool.ntp.org"
#define NTP_TIMEZONE            "Europe/Madrid"
#define NTP_POSIX_TIMEZONE      "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_DELAY_ON_FAILURE    10000
#define NTP_DELAY_ON_SUCCESS    3600000

//===========================================================================
//============================ EEPROM SECTION ===============================
//===========================================================================

#define EEPROM_SIZE                             EEPROM_FAN_ENABLED_ADDRESS
#define EEPROM_INIT_RETRIES                     10
#define EEPROM_INA_SIZE                         512

#define EEPROM_STA_SSID_NAME_ADDRESS            4
#define EEPROM_SSID_NAME_LENGTH                 32
#define EEPROM_STA_PASSWORD_ADDRESS             EEPROM_STA_SSID_NAME_ADDRESS + EEPROM_SSID_NAME_LENGTH
#define EEPROM_STA_PASSWORD_LENGTH              64

#define EEPROM_CHANNEL_FIRST_ADDRESS            EEPROM_STA_PASSWORD_ADDRESS +  EEPROM_STA_PASSWORD_LENGTH
#define EEPROM_CHANNEL_MEMORY_SIZE              16
#define EEPROM_CHANNEL_LED_MEMORY_SIZE          7
#define EEPROM_CHANNEL_LED_COUNT                10

#define EEPROM_MAX_SCHEDULE_POINTS              50
#define EEPROM_SCHEDULE_STATUS_ADDRESS          EEPROM_CHANNEL_FIRST_ADDRESS + EEPROM_CHANNEL_MEMORY_SIZE + (EEPROM_CHANNEL_LED_MEMORY_SIZE * EEPROM_CHANNEL_LED_COUNT)
#define EEPROM_SCHEDULE_FIRST_ADDRESS           EEPROM_SCHEDULE_STATUS_ADDRESS + 1
#define EEPROM_SCHEDULEPOINT_SIZE               5

#define EEPROM_NTP_ENABLED_ADDRESS              EEPROM_SCHEDULE_FIRST_ADDRESS + (EEPROM_SCHEDULEPOINT_SIZE * EEPROM_MAX_SCHEDULE_POINTS )

#define EEPROM_MDNS_ENABLED_ADDRESS             EEPROM_NTP_ENABLED_ADDRESS + 1
#define EEPROM_MDNS_HOSTNAME_ADDRESS            EEPROM_MDNS_ENABLED_ADDRESS + 1
#define EEPROM_MDNS_HOSTNAME_LENGTH             16

#define EEPROM_NTP_SERVERNAME_ADDRESS           EEPROM_MDNS_HOSTNAME_ADDRESS + EEPROM_MDNS_HOSTNAME_LENGTH
#define EEPROM_NTP_SERVERNAME_LENGTH            32
#define EEPROM_NTP_TIMEZONENAME_ADDRESS         EEPROM_NTP_SERVERNAME_ADDRESS + EEPROM_NTP_SERVERNAME_LENGTH
#define EEPROM_NTP_TIMEZONENAME_LENGTH          32
#define EEPROM_NTP_TIMEZONEPOSIX_ADDRESS        EEPROM_NTP_TIMEZONENAME_ADDRESS + EEPROM_NTP_TIMEZONENAME_LENGTH
#define EEPROM_NTP_TIMEZONEPOSIX_LENGTH         32

#define EEPROM_FAN_ENABLED_ADDRESS              EEPROM_NTP_TIMEZONEPOSIX_ADDRESS + EEPROM_NTP_TIMEZONEPOSIX_LENGTH
#endif /* GLOBAL_CONFIGURACION_h */