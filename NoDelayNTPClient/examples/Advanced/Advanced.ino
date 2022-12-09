//Настройки NTP
#define SET_GMT 6                   // Часовой пояс в "GMT" формате (цифры или цифры с "-") [К примеру: У Омска GMT+6, указываем просто цифру 6 или у Гренландии GMT-3, указываем -3];
#define SET_PERIOD 40               // Установить период обновления "в минутах" (период 1 минуты равен 60000 миллисекунд)
#define SET_HOST "ru.pool.ntp.org"  // Установить хост


#include <NoDelayNTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "SSID";
const char *password = "PASSWORD";

WiFiUDP ntpUDP;         // Инициализация объекта UDP
NTPClient ntp(ntpUDP);  //Инициализация класс ntp

//Можно так: раскомментировать эту строчку и закомментировать 3 строчки ниже
//NTPClient timeClient(ntpUDP, SET_HOST, SET_GMT * 3600U, SET_PERIOD * 60000UL);

void setup() {
  //Или вот так: закомментировать 3 строчки ниже, и раскомментировать NTPClient выше
  // SET_GMT - Для перевода часового пояса GTM "в часы" (часовой пояс в часах = часовой пояс * 3600U), (U - unsigned, выделяем память для беззнаковой константы).
  // SET_PERIOD - Для перевода периода обновления "в минуты" (количество миллисекунд = количество минут * 60000UL), (UL - unsigned long, выделяем память для беззнаковой константы).
  ntp.setTimeOffset(SET_GMT * 3600U);           // Устанавливаем часовой пояс "в часах"
  ntp.setUpdateInterval(SET_PERIOD * 60000UL);  // Устанавливаем период обновления "в минутах"
  ntp.setPoolServerName(SET_HOST);              // Устанавливаем NTP хост для запросов

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
  ntp.update();  // Получаем время с NTP


  static uint32_t tmr = 0;
  if (millis() - tmr >= 1000) {                  // Выполняем раз в 1 сек
    tmr = millis();                              // Обнуляем таймер
    Serial.println(ntp.getFullFormattedTime());  // Выводим в терминал дату и время в формате 'ДД:ММ:ГГГГ ЧЧ:ММ:СС'
  }
}
