/**
 * Copyright (c) 2022 Otto
 *
 * Данная лицензия разрешает лицам, получившим копию данного программного обеспечения 
 * и сопутствующей документации (в дальнейшем именуемыми «Программное обеспечение»), 
 * безвозмездно использовать Программное обеспечение без ограничений, 
 * включая неограниченное право на использование, копирование, изменение, слияние, публикацию, 
 * распространение, сублицензирование и/или продажу копий Программного обеспечения, а также лицам, 
 * которым предоставляется данное Программное обеспечение, при соблюдении следующих условий:
 * 
 * Указанное выше уведомление об авторском праве и данные условия должны быть включены во все 
 * копии или значимые части данного Программного обеспечения.
 *
 * ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, 
 * ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ ПРИГОДНОСТИ, 
 * СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ. 
 * НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, 
 * ЗА УЩЕРБ ИЛИ ПО ИНЫМ ТРЕБОВАНИЯМ, В ТОМ ЧИСЛЕ, ПРИ ДЕЙСТВИИ КОНТРАКТА, 
 * ДЕЛИКТЕ ИЛИ ИНОЙ СИТУАЦИИ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ 
 * ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.
 *
 */

#include "NoDelayNTPClient.h"

NTPClient::NTPClient(UDP& udp) {
  this->_udp            = &udp;
}

NTPClient::NTPClient(UDP& udp, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName) {
  this->_udp            = &udp;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP) {
  this->_udp            = &udp;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset){
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
  this->_updateInterval = updateInterval;
}

NTPClient::NTPClient(UDP& udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerIP   = poolServerIP;
  this->_poolServerName = NULL;
  this->_updateInterval = updateInterval;
}

void NTPClient::begin() {
  this->begin(NTP_DEFAULT_LOCAL_PORT);
}

void NTPClient::begin(int port) {
  this->_port = port;

  this->_udp->begin(this->_port);

  this->_udpSetup = true;
}

bool NTPClient::forceUpdate() {
  this->_lastUpdate = 0;
  this->_requestSent = 0;
  this->_requestDelay = 1;
  return true;
}

bool NTPClient::update() {
  int now = millis();

  if(!this->_udpSetup)
    this->begin();

  // Мы должны отправить запрос?
  if(this->_lastUpdate > 0 && now < this->_lastUpdate + this->_updateInterval) {
    // Обновление ещё не настало, продолжаем.
    return false;
  }

  // Мы должны обновить - мы отправили запрос, и время ожидания истекло,
  // или еще не отправили запрос?
  if(this->_requestSent == 0 || now > this->_requestSent + this->_requestDelay + REQUEST_TIMEOUT) {
    // если мы уже отправили запрос, давайте увеличим _requestDelay, чтобы не
    // забить потенциально неработающий NTP-сервер!
    if(this->_requestSent > 0) {
        this->_requestDelay *= 2;
      if(this->_requestDelay > 30000)
        this->_requestDelay = 30000;
    } else {
      // Это первый раз, когда мы пытаемся отправить запрос.
      // Очистить все старые пакеты, которые могут быть буферизованы
      while(this->_udp->parsePacket() != 0) {
        this->_udp->flush();
      }
    }

    // Правильно. Отправляем NTP-пакет!
    // Serial.printf("Sending an NTP packet (timeout=%i)!\n", this->_requestDelay + REQUEST_TIMEOUT);
    this->sendNTPPacket();

    // Запомнить, когда мы в последний раз отправляли запрос
    this->_requestSent = now;
  }

  // Проверяем ответы!
  int length = this->_udp->parsePacket();
  if( length > 0 ) {
   // Serial.println("Got an NTP reply!");
    this->_lastUpdNTP = true;
    this->_udp->read(this->_packetBuffer, NTP_PACKET_SIZE);
    this->_udp->flush();

    unsigned long highWord = word(this->_packetBuffer[40], this->_packetBuffer[41]);
    unsigned long lowWord = word(this->_packetBuffer[42], this->_packetBuffer[43]);
    // Объединить четыре байта (два слова) в длинное целое число.
    // Это время NTP (секунды с 1 января 1900 года):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    this->_currentEpoc = secsSince1900 - SEVENZYYEARS;

    // Очистить и сбросить наше состояние
    this->_requestSent = 0;
    this->_requestDelay = 1;
    this->_lastUpdate = now;
    return true;
  }

  return false;
}

