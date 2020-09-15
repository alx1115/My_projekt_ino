
/*
 свободные цифровые пины
0   Насос теплицы 1  
1   Насос теплицы 2           
2  18b20         
3   cross zero 0 
4  biper
5 - Сервоклапан  минус
6 - Вентилятор котла
7 - Сервоклапан  плюс
8  - Насос котла 
9   RX       -модем
10  TX       -модем  
11   S0 термопара
12   CS  термопара
13   CSK  термопара


 Аналоговые
 0  кнопка вверх 
 1  средняя
 2  кнопка вниз
 3
 4 экран SDA   часы
 5 экран SCL   часы
 6  термистор теплицы 2
 7 термистор теплицы 1
 

 
  
  
  
 */


 #include <VitconNTC.h>

//#define THERMISTORPIN A7 // Analog Input for Thermistor

// 25 Celsius - 10 Kohm, B = 4200 for NTC-10KD-5J
// There may error rate depending on the thermistor specification.
// Please change the values
#define NTC_NORM_TEMPERATURE 25.0
#define NTC_NORM_RESISTANCE 10000.0
#define NTC_DIVIDER_RESISTANCE 10000.0
#define NTC_B_VALUE 4200
using namespace vitcon;
VitconNTC ntc(NTC_NORM_TEMPERATURE, NTC_NORM_RESISTANCE, NTC_B_VALUE);


#include <CyberLib.h> //Библиотека от Cyber-Place.ru

//#include "TimerOne.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


#include <LCD_1602_RUS.h>
LCD_1602_RUS lcd(0x27, 20, 4);

#include <max6675.h>

#define thermoDO  12
#define thermoCS  11
#define thermoCLK 13
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

#include <EEPROM.h>

#define keyDow  A2 // кнопки
#define keySel  A1
#define keyUp   A0

#define V_Vent  6
#define Servo_Minus  5
#define Servo_Plus   7

unsigned long currentMillis = millis();
long loopTime = 3000; // 3 минуты время простоя меню
int i=0;    //  счетчик удердания кнопки.
int temp; // текущая температура  котел_вода
byte z_voda; //  заданная температура
int  Temp_Outdoors; // Уличная температура 
byte Temp_Nasos_On; // Температура включения насоса котла  
int Temp_Servo;    // Температура после сервопривода

int Temp_Servo_Preset;
byte Delta_Temp_Teplic;
int Temp_Teplic1=35;
int Temp_Teplic2=17;
int Temp_Tepl1_Preset=0;
int Temp_Tepl2_Preset=0;
byte Temp_Tepl1_Correct=0;
byte Temp_Tepl2_Correct=0;
int Temp_Tepl1_Alarm=0;
int Temp_Tepl2_Alarm=0;

//bool SERVO=0;
int smoke=0;  //  текущая температура дыма
int z_maxsmoke; // максимальная температура дыма
int obr;    //  обороты вентилятора
int stop_smoke=0; // температура отключения котла по дыму
byte deltavoda;

//String mytime;
//String num;
int time_rozjich=0;
int obr_vent=0;
int obr_pause=0;
byte tt=0;


//byte pause=255;  // пауза на гистерезис
//String toWait="";
int ve=0;

int Min_Servo=0;
int Max_Servo=0;
int Min_Outdoors=0;
int Max_Outdoors=0;

int state=0;

 byte col=5; // курсор

 int ch = 0;
int Backlight=0;
  wchar_t  *str1=L"ТЕМП-РА";
  wchar_t  *str2=L"НА КОТЕЛЕ";     
  wchar_t *St_state = L" STOP ";
        
//byte Nasos_Kotel=255;
volatile uint8_t tic, Ob_litle=255;
//uint8_t data;
 VitconNTC ntc1(NTC_NORM_TEMPERATURE, NTC_NORM_RESISTANCE, NTC_B_VALUE);

 VitconNTC ntc2(NTC_NORM_TEMPERATURE, NTC_NORM_RESISTANCE, NTC_B_VALUE);



#include <SoftwareSerial.h>
SoftwareSerial mySerial(9, 10); // TX,RX // GSM модем
String number = "", val= "";
//String money = "";
int cf = 0; // байт с порта модема  



