#include <NoDelayNTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "SSID";
const char *password = "PASSWORD";

WiFiUDP ntpUDP;         // Инициализация объекта UDP
NTPClient ntp(ntpUDP);  //Инициализация класс ntp

void setup() {
  Serial.begin(115200);

  //Подключаемся к WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ntp.begin();  // Запускаем NTP
}

void loop() {
  ntp.update();

  static uint32_t tmr = 0;
  if (millis() - tmr >= 1000) {                  // Выполняем раз в 1 сек
    tmr = millis();                              // Обнуляем таймер
    Serial.println(ntp.getFullFormattedTime());  // Выводим в терминал дату и время в формате 'ДД:ММ:ГГГГ ЧЧ:ММ:СС'
  }
}