unsigned long NTPClient::getEpochTime() const {
  return this->_timeOffset + // Пользовательское смещение
         this->_currentEpoc + // Эпоха возвращенная NTP-сервером
         ((millis() - this->_lastUpdate) / 1000); // Время с момента последнего обновления
}

int NTPClient::getDay() const {
  return (((this->getEpochTime()  / 86400L) + 4 ) % 7); //0 это воскресенье
}
int NTPClient::getHours() const {
  return ((this->getEpochTime()  % 86400L) / 3600);
}
int NTPClient::getMinutes() const {
  return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds() const {
  return (this->getEpochTime() % 60);
}

String NTPClient::getFormattedTime() const {
  unsigned long rawTime = this->getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

void NTPClient::end() {
  this->_udp->stop();

  this->_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset) {
  this->_timeOffset     = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval) {
  this->_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char* poolServerName) {
    this->_poolServerName = poolServerName;
}

void NTPClient::sendNTPPacket() {
  // Установить все байты в буфере на 0
  memset(this->_packetBuffer, 0, NTP_PACKET_SIZE);
  // Инициализировать значения, необходимые для формирования запроса NTP
  // (см. URL-адрес выше для получения подробной информации о пакетах)
  this->_packetBuffer[0] = 0b11100011;   // LI, версия, режим
  this->_packetBuffer[1] = 0;     // Stratum, или тип часов
  this->_packetBuffer[2] = 6;     // Интервал опроса
  this->_packetBuffer[3] = 0xEC;  // Точность одноранговых часов
  // 8 нулевых байтов для корневой задержки и корневой дисперсии
  this->_packetBuffer[12]  = 49;
  this->_packetBuffer[13]  = 0x4E;
  this->_packetBuffer[14]  = 49;
  this->_packetBuffer[15]  = 52;

  // Всем полям NTP были присвоены значения, теперь
  // вы можете отправить пакет с запросом метки времени:
  if  (this->_poolServerName) {
    this->_udp->beginPacket(this->_poolServerName, 123);
  } else {
    this->_udp->beginPacket(this->_poolServerIP, 123);
  }
  this->_udp->write(this->_packetBuffer, NTP_PACKET_SIZE);
  this->_udp->endPacket();
}


//Получаем Год
int NTPClient::getYear() const {
  time_t rawtime = this->getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int year = ti->tm_year + 1900;
  
  return year;
}

//Получаем Месяц
int NTPClient::getMonth() const {
  time_t rawtime = this->getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int month = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);
  
  return month;
}

//Получаем День
int NTPClient::getDate() const {
  time_t rawtime = this->getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int day = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);
  
  return day;
}

//Получаем Форматированную дату одной строкой 'DD:MM:YYYY'
String NTPClient::getFormattedDate() const{
	int day = this->getDate();
	int month = this->getMonth();
	int year = this->getYear();
	
	String dayStr = day < 10 ? "0" + String(day) : String(day);
	String monthStr = month < 10 ? "0" + String(month) : String(month);
	String yearStr = String(year);
	
	return dayStr + "." + monthStr + "." + yearStr;
}

//Получаем Форматированную дату и время одной строкой 'DD:MM:YYYY ЧЧ:ММ:СС'
String NTPClient::getFullFormattedTime() const {
   time_t rawtime = this->getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return dayStr + "-" + monthStr + "-" + yearStr + " " + hoursStr + ":" + minuteStr + ":" + secondStr;
}


//Получаем миллисекунды текущей секунды
uint16_t NTPClient::getMs() const {
    return (millis() - this->_lastUpdate) % 1000;
}


//Получаем дату и время последней успешной синхронизации (всегда возвращает дату и время последней синхронизации)
String NTPClient::getFullFormLastUpdate() {
	static String NewData;
	if(this->_lastUpdNTP == true) {
	NewData = this->getFullFormattedTime();
	this->_lastUpdNTP = false;
	}

	return NewData;
}