void setup() {
 mySerial.begin(38400);        // 115200 38400  AT+IPR=115200   38400


   D0_Out; // Насос теплицы 1
   D1_Out; // Насос теплицы 2
   D6_Out;//Вентилятор котла.
   D8_Out; // Насос котла
   D7_Out;//Сервопривод плюс
   D5_Out;;//Сервопривод минус
   
   D0_High; // Насос теплицы1 выкл
   D1_High; // Насос теплицы2 выкл
   D7_Low; // Сервопривод 
   D5_Low; // Сервопривод
   D8_High; //  Насос котла выкл
   
  D3_In; //настраиваем порт на вход для отслеживания прохождения сигнала через ноль  
  
//CHANGE –  прерывание вызывается при любом изменении значения на входе; 
//RISING – вызов прерывания при изменении уровня напряжения с низкого (Low) на высокий(HIGH) 
//FALLING – вызов прерывания при изменении уровня напряжения с высокого (HIGH) на низкий (Low) 
    attachInterrupt(1, detect_up, RISING);  // настроить срабатывание прерывания interrupt0 на pin 2 на низкий уровень
    StartTimer1(halfcycle, 40); //время для одного разряда ШИМ
    StopTimer1(); //остановить таймер
     
  lcd.init();  
  lcd.backlight();
 
//pinMode(keyDow, INPUT);           // назначить выводу порт ввода
digitalWrite(keyDow, HIGH); 
//pinMode(keySel, INPUT);           // назначить выводу порт ввода
digitalWrite(keySel, HIGH); 
//pinMode(keyUp, INPUT);           // назначить выводу порт ввода
digitalWrite(keyUp, HIGH); 

//pinMode(A7, INPUT);  
//pinMode(A6, INPUT);  
   sensors.begin();
   //   Ob_litle=255; //255=мин. 0=макс 
     // Nasos_Kotel=255; //255=мин. 0=макс 
  bip();
 
lcd.clear();
smoke=thermocouple.readCelsius();

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  temp=sensors.getTempCByIndex(0);
  

   EEPROM.get(5,z_voda); // читаем сохраненные значения в переменные
   EEPROM.get(10,deltavoda);
   EEPROM.get(12,Temp_Nasos_On);
   EEPROM.get(15,obr);
   EEPROM.get(17,obr_pause);
   obr_pause=255;
   EEPROM.get(20,z_maxsmoke);
   EEPROM.get(27,stop_smoke);
   EEPROM.get(29,Temp_Servo_Preset);
   EEPROM.get(31,Temp_Tepl1_Preset);
   EEPROM.get(33,Temp_Tepl2_Preset);
   EEPROM.get(35,Delta_Temp_Teplic);

       EEPROM.get(37,Min_Servo);  
       EEPROM.get(39,Max_Servo);
       EEPROM.get(41,Min_Outdoors);
       EEPROM.get(43,Max_Outdoors);
       EEPROM.get(45,Temp_Tepl1_Correct);
       EEPROM.get(47,Temp_Tepl2_Correct);
       EEPROM.get(49,Temp_Tepl1_Alarm);
       EEPROM.get(51,Temp_Tepl2_Alarm);



     
   
  for ( int i = 60; i <= 68; i++) //читаем номер телефона
  {
    number = number + String(EEPROM.read(i),DEC);
    
    delay(15);
  }

  number = "+380"+number;
   

   lcd.setCursor(5, 1);
  lcd.print(number);
   lcd.setCursor(0, 2);
 // lcd.print(L"Modem initialization");
 delay(10000); 
 buffcle();
mySerial.begin(9600);
delay(200); 
mySerial.println("AT");
delay(200); 
      // 115200 38400  AT+IPR=115200   38400
//mySerial.println("AT+IPR=9600");
//mySerial.println("AT");
//mySerial.println("ATE0");
delay(200); 
mySerial.println("AT+CMGD=4");
delay(200); 
mySerial.println("AT+CLIP=1"); //АОН будет включен 
delay(200); 
mySerial.println("AT+CMGF=1"); //обычный режим кодировки СМС 
delay(200); 
mySerial.println("AT+CSCS=\"GSM\"\r"); //режим кодировки текста 
//"AT+CSCS=\"GSM\"\r"
delay(2000); 


  //  EEPROM.get(2,state);// последний статус в работе
  lcd.clear();

   
  
  
    scr();
 
     if (stop_smoke<smoke)
      {   state=2; //нагрев
          St_state = L"НАГРЕВ";  }
   
     if (temp>Temp_Nasos_On) { D8_High;  } // включили насос
      sensors.setWaitForConversion(false);
       sensors.setResolution(9);
}
 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////                               
