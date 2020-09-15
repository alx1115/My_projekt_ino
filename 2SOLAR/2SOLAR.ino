/*
9 - КОНТРАСТ НА Vo
10 - DS18B20
13 - реле
A0 - кнопка вверх
A1  - кнопка select
A2  - кнопка вниз

дисплей
12 - RS
11 - E
9 - КОНТРАСТ НА Vo
2 - DB7
3 - DB6
4 - DB5
5 - DB4
GND - K
GND - R\W
GND - Vss
Vcc - Vdd
Vcc - 510 Om - A
 
 */


#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_1602_RUS.h>
LiquidCrystal_1602_RUS lcd(12, 11, 5, 4, 3, 2);
#define ONE_WIRE_BUS 10
#define rele 13  //  пин реле
#define contrs 9  //  контраст
#define keyUp     A0
#define keySelect A1
#define keyDown   A2
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

  float tempTAup;  // температура в ТА верх
  float tempTAdown; // температура в ТА низ
  float tempSolar;  // температура на коллекторе
  float tempDelta=0.1;  // температура дельта на включение насоса 
    unsigned long currentMillis = millis();
    unsigned long Millis = millis();
    long loopTime = 4000; // 2 сек время простоя  в меню
      int Ntemp;
      int kontrast=70;
    

void setup() {
pinMode( keyUp, INPUT);           // назначить выводу порт ввода
digitalWrite(keyUp, HIGH); 
pinMode(keySelect, INPUT);           // назначить выводу порт ввода
digitalWrite(keySelect, HIGH); 
pinMode(keyDown, INPUT);           // назначить выводу порт ввода
digitalWrite(keyDown, HIGH); 
tempDelta=0.01;
  pinMode(rele,OUTPUT);
  digitalWrite(rele,0);
  pinMode(contrs,OUTPUT);
  analogWrite(contrs,kontrast); // контраст 0-255
  pinMode(10,INPUT);  // - DS18B20
  lcd.begin(16, 2);

 lcd.clear(); 
 lcd.setCursor(0, 0);
 lcd.print(L"\xDE\xB2");
 lcd.setCursor(6, 0);
 lcd.print(L"]---\x7EГ");
 lcd.setCursor(10, 1);
 lcd.print(L"-L");
 lcd.setCursor(1, 1);
 lcd.print(L"я----(F)-");
sensors.setWaitForConversion(false); 
sensors.setResolution(0, 9);
sensors.setResolution(1, 9);
sensors.setResolution(2, 9);
}

void loop() 
{

 tempTAup=15.00;
tempTAdown=15.00;
 sensors.setWaitForConversion(false); 
 sensors.requestTemperatures();
 lcd.setCursor(2, 0);
 lcd.print(tempSolar,1);
 lcd.setCursor(12, 0);
 lcd.print(tempTAup,1);
 lcd.setCursor(12, 1);
 lcd.print(tempTAdown,1);
 if(tempSolar-tempTAdown>=tempDelta) //  включаем насос
 {   if( tempTAdown>-100 && tempSolar>-100) {digitalWrite(rele,1);} else {digitalWrite(rele,0);} //  проверка датчиков температуры
 if(Ntemp==1){lcd.setCursor(6, 1);lcd.print(L"(|");lcd.setCursor(0, 0);lcd.print(L"ы"); lcd.setCursor(7, 0);lcd.print(L"\x7E---"); lcd.setCursor(2, 1);lcd.print(L"-\x7F--");tempTAup=sensors.getTempCByIndex(0);}
 if (analogRead(keySelect)<100){ menu(); } 
 if(Ntemp==15){lcd.setCursor(7, 1);lcd.print(L"/");lcd.setCursor(0, 0);lcd.print(L"ь"); lcd.setCursor(7, 0);lcd.print(L"-\x7E--");lcd.setCursor(2, 1);lcd.print(L"\x7F---");tempTAdown=sensors.getTempCByIndex(1);}
 if (analogRead(keySelect)<100){ menu(); } 
 if(Ntemp==30){lcd.setCursor(7, 1);lcd.print(L"-");lcd.setCursor(0, 0);lcd.print(L"э");lcd.setCursor(7, 0);lcd.print(L"--\x7E-");lcd.setCursor(2, 1);lcd.print(L"---\x7F");tempSolar=sensors.getTempCByIndex(2);}
 if (analogRead(keySelect)<100){ menu(); } 
 if(Ntemp==45){lcd.setCursor(7, 1);lcd.print(L"ё");lcd.setCursor(0, 0);lcd.print(L"ю");lcd.setCursor(7, 0);lcd.print(L"---\x7E");lcd.setCursor(2, 1);lcd.print(L"--\x7F-");delay(100);}
 if (analogRead(keySelect)<100){ menu(); } 
 if (Ntemp==45){Ntemp=0;};
 Ntemp++;   

 }
 else 
 {
  digitalWrite(rele,0); // выключаем насос
  lcd.setCursor(0, 0);lcd.print(L" ");
  lcd.setCursor(7, 0);  lcd.print(L"----");
  lcd.setCursor(2, 1); lcd.print(L"--(OFF)-");
  if (analogRead(keySelect)<100){ menu();}
  tempTAup=sensors.getTempCByIndex(0);
  tempTAdown=sensors.getTempCByIndex(1)+0.5;
  tempSolar=sensors.getTempCByIndex(2);
 }
}

