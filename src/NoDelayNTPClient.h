#pragma once

#include "Arduino.h"

#include <Udp.h>

#define REQUEST_TIMEOUT 1000UL
#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

class NTPClient {
  private:
    UDP*          _udp;
    bool          _udpSetup       = false;

	bool		  _lastUpdNTP     = false;

    const char*   _poolServerName = "ru.pool.ntp.org"; // Сервер по умолчанию
    IPAddress     _poolServerIP;
    int           _port           = NTP_DEFAULT_LOCAL_PORT;
    long          _timeOffset     = 0;

    unsigned long _updateInterval = 60000;  // В мс

    unsigned long _currentEpoc    = 0;      // В секундах
    unsigned long _lastUpdate     = 0;      // В мс

    unsigned long _requestSent    = 0;      // В мс (когда был отправлен последний запрос)
    unsigned long _requestDelay   = 1;      // В мс (кумулятивная задержка для замедления постоянных сбоев)

    byte          _packetBuffer[NTP_PACKET_SIZE];

    void          sendNTPPacket();

  public:

    NTPClient(UDP& udp);
    NTPClient(UDP& udp, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval);
    NTPClient(UDP& udp, IPAddress poolServerIP);
    NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset);
    NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval);

    /**
     * Установить имя сервера времени
     *
     * @param poolServerName
     */
    void setPoolServerName(const char* poolServerName);

    /**
     * Запускает базовый UDP-клиент с локальным портом по умолчанию
     */
    void begin();

    /**
     * Запускает базовый UDP-клиент с указанным локальным портом
     */
    void begin(int port);

    /**
     * Это должно вызываться в основном цикле вашего приложения. По умолчанию обновление с NTP-сервера
     * производится каждые 60 секунд. Это можно настроить в конструкторе NTPClient
     *
     * @return true в случае успеха, false в случае неудачи
     */
    bool update();

    /**
     * Это приведет к принудительному обновлению с NTP-сервера
     *
     * @return true в случае успеха, false в случае неудачи
     */
    bool forceUpdate();

    int getDay() const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;

    /**
     * Изменяет смещение времени. Полезно для динамической смены часовых поясов
     */
    void setTimeOffset(int timeOffset);

    /**
     * Установите интервал обновления на другую частоту. Например. полезно, когда
     * timeOffset не должен быть установлен в конструкторе
     */
    void setUpdateInterval(unsigned long updateInterval);

    /**
     * @return время отформатировано как 'ЧЧ:ММ:СС'
     */
    String getFormattedTime() const;

    /**
     * @return время в секундах с 1 января 1970 г.
     */
    unsigned long getEpochTime() const;

    /**
     * Останавливает базовый UDP-клиент
     */
    void end();
	
	/**
     * @return Год (int)
     */
	int getYear() const;
	
	/**
     * @return Месяц (int)
     */
	int getMonth() const;
	
	/**
     * @return Дату (int)
     */
	int getDate() const;

	/**
     * @return Миллисекунды (uint16_t)
     */
	uint16_t getMs() const;

	/**
     * @return Форматированную дату (String) 'ДД.ММ.ГГГГ'
     */
	String getFormattedDate() const;
	
	/**
     * @return Форматированную дату и время (String) 'DD:MM:YYYY ЧЧ:ММ:СС'
     */
    String getFullFormattedTime() const; 
	
	/**
     * @return Форматированную дату и время последней успешной синхронизации (String) 'DD:MM:YYYY ЧЧ:ММ:СС'
     */
    String getFullFormLastUpdate();
};