void loop() 
{
     
     

      temp=sensors.getTempCByIndex(0);   // опрос датчиков температуры
      Temp_Servo=sensors.getTempCByIndex(1);
      Temp_Outdoors=sensors.getTempCByIndex(2);
      sensors.requestTemperatures();
         
      int raw_adc = analogRead(A7);
      Temp_Teplic1 = ntc1.GetTemperature(raw_adc)+Temp_Tepl1_Correct;
      raw_adc = analogRead(A6);
      Temp_Teplic2 = ntc2.GetTemperature(raw_adc)+Temp_Tepl1_Correct;

           
      smoke=thermocouple.readCelsius();
      
      //smoke=121;
   // temp=45;
  //  Temp_Servo=35;
  //  smoke=155;
    //вывод температур
  lcd.setCursor(18, 1);
  lcd.print(temp); if(temp<10)lcd.print(" ");
  scr();
   lcd.setCursor(18, 2);
  lcd.print(Temp_Teplic1);if(Temp_Teplic1<10)lcd.print(" ");
   lcd.setCursor(18, 3);
  lcd.print(Temp_Teplic2); if(Temp_Teplic2<10)lcd.print(" ");
  
  lcd.setCursor(4, 1);
  lcd.print(smoke); lcd.print(" ");
  
     lcd.setCursor(5, 0);   lcd.print(obr_vent); if(obr_vent<10){lcd.print(" ");}
     
   currentMillis = millis(); 
   if( Backlight>100){Backlight=0;tt=0;}
   Backlight++;
  while(millis()-currentMillis<=loopTime)  // основной цикл опроса кнопок. 
    {  
      if (analogRead(keySel)<100){
      i=0;
      Backlight=0;
      tone(4, 3400);
      
      delay(50);
      while(analogRead(keySel)<100 && i<10000) {i++;}
      noTone(4);
      if (i>=9999){  bip();lcd.clear();lcd.setCursor(0, 0);lcd.print(L"ГЛАВНОЕ МЕНЮ");bip();delay(1000); menu(); i=0;}  // нажата центральная  и  удержана -  вызов меню настройки 
      else { bip(); STATUS(); scr();  i=0; } // нажата центральная кнопка START
      
      }
      if (analogRead(keyUp)<100)
      {tone(4, 3400);Backlight=0; delay(100);i = 0;
      while (analogRead(keyUp) < 100 && i < 10000) {i++;}
      noTone(4);
      if (i >= 9999) {
        bip();  // нажата центральная  и  удержана -  вызов меню настройки телефона
        lcd.clear(); lcd.setCursor(0, 0); lcd.print(L"GSM"); bip();delay(1000);configGSM();scr();i = 0;} 
      else if (i != 0) { bip(); bip();str1=L"ТЕМПЕР-РА";   str2=L"НА КОТЛЕ";z_voda=vvod(str1,str2, z_voda,5,12,1);scr();  i=0;}  // нажата центральная кнопка вызов меню настройки температур и угла сервы.
      } 
      
     if (analogRead(keyDow)<100)
      {Backlight=0;
       i=0;tone(4, 3400);
       delay(200);
       while(analogRead(keyDow)<100 && i<10000) {i++;}
      noTone(4);
      if (i>=9999){  bip();lcd.clear();lcd.setCursor(0, 0);lcd.print(L"МЕНЮ СЕРВО");bip();delay(1000); menu_servo();lcd.clear();scr(); i=0;}  // нажата центральная  и  удержана -  вызов меню настройки 
      else { bip();  bip(); bip();str1=L"ТЕМПЕР-РА";   str2=L"НА КОТЛЕ";   z_voda=vvod(str1,str2, z_voda,5,12,1); scr();  i=0; } // нажата центральная кнопка START
        delay(100);
     
     // нажата центральная кнопка вызов меню настройки температур и угла сервы.
      } 
      
   if (millis()-currentMillis<=loopTime/2){lcd.setCursor(14, 0); lcd.print("      ");}
     if (millis()-currentMillis>=loopTime/2) {lcd.setCursor(14, 0); lcd.print(St_state); lcd.setCursor(6, 2);lcd.print(" ");lcd.print(Temp_Servo); lcd.print(" ");D7_Low;D5_Low;}
   //if(obr!=255)
   // {
   //              if(ve==0){lcd.setCursor(4, 0);lcd.print(L"|");}
    //            if(ve==900){lcd.setCursor(4, 0);lcd.print(L"/");}
    //             if(ve==1800){lcd.setCursor(4, 0);lcd.print(L"-");}
    //             if(ve==2700){lcd.setCursor(4, 0);lcd.print(L"ё");}
     //            ve=ve+100;
     //           if(ve>3400)ve=0;
      
    //................................... }
         if (state==3){
         bip();
          } // авария
     }


     monitor_port();
     
    if (state==1 && time_rozjich < 140) // переход с режима розжиг в нагрев
       {
        time_rozjich++;
        lcd.setCursor(14, 0); lcd.print(140-time_rozjich);lcd.print("_");
        if( smoke > stop_smoke) 
        { bip(); bip();
          state=2; //нагрев
          St_state = L"НАГРЕВ";
          time_rozjich=0;
        }
        if(time_rozjich == 139)//139
        {
          state=3; //затухание при розжиге
          St_state = L"ЗАТУХ ";
          Ob_litle=255;
          lcd.setCursor(14, 0);lcd.print(St_state);
          time_rozjich=0;
          
         
          delay(2000);
           
           scr();
        }
       }
       if (Temp_Teplic1 <= Temp_Tepl1_Alarm && tt==0){ tt=1;sms("Tepl1_Alarm");GSMCall();Backlight=0;}
       if (Temp_Teplic2 <= Temp_Tepl2_Alarm && tt==0){ tt=1;sms("Tepl2_Alarm");GSMCall();Backlight=0;}
       if (state == 2 && smoke < stop_smoke && tt==0){ tt=1; state = 3; St_state = L"ЗАТУХ "; sms("3ATUX ");GSMCall();Backlight=0;scr();lcd.setCursor(14, 0);lcd.print(St_state);}                           //затухание при нагреве
   
       
       



         if (state==3){
         
          } // авария
      
  // lcd. rightToLeft();
 //  lcd.leftToRight();
 //   t = rtc.getTime();
 WORK();

//if (temp<-100 || smoke==0) // обрыв датчиков температуры
//{ state=3; St_state = L"Sensor"; }

scr();
 
//  temp=sensors.getTempCByIndex(0); 
//  Temp_Servo=sensors.getTempCByIndex(1);
  
  // temp=21;//*************************************************
  // Temp_Servo=19;
    Temp_Servo_Preset=map(Temp_Outdoors,Min_Outdoors,Max_Outdoors,Max_Servo,Min_Servo);
    if(Temp_Servo_Preset>Max_Servo)Temp_Servo_Preset=Max_Servo;
    if(Temp_Servo_Preset<Min_Servo)Temp_Servo_Preset=Min_Servo;
    lcd.setCursor(6, 3);lcd.print("{");lcd.print(Temp_Servo_Preset);lcd.print("}");
    
   if(Temp_Servo<Temp_Servo_Preset-1) {D5_Low;D7_High; lcd.setCursor(6, 2);lcd.print(" ");lcd.print(Temp_Servo); lcd.print("\x7E"); }   // D7_Out;//Сервопривод плюс (Low) на высокий(HIGH) 
    if(Temp_Servo>Temp_Servo_Preset+1) {D7_Low;D5_High;lcd.setCursor(6, 2);lcd.print("\x7F");lcd.print(Temp_Servo); lcd.print(" ");}  // D5_Out;;//Сервопривод минус
  
      //  lcd.print(L"Servo ");lcd.print("\x7F");lcd.print(Temp_Servo); lcd.print("\x7E"); 
     
}