void menu()
{
float td=tempDelta;
int kn=kontrast;
lcd.clear(); 
lcd.setCursor(0, 0);lcd.print(L"НАСТРОЙКА насос");
lcd.setCursor(0, 1);lcd.print(L"t\xB2-tL =");
lcd.setCursor(8, 1);lcd.print(tempDelta,1);lcd.print(L"\xDF ");
lcd.setCursor(14, 1);lcd.print(L"ON");
delay(200);
    currentMillis = millis();
    while(millis()-currentMillis<=loopTime)
     {
      if (analogRead(keyDown) < 100 ) { tempDelta=tempDelta-0.1;currentMillis = millis(); if(tempDelta<0.1){tempDelta=0.1;}lcd.setCursor(8, 1); lcd.print(tempDelta,1);lcd.print(L"\xDF "); delay(100);}
      if (analogRead(keyUp) < 100 ) { tempDelta=tempDelta+0.1;currentMillis = millis(); if(tempDelta>99.9){tempDelta=99.9;}lcd.setCursor(8, 1); lcd.print(tempDelta,1);lcd.print(L"\xDF "); delay(100);}
       if (analogRead(keySelect) < 100 ) {currentMillis = 0; lcd.setCursor(8, 1); lcd.print("OK   ");delay(100);}
      }
lcd.clear(); 
lcd.setCursor(0, 0);lcd.print(L"КОНТРАСТНОСТЬ");
lcd.setCursor(14, 0);lcd.print(kontrast);
delay(300);
      currentMillis = millis();
    while(millis()-currentMillis<=loopTime)
     {
      if (analogRead(keyDown) < 100 ) { kontrast=kontrast-1;currentMillis = millis(); if(kontrast<1){kontrast=1;}lcd.setCursor(14, 0); lcd.print(kontrast);  analogWrite(9,kontrast);delay(200);}
      if (analogRead(keyUp) < 100 )  { kontrast=kontrast+1;currentMillis = millis(); if(kontrast>99){kontrast=99;}lcd.setCursor(14, 0); lcd.print(kontrast);  analogWrite(9,kontrast);delay(200);}
       if (analogRead(keySelect) < 100 ) {currentMillis = 0; lcd.setCursor(14, 0); lcd.print("OK");}
       if(kontrast<10){lcd.setCursor(14, 0); lcd.print("  ");}
      }
delay(300);
lcd.clear(); 
lcd.setCursor(0, 0);
 lcd.print(L"\xDE\xB2");
 lcd.setCursor(6, 0);
 lcd.print(L"]---\x7EГ");
 lcd.setCursor(10, 1);
 lcd.print(L"-L");
 lcd.setCursor(1, 1);
 lcd.print(L"я----(F)-");
 delay(300);
}
