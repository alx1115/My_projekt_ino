// проект автозапуск бензинового генератора на ARDURINO Ver 090717

//задаю константы

#define city 12                      //напряжение из города

#define he_voltage 11                //напряжение на генераторе




#define starter_relay 7              //реле стартера

#define magnetic 5                   //включает пускатель

#define disconnection 6             //реле датчика масла


#define choke_on 8                   //подсос включить

#define choke_of 9                   //подсос выключить


// задаю переменные

bool t = 0; //отображает работу генератора
bool m = 0; //отображает наличие напряжения в сети
byte a = 0; //количество попыток запуска ген
int val = 0;  // переменная для хранения считываемого значения с
int g = 0; //количество команд на пускатель
byte regum = 0;     // 0 - робота від мережі
short int proces = 0;
// 1 - процес заведення
// 2 - робота від генератора
// 3 - процес вимкнення генератора і перехід на роботу від мережі

unsigned long globalDelay = millis(), millisproces = millis(), waitStart;

void setup()
{
  pinMode(city, INPUT_PULLUP);         // назначаем  входом  напряжение из города  в схеме

  pinMode(he_voltage, INPUT_PULLUP);         // назначаем the_voltage входом  напряжение с генератора

  pinMode(starter_relay, OUTPUT);         //назначаем  starter_relay выходом реле стартера

  pinMode(choke_on, OUTPUT);         //назначаем choke_on выходом подсос включает

  pinMode(choke_of, OUTPUT);         //назначаем choke_of выходом подсос вЫключает

  pinMode(magnetic, OUTPUT);         //назначаем magnetic выходом переключает пускатели на генератор

  pinMode(disconnection, OUTPUT);         //назначаем disconnection выходом включает и выключает  зажигание

  Serial.begin(9600);              //  установка связи по serial
  digitalWrite(magnetic, HIGH);
  digitalWrite(disconnection, HIGH);
  digitalWrite(starter_relay, HIGH);
}

void loop()
{
  if (millis() - globalDelay >= 1000) {
    globalDelay = millis();
    m =  !digitalRead(city);
    t =  !digitalRead(he_voltage);
    Serial.print(m); Serial.print("  "); Serial.println(t);

    if (regum == 0) {                         //події підчас нормальної роботи генератора
      if (m == LOW && t == LOW && a != 6)
      {
        regum = 1;
        a = 0;
        proces = 0;
      }
      if (m == HIGH) {
        a = 0;
      }
      if (millis() - millisproces >= 5000) {
        millisproces = millis();
        digitalWrite(disconnection, HIGH);
      }




    } else if (regum == 1) {                              //Завод генератора
      if (a == 0) {
        delay(10000);
        if (m == HIGH && t == LOW) {
          regum = 0;
          return 0;
        }
        a++;
      }
      if (m == HIGH && t == LOW) {
        regum = 3;
      }

      if (waitStart - millis() >= 30 * 1000 || a == 1) {

        if (proces == 0) {
          digitalWrite(disconnection, LOW);
          proces++;
          millisproces = millis();
          Serial.println("0");
        } else if (proces == 1 && millis() - millisproces >= 700) {
          digitalWrite(choke_on, HIGH);
          proces++;
          millisproces = millis();
          Serial.println("1");
        } else if (proces == 2 && millis() - millisproces >= 1000) {
          digitalWrite(starter_relay, LOW);
          proces++;
          millisproces = millis();
          Serial.println("2");
        } else if (proces == 3 && ((millis() - millisproces >= 2000) || t == HIGH)) {

          digitalWrite(starter_relay, HIGH);
          proces++;
          millisproces = millis();
          Serial.println("3"); 
          digitalWrite(choke_on, LOW);
        } else if (proces == 4 && millis() - millisproces >= 500) {

          Serial.println("4");

          digitalWrite(choke_of, HIGH);

          proces++;
          millisproces = millis();
        } else if (proces == 5 && millis() - millisproces >= 500) {
          digitalWrite(choke_of, LOW);
          Serial.println("5");
          proces++;
          millisproces = millis();
          if (m == LOW  && t == HIGH) {
            regum = 2;
          }
        } else if (millis() - millisproces >= 10000) {
          proces = 0;
          digitalWrite(choke_on, LOW);
          Serial.print("choke_on, LOW2");
          millisproces = millis();
          waitStart = millis();
          a++;
          Serial.println("-1");
        }
        if (a == 6) {
          regum = 0;
        }
      }




    } else if (regum == 2) {
      if (m == HIGH  && t == HIGH) {
        regum = 3;
        millisproces = millis();
      }
      if (m == LOW && t == LOW)
      {
        digitalWrite(magnetic, HIGH);
        regum = 1;
        a = 0;
        proces = 0;
      }
      if (millis() - millisproces > 30000) {
        digitalWrite(magnetic, LOW);
        millisproces = millis();
      }




    } else if (regum == 3) {
      if (millis() - millisproces > 10000) {
        digitalWrite(magnetic, HIGH);
        millisproces = millis();
        millisproces = millis();
        if (m == HIGH  && t == HIGH) {
          regum = 0;
        }
      }
    }
  }
}