///////////////////////////////////////////////////////////////////////////////////////////////
void monitor_port()
{ val = "";
  if (mySerial.available()) { //если в мониторе порта ввели что-то
    while (mySerial.available()) { //сохраняем строку в переменную val
      ch = mySerial.read();
      val += char(ch);
      delay(10);
    }
  }
  if (val.indexOf("RING") > -1) { //если звонок обнаружен, то проверяем номер
    if (val.indexOf(number) > -1) { //если номер звонящего наш. Укажите свой номер без "+"
      lcd.setCursor(14, 0); lcd.print("->RING");
      val = "";
      buffcle();
      delay(100);
      val = "";

      mySerial.println("ATH0"); //разрываем связь
     // GSMCall();
       sms(" ");
        buffcle();

    }
  }

}
///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////G S M /////////////////////
void configGSM()
{

/*
  //D1  GSM TX  модем
  //D0  GSM RX  модем
  //
  // позвонить ATD+380661152112;
  // ATD*111#   сколько денег на счету
  //   AT+CPAS   - 0  готов к работе
  // ATH0 повесить трубку
  // AT+CSQ  -  качество сигнала
  // AT+COPS? информация о домашней сети
  // AT+CMGD=4 удалить все сообщения

  // ATE0  выключить эхо
  int ch = 0;
  //String val = "";
  money = "";
  int ur = 0;

  //String input = "";

  //mySerial.begin(38400);        // 115200 38400  AT+IPR=115200   38400



 // buffcle();


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(val);
  delay(3000);
  val = "";


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Operator:");
  mySerial.println("AT+COPS?"); // оператор
  delay(2000);
  while (mySerial.available()) { //сохраняем входную строку в переменную val
    ch = mySerial.read();
    val += char(ch);
    delay(10);
  }
  toWait = val;
  val = "";
  if (toWait.indexOf("+COPS:") > -1)  //AT+COPS? информация о операторе
  { toWait =  toWait.substring(10);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Operator:");
    lcd.setCursor(0, 1);
    lcd.print(toWait);


    //if (toWait.indexOf("+COPS:") > 0){myGLCD.print("No operator  ", 270,40);}

  }
  delay(1000);
  //_______________________________________________________________________________
  val = "";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Signal:");
  // while(ur==0)
  {
    mySerial.println("AT+CSQ"); // уровень сигнала
    delay(2000);
    while (mySerial.available()) { //сохраняем входную строку в переменную val
      ch = mySerial.read();
      val += char(ch);
      delay(10);
    }
    toWait = val;


    if (toWait.indexOf("+CSQ:") > -1)  //сигнал
    { toWait =  toWait.substring(15, 19);
      ur = 100 / 31 * toWait.toInt();
      if (ur > 100) {
        ur = 0;
      }

      lcd.setCursor(0, 1);
      lcd.print(ur); //ur
      lcd.setCursor(2, 1);
      lcd.print("%");
      delay(1000);


    }

    val = "";
    delay(1000);
  }
  val = "";
  //____________________________________________________________________________
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Money:");
  mySerial.println("ATD*111#"); // денег на счету
  delay(5000);
  while (mySerial.available()) { //сохраняем входную строку в переменную val
    ch = mySerial.read();
    val += char(ch);
    delay(10);
  }
  toWait = val;

  if (toWait.indexOf("+CUSD:") > -1)  //  счет
  { toWait = toWait.substring(21,37                                                                                                                                                                                                                                  );// toWait.substring(32, 48);
    lcd.setCursor(0, 1);
    toWait = " Na " + toWait;
    lcd.print(toWait);
    delay(1000);
    money = toWait;

  }
  val = "";
  delay(1000);
  ///////////////////



  //String number="";//String(a);
  //for(int i=30;i<=38;i++)
  //{
  // number=number+String(EEPROM.read(i));
  //}
*/
  //  mySerial.println(number);


  col=5;
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print(L"НОМЕР ТЕЛЕФОНА:");
  
  //lcd.blink(); //lcd. noblink();
 // lcd.cursor();//lcd.noCursor();
 //lcd.setCursor(0, 1);
// lcd.print("+380");lcd.print(number);
 //delay(2000);



  // int num=0;

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("+380");
  delay(200);
  for (  i = 60; i <= 68; i++)
  { cf = EEPROM.read(i);
    delay(15);
    lcd.setCursor(col, 1);
    lcd.print(cf);

    while ( analogRead(keySel) > 100)
    {
      if (analogRead(keyUp) < 100) {
        bip();
        cf = cf + 1;
        if (cf > 9) {
          cf = 9;
        };
        lcd.setCursor(col, 1);
        lcd.print(cf);
        delay(200);
      }
      if (analogRead(keyDow) < 100) {
        bip();
        cf = cf - 1;
        if (cf < 0) {
          cf = 0;
        };
        lcd.setCursor(col, 1);
        lcd.print(cf);
        delay(200);
      }
    }

    lcd.setCursor(col, 1);
    lcd.print(cf);

    bip();
    delay(500);
    col = col + 1;
    if ( EEPROM.read(i) != cf) {
      EEPROM.write(i, cf);
      bip();
    }
  }
number="";
  for ( int i = 60; i <= 68; i++) //читаем номер телефона
  {     number = number + String(EEPROM.read(i),DEC);
    delay(15);
  }

  number = "+380"+number;
  lcd.noCursor();
 // lcd.clear();
 // lcd.setCursor(0, 3);
// lcd.print(number);
 // delay(2000);
  lcd.clear();
  loopTime = 4000;
/*
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(L"ЗВОНИМ+SMS");
  lcd.setCursor(0, 1);
  lcd.print(number);


  //  String numT = "+380";
  //       numT=numT+number;
  //   String smsT = "temp:";
  //   smsT=smsT+temp;
  sms("Temp:", number); //отправляем СМС на номер
  delay(2000);
  GSMCall();

  lcd.clear();
  col = 5;
  scr();
  return;
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////
void menu_servo()
{
///                             01234567890123456789
lcd.setCursor(0, 1);lcd.print(L"СЕРВО        УЛИЦА");
lcd.setCursor(0, 2);lcd.print(L"Min          Min");
lcd.setCursor(0, 3);lcd.print(L"Max          Max");


 str1 = L"";
   str2 = L"";
   Min_Servo = vvod(str1,str2, Min_Servo,37,5,2);
  //  str1 = L"";
 //  str2 = L"";
   Max_Servo = vvod(str1,str2, Max_Servo,39,5,6);
//    str1 = L"";
//   str2 = L"";
   Min_Outdoors = vvod(str1,str2, Min_Outdoors,41,17,2);
//    str1 = L"";
//   str2 = L"";
   Max_Outdoors = vvod(str1,str2, Max_Outdoors,43,17,3);
delay(1000);

 //      EEPROM.get(37,Min_Servo);  
   //    EEPROM.get(39,Max_Servo);
   //    EEPROM.get(41,Min_Outdoors);
   //    EEPROM.get(43,Max_Outdoors);
  
}

/////////////////////////////////////////////////////////////////////////////////////////////
int obr_pr(int ob)
{   
  if( ob==0){return 255;}
 //Ob_litle=map(ob,1,99,150,0)
   return   map(ob, 0, 99, 180, 0);
 //;
  
  
}
///////////////////////////////////////////////////////////////////////////////////////////////
void WORK()
{
     lcd.setCursor(17, 1);  
     if(smoke >= stop_smoke)
     {
      if(temp>=Temp_Nasos_On){ D8_Low; lcd.print(L"ю"); } // включили насос
      if(temp<Temp_Nasos_On-1){D8_High; lcd.print(L"ы");} // выключили насос
     }else if (smoke < stop_smoke) {D8_High; lcd.print(L"ы");}

     
     lcd.setCursor(17, 2);
     if(Temp_Servo >= 25)
     {
      if(Temp_Teplic1<=Temp_Tepl1_Preset){ D0_Low; lcd.print(L"ю"); } // включили насос
      if(Temp_Teplic1>Temp_Tepl1_Preset){ D0_High; lcd.print(L"ы");} // выключили насос
     }else if (Temp_Servo < 25) {D0_High; lcd.print(L"ы");}  // сервоклапан остыл ниже 25 отключаем насос


     lcd.setCursor(17, 3);
     if(Temp_Servo >= 25)
     {
      if(Temp_Teplic2<=Temp_Tepl2_Preset){ D1_Low; lcd.print(L"ю"); } // включили насос
      if(Temp_Teplic2>Temp_Tepl2_Preset){ ;D1_High; lcd.print(L"ы");} // выключили насос
     }else if (Temp_Servo < 25){ D1_High; lcd.print(L"ы");} //сервоклапан остыл ниже 25 отключаем насос
     


     
switch (state) {
  
case 0:  /// текущий режим стоп
      Ob_litle=255; //255=мин. 0=макс 
     // Nasos_Kotel=255; //255=мин. 0=макс 
   //   Nasos_Kotel=255;//D8_Low;
     // tt=0;
    //  if (stop_smoke<smoke)
    //  { Nasos_Kotel=255; }// ОТКЛЮЧИЛИ НАСОС ПРИ ОСТЫВШЕМ ДЫМОХОДЕ.
    
      break;
     
case 1:   /// текущий режим розжиг
      obr_vent = obr;
      Ob_litle=obr_pr(obr_vent); //0=мин. 99=макс 
     //Nasos_Kotel=255; //255=мин. 0=макс
    //  if(temp>=Temp_Nasos_On){ Nasos_Kotel=0;D8_Low;lcd.setCursor(17, 1); lcd.print(L"ю"); } // включили насос
    //  if(temp<Temp_Nasos_On-1){ Nasos_Kotel=255;D8_High;lcd.setCursor(17, 1); lcd.print(L"ы");} // выключили насос
      break;

case 2:  /// текущий режим нагрев
     // if(temp>=Temp_Nasos_On){   Nasos_Kotel=0;D8_Low;lcd.setCursor(17, 2); lcd.print(L"ю"); } // включили насос
     // if(temp<Temp_Nasos_On-1){ Nasos_Kotel=255;D8_High;lcd.setCursor(17, 2); lcd.print(L"ы");} // выключили насос
      if (temp>=z_voda )
          {
           obr_vent=0;
           St_state = L"pause ";
           obr_vent=0;
           Ob_litle=255; //0=мин. 99=макс 
          }
       if (temp<z_voda )
         {
         obr_vent = obr;/////           34     30      35     95  0
         if(temp+5>z_voda){obr_vent=  map(temp, z_voda-6, z_voda, obr, 0);}
         if(smoke+20>z_maxsmoke) // перегрев котла
         {
          obr_vent=  map(smoke, z_maxsmoke-20, z_maxsmoke, obr, 0);
          if ( obr_vent<2){obr_vent=0;}
         }
          Ob_litle=obr_pr(obr_vent); //0=мин. 99=макс 
          St_state = L"НАГРЕВ";        
          }
      break;
      
case 3:   /// текущий режим затухание 
   Ob_litle=255; //0=мин. 99=макс 
  // Nasos_Kotel=255;//D8_High; //255=мин. 0=макс
   obr_vent = 0;
   time_rozjich++;
   if(time_rozjich > 50)
     { 
      scr();
      time_rozjich=0;
      }
if (stop_smoke<smoke)
      {   state=2; //нагрев
          St_state = L"НАГРЕВ"; 
        // Ob_litle=obr_pr(obr_vent); //0=мин. 99=макс 
     //if (Nasos_Kotel==255) { for (int i=0; i<=99; i=i+2){Nasos_Kotel=200-i*2;} };D8_High;// включили большой вент
      }
    break;
//default: 
  
}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void STATUS()
{
 switch (state) {
case 0:  /// текущий режим стоп
      state=1; //розжиг
      St_state = L"PO3Ж. ";
      lcd.setCursor(14, 0); lcd.print(St_state);
      time_rozjich=0;
      for (int i=0; i<=obr; i=i+5){Ob_litle=200-i*2;delay(100);
      lcd.setCursor(5, 0); obr_vent=i; lcd.print(obr_vent); if(obr_vent<10){lcd.print(" ");}}
      if( smoke > stop_smoke)
       {
        state=2; //нагрев
        St_state = L"HAГPEB";
        lcd.setCursor(14, 0); lcd.print(St_state);
        }
     break;
 case 1:   /// текущий режим розжиг
      obr_vent=0;
      state=0;   //стоп
      St_state = L" STOP ";
     break;

case 2:  /// текущий режим нагрев
      obr_vent=0;
      state=0;  //стоп
      St_state = L" STOP ";
     break;
case 3:   /// текущий режим затухание 
      state=0;  //стоп
      St_state = L" STOP ";
    break;
//default: 
 }
    //EEPROM.put(2,state);
/// lcd.setCursor(14, 0); lcd.print(St_state); // потом
}


////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------
void menu()  
{


    loopTime = 5000;

   str1 = L"МАКС.ОБР";
   str2 = L"ВЕНТ";
   obr = vvod(str1,str2, obr,15,12,1);
   
   str1 = L"ТЕМП.ВКЛ";
   str2 = L"НАСОС КОТЛА";
   Temp_Nasos_On = vvod(str1,str2, Temp_Nasos_On,12,12,1);
   
   str1 = L"ГИСТ.ТЕМ-РЫ";
   str2 = L"НА КОТЛЕ:";
   deltavoda = vvod(str1,str2, deltavoda,10,12,1);
   
   str1 = L"MAKC.ТЕМ-РА";
   str2 = L"ДЫМ";
   z_maxsmoke = vvod(str1,str2, z_maxsmoke,20,12,1);
 
   str1 = L"ТЕМП.ОТК";
   str2 = L"КОТЛА(ДЫМ)";
   stop_smoke = vvod(str1,str2, stop_smoke,27,12,1);

   
    str1 = L"ТЕМП.";
   str2 = L"TEПЛ1";
   Temp_Tepl1_Preset = vvod(str1,str2, Temp_Tepl1_Preset,31,12,1);
   
   str1 = L"ТЕМП.";
   str2 = L"TEПЛ2";
   Temp_Tepl2_Preset = vvod(str1,str2, Temp_Tepl2_Preset,33,12,1);


// str1 = L"ГИСТ.ТЕМ-РЫ";
 //  str2 = L"НАСОС TEПЛ";
 //  Delta_Temp_Teplic = vvod(str1,str2, Delta_Temp_Teplic,35,12,1);

   str1 = L"KOPPЕКТ.ТЕМП";
   str2 = L"TEПЛИЦА1";
   Temp_Tepl1_Correct = vvod(str1,str2, Temp_Tepl1_Correct,45,12,1);
   
   str1 = L"KOPPЕКТ.ТЕМП";
   str2 = L"TEПЛ2";
   Temp_Tepl2_Correct = vvod(str1,str2, Temp_Tepl2_Correct,47,12,1);


    str1 = L"ТЕМ-РА АВАРИЯ";
    str2 = L"TEПЛ1";
    Temp_Tepl1_Alarm = vvod(str1,str2, Temp_Tepl1_Alarm,49,12,1);
   
    str1 = L"ТЕМ-РА АВАРИЯ";
    str2 = L"TEПЛ2";
    Temp_Tepl2_Alarm = vvod(str1,str2, Temp_Tepl2_Alarm,51,12,1);
   


 
  
   loopTime = 3000;
   
 scr();

}  

///////////////////////////////////////////////////////////////////////////////////////
int vvod(wchar_t *t1,wchar_t *t2, int v,byte e,byte x,byte y)  // функция ввода параметров основного меню
{ 
   int tp=0;
  EEPROM.get(e,tp); // запоминаем  начальное значение 
   delay(100);
     if(x==12) lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print(t1);
      lcd.setCursor(0,1);
      lcd.print(t2);
      lcd.setCursor(x,y); lcd.print(v);
    currentMillis = millis();
    while(millis()-currentMillis<=loopTime)
     {
      if (analogRead(keyDow) < 100 ) { bip();v=v-1;currentMillis = millis(); /*if(v<0&&x==12){v=0;}*/lcd.setCursor(x,y); lcd.print(v);lcd.print(L"  "); delay(100);}
      if (analogRead(keyUp) < 100 ) { bip();v=v+1; currentMillis = millis();/*if(v>500){v=500;} if (e==15||e==12||e==10){if(v>99){v=99;}}*/ lcd.setCursor(x,y); lcd.print(v);lcd.print(L"  ");delay(100);} 
       if (analogRead(keySel) < 100 ) {bip();bip();currentMillis = 0; lcd.setCursor(x,y); lcd.print(v);lcd.print(L"  ");delay(100);}
     if (e==15){Ob_litle=map(v,1,99,180,0);}
      }
       if (e==15){Ob_litle=obr_pr(obr_vent);}
     if( v != tp) { EEPROM.put(e,v);bip();} // если значение поменялось записываем в еером
    if(x==12) lcd.clear(); 
 return v;
}




