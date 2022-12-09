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

    const char*   _poolServerName = "ru.pool.ntp.org"; // Сервер по умолчанию
    IPAddress     _poolServerIP;
    int           _port           = NTP_DEFAULT_LOCAL_PORT;
    long          _timeOffset     = 0;

    unsigned long _updateInterval = 60000;  // In ms

    unsigned long _currentEpoc    = 0;      // In s
    unsigned long _lastUpdate     = 0;      // In ms

    unsigned long _requestSent    = 0;      // in ms (when the last request was sent)
    unsigned long _requestDelay   = 1;      // in ms (a cumulative delay to slow down constant failures)

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
     * Set time server name
     *
     * @param poolServerName
     */
    void setPoolServerName(const char* poolServerName);

    /**
     * Starts the underlying UDP client with the default local port
     */
    void begin();

    /**
     * Starts the underlying UDP client with the specified local port
     */
    void begin(int port);

    /**
     * This should be called in the main loop of your application. By default an update from the NTP Server is only
     * made every 60 seconds. This can be configured in the NTPClient constructor.
     *
     * @return true on success, false on failure
     */
    bool update();

    /**
     * This will force the update from the NTP Server.
     *
     * @return true on success, false on failure
     */
    bool forceUpdate();

    int getDay() const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;

    /**
     * Changes the time offset. Useful for changing timezones dynamically
     */
    void setTimeOffset(int timeOffset);

    /**
     * Set the update interval to another frequency. E.g. useful when the
     * timeOffset should not be set in the constructor
     */
    void setUpdateInterval(unsigned long updateInterval);

    /**
     * @return time formatted like 'ЧЧ:ММ:СС'
     */
    String getFormattedTime() const;

    /**
     * @return time in seconds since Jan. 1, 1970
     */
    unsigned long getEpochTime() const;

    /**
     * Stops the underlying UDP client
     */
    void end();
	
	//Эти блоки добавил вручную, для получения форматированной даты
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
     * @return Форматированную дату (String) 'ДД.ММ.ГГГГ'
     */
	String getFormattedDate() const;
	
	/**
     * @return Форматированную дату и время (String) 'DD:MM:YYYY ЧЧ:ММ:СС'
     */
    String getFullFormattedTime() const; 
};