//-------------------------------------------------
void scr()
{

 lcd.setCursor(12, 1);
 lcd.print(L"KOTEL"); 
 lcd.setCursor(12, 2);
 lcd.print(L"TEPL1");   
 lcd.setCursor(12, 3);
 lcd.print(L"TEPL2");   
 lcd.setCursor(0, 0);
  lcd.print(L"BEHT");  
 lcd.setCursor(0, 1);
 lcd.print(L"ДЫМ");  
  lcd.setCursor(0, 2);
  lcd.print(L"Servo "); 
 lcd.setCursor(0, 3);
  lcd.print(L"t");
  lcd.print("\xBF"); lcd.print("\xDF"); lcd.print(Temp_Outdoors); 
  
}
//-------------------------------------------------------------------------------------------------

void bip() {
  
 tone(4,2400); delay(50);  noTone(4);;
  }


///////////////////////////////////////////////////////////////////////////////////////////////
//********************обработчики прерываний*******************************
void halfcycle()  //прерывания таймера
{ 
  tic++;  //счетчик  
  if(Ob_litle < tic ) D6_High;  //управляем выходом вентилятора котла
}

void  detect_up()  // обработка внешнего прерывания. Сработает по переднему фронту
{  
 tic=0;             //обнулить счетчик
 ResumeTimer1();   //запустить таймер
 attachInterrupt(1, detect_down, HIGH);  //перепрограммировать прерывание на другой обработчик
}  

void  detect_down()  // обработка внешнего прерывания. Сработает по заднему фронту
{   
 StopTimer1(); //остановить таймер
 D6_Low; //логический ноль на выходы
 tic=0;       //обнулить счетчик
 attachInterrupt(1, detect_up, LOW); //перепрограммировать прерывание на другой обработчик
} 
//*************************************************************************



void sms(String text) //процедура отправки СМС
{ val = "";
 lcd.setCursor(14, 0); lcd.print("SMS->  ");
 
  buffcle(); buffcle();
delay(1000);
 
  // lcd.setCursor(0, 1);
 // lcd.print(phone);
//6  mySerial.println("AT+CMGS=\"" + phone + "\"");
   mySerial.println("AT+CMGS=\"" + number + "\"");
  delay(1000);

  mySerial.print(text);
  delay(500);

 
  mySerial.print(" Kotel:");
  delay(500);
  mySerial.print(temp);
  delay(500);

 
  mySerial.print(" TEPL1:");
  delay(500);
  mySerial.print(Temp_Teplic1);
  delay(500);
  mySerial.print(" TEPL2:");
  delay(500);
  mySerial.print(Temp_Teplic2);
  delay(500);
  mySerial.print(" SERVO:");
  delay(500);
  mySerial.print(Temp_Servo);
  delay(500);
  mySerial.print(" ULICA:");
  delay(500);
  mySerial.print(Temp_Outdoors);
  delay(500);
 //mySerial.print((char)26);
 // delay(500);
  
 // text = " Status:NAGREV* ";
 // if (state == 0) {
  //  text = " Status:STOP ";
 // }
//  if (state == 1) {
//    text = " Status:NAGREV ";
//  }
//  if (state == 3) {
//    text = " Status:AVARIA ";
//  }
//  mySerial.print(text);
 // delay(1000);

 // mySerial.print((char)26);
 // delay(1500);
  
   mySerial.print((char)26);
  delay(5000);

}

void GSMCall()
{ 

  lcd.setCursor(14, 0); lcd.print("Call->");
  delay(200);
  mySerial.println("ATD+380" + number + ";");

}



void buffcle()
{
  while (mySerial.available()) { //сохраняем входную строку в переменную val
    val = mySerial.read();
    delay(10);
  }  delay(100);
  val = "";
}
