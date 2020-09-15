#define NUM_LEDS 60         // число МИКРОСХЕМ на ленте
#define BTN_TIMEOUT 800     // задержка кнопки для удерживания (миллисекунды)
#define BRIGHTNESS 100      // максимальная яркость ленты (0 - 255)

#define SWING_TIMEOUT 500   // таймаут между двумя взмахами
#define SWING_L_THR 150     // порог угловой скорости для взмаха
#define SWING_THR 300       // порог угловой скорости для сильного взмаха
#define STRIKE_THR 150      // порог ускорения для распознавания удара
#define STRIKE_S_THR 320    // порог ускорения для распознавания сильного удара
#define FLASH_DELAY 80      // время вспышки при ударе (миллисекунды)

#define BLINK_ALLOW 1       // разрешить мерцание (1 - разрешить, 0 - запретить)
#define BLINK_AMPL 20       // амплитуда мерцания клинка
#define BLINK_DELAY 30      // задержка между мерцаниями

#define R1 100000           // сопротивление резистора делителя    
#define R2 51000            // сопротивление резистора делителя
#define BATTERY_SAFE 0      // не включаться и выключаться при низком заряде АКБ (1 - разрешить, 0 - запретить)

#define DEBUG 1            // вывод в порт отладочной информации (1 - разрешить, 0 - запретить)
// ---------------------------- НАСТРОЙКИ -------------------------------

#define LED_PIN 6           // пин, куда подключен DIN ленты
#define BTN 3               // пин кнопки
#define VOLT_PIN A6         // пин вольтметра


#include <avr/pgmspace.h>   
#include <SD.h>             
#include <TMRpcm.h>         
#include "Wire.h"           
#include "I2Cdev.h"         
#include "MPU6050.h"        
#include <toneAC.h>         
#include "FastLED.h"        
#include <EEPROM.h>         


CRGB leds[NUM_LEDS];
#define SD_ChipSelectPin 10
TMRpcm tmrpcm;
MPU6050 accelgyro;


// ------------------------------ ПЕРЕМЕННЫЕ ---------------------------------
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
float k = 0.2;
unsigned long humTimer = -9000, mpuTimer, nowTimer;
int stopTimer;
boolean bzzz_flag, ls_chg_state, ls_state;
boolean btnState, btn_flag, hold_flag;
byte btn_counter;
unsigned long btn_timer, blink_timer, swing_timer, swing_timeout, battery_timer, bzzTimer;
byte nowNumber;
byte LEDcolor;  // 0 - красный, 1 - синий, 2 - зелёный, 3 - розовый, 4 - жёлтый
byte nowColor, red, green, blue, redOffset, greenOffset, blueOffset;
boolean eeprom_flag, swing_flag, swing_allow, strike_flag, HUMmode;
float voltage;
int blinkOffset;
// ------------------------------ ПЕРЕМЕННЫЕ ---------------------------------

// --------------------------------- ЗВУКИ УДАРОВ ----------------------------------
const char strike1[] PROGMEM = "SK1.wav";
const char strike2[] PROGMEM = "SK2.wav";
const char strike3[] PROGMEM = "SK3.wav";
const char strike4[] PROGMEM = "SK4.wav";
const char strike5[] PROGMEM = "SK5.wav";
const char strike6[] PROGMEM = "SK6.wav";
const char strike7[] PROGMEM = "SK7.wav";
const char strike8[] PROGMEM = "SK8.wav";

const char* const strikes[] PROGMEM  = {strike1, strike2, strike3, strike4, strike5, strike6, strike7, strike8
};

int strike_time[8] = {779, 563, 687, 702, 673, 661, 666, 635};

const char strike_s1[] PROGMEM = "SKS1.wav";
const char strike_s2[] PROGMEM = "SKS2.wav";
const char strike_s3[] PROGMEM = "SKS3.wav";
const char strike_s4[] PROGMEM = "SKS4.wav";
const char strike_s5[] PROGMEM = "SKS5.wav";
const char strike_s6[] PROGMEM = "SKS6.wav";
const char strike_s7[] PROGMEM = "SKS7.wav";
const char strike_s8[] PROGMEM = "SKS8.wav";

const char* const strikes_short[] PROGMEM = {        
  strike_s1, strike_s2, strike_s3, strike_s4,
  strike_s5, strike_s6, strike_s7, strike_s8
};
int strike_s_time[8] = {270, 167, 186, 250, 252, 255, 250, 238};

const char swing1[] PROGMEM = "SWS1.wav";
const char swing2[] PROGMEM = "SWS2.wav";
const char swing3[] PROGMEM = "SWS3.wav";
const char swing4[] PROGMEM = "SWS4.wav";
const char swing5[] PROGMEM = "SWS5.wav";

const char* const swings[] PROGMEM  = {swing1, swing2, swing3, swing4, swing5
};
int swing_time[8] = {389, 372, 360, 366, 337};

const char swingL1[] PROGMEM = "SWL1.wav";
const char swingL2[] PROGMEM = "SWL2.wav";
const char swingL3[] PROGMEM = "SWL3.wav";
const char swingL4[] PROGMEM = "SWL4.wav";

const char* const swings_L[] PROGMEM  = {        
  swingL1, swingL2, swingL3, swingL4
};
int swing_time_L[8] = {636, 441, 772, 702};

char BUFFER[10];


void setup() {
  if (SD.begin(8)) Serial.println(F("SD OK"));
  FastLED.addLeds<WS2813, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);  
  setAll(0, 0, 0);             

  Wire.begin();
  Serial.begin(9600);

 
  pinMode(BTN, INPUT_PULLUP);


  randomSeed(analogRead(2)); 


  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  if (DEBUG) {
    if (accelgyro.testConnection()) Serial.println(F("MPU6050 OK"));
    else Serial.println(F("MPU6050 fail"));
  }


  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(3);
  tmrpcm.quality(1);
  if (DEBUG) {
    if (SD.begin(8)) Serial.println(F("SD OK"));
    else Serial.println(F("SD fail"));
  } else {
    SD.begin(8);
  }

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  
    nowColor = EEPROM.read(0);  
    HUMmode = EEPROM.read(1);
    Serial.println(F("ok"));
  } else {                       
    EEPROM.write(0, 0);          
    EEPROM.write(1, 0);          
    nowColor = 0;                
    Serial.println(F("fail"));
  }

  setColor(nowColor);                      
  byte capacity = voltage_measure();       
  capacity = map(capacity, 100, 0, (NUM_LEDS / 2 - 1), 1);  
  if (DEBUG) {
    Serial.print(F("Battery: "));
    Serial.println(capacity);
  }
  Serial.println(F("ll"));


  setAll(255, 255, 255);
  delay(1000);                         
  setAll(0, 0, 0);
    Serial.println(F("kkk"));
  FastLED.setBrightness(BRIGHTNESS);   
}

void loop() {
  randomBlink();      
  getFreq();          
  on_off_sound();    
  btnTick();          
  strikeTick();       
  swingTick();        
  batteryTick();      
}

void btnTick() {
  btnState = !digitalRead(BTN);    
  if (btnState && !btn_flag) {
    if (DEBUG) Serial.println(F("BTN PRESS"));
    btn_flag = 1;
    btn_counter++;                 
    btn_timer = millis();
  }
  if (!btnState && btn_flag) {     
    btn_flag = 0;
    hold_flag = 0;                 
  }

  if (btn_flag && btnState && (millis() - btn_timer > BTN_TIMEOUT) && !hold_flag) {
    ls_chg_state = 1;     
    hold_flag = 1;
    btn_counter = 0;
  }
  
  if ((millis() - btn_timer > BTN_TIMEOUT) && (btn_counter != 0)) {
    if (ls_state) {
      if (btn_counter == 3) {              
        nowColor++;                         
        if (nowColor >= 6) nowColor = 0;    
        setColor(nowColor);                 
        setAll(red, green, blue);           
        eeprom_flag = 1;                    
      }
      if (btn_counter == 5) {               
        HUMmode = !HUMmode;
        if (HUMmode) {
          noToneAC();                       
          tmrpcm.play("HUM.wav");           
        } else {
          tmrpcm.disable();                 
          toneAC(freq_f);                   
        }
        eeprom_flag = 1;                    
      }
    }
    btn_counter = 0;
  }
}

void on_off_sound() {                
  if (ls_chg_state) {                
    if (!ls_state) {                 
      if (voltage_measure() > 10 || !BATTERY_SAFE) {
        if (DEBUG) Serial.println(F("SABER ON"));
        tmrpcm.play("ON.wav");         
        delay(200);                    
        light_up();                    
        delay(200);                   
        bzzz_flag = 1;                 
        ls_state = true;              
        if (HUMmode) {
          noToneAC();                 
          tmrpcm.play("HUM.wav");      
        } else {
          tmrpcm.disable();           
          toneAC(freq_f);             
        }
      } else {
        if (DEBUG) Serial.println(F("LOW VOLTAGE!"));
        for (int i = 0; i < 5; i++) {

        }
      }
    } else {                         
      noToneAC();                    
      bzzz_flag = 0;                
      tmrpcm.play("OFF.wav");       
      delay(300);                    
      light_down();                
      delay(300);                    
      tmrpcm.disable();              
      if (DEBUG) Serial.println(F("SABER OFF"));
      ls_state = false;              
      if (eeprom_flag) {            
        eeprom_flag = 0;
        EEPROM.write(0, nowColor);   
        EEPROM.write(1, HUMmode);    
      }
    }
    ls_chg_state = 0;                
  }

  if (((millis() - humTimer) > 9000) && bzzz_flag && HUMmode) {   
    tmrpcm.play("HUM.wav");
    humTimer = millis();                                         
    swing_flag = 1;
    strike_flag = 0;
  }
  long delta = millis() - bzzTimer;
  if ((delta > 3) && bzzz_flag && !HUMmode) {   
    if (strike_flag) {
      tmrpcm.disable();                             
      strike_flag = 0;
    }
    toneAC(freq_f);                                 
    bzzTimer = millis();                           
  }
}

void randomBlink() {
  if (BLINK_ALLOW && ls_state && (millis() - blink_timer > BLINK_DELAY)) {
    blink_timer = millis();
    blinkOffset = blinkOffset * k + random(-BLINK_AMPL, BLINK_AMPL) * (1 - k);
    if (nowColor == 0) blinkOffset = constrain(blinkOffset, -15, 5);
    redOffset = constrain(red + blinkOffset, 0, 255);
    greenOffset = constrain(green + blinkOffset, 0, 255);
    blueOffset = constrain(blue + blinkOffset, 0, 255);
    setAll(redOffset, greenOffset, blueOffset);
  }
}

void strikeTick() {
  if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR)) {     
    if (!HUMmode) noToneAC();                      
    nowNumber = random(8);             
    strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes_short[nowNumber])));
    tmrpcm.play(BUFFER);              
    strike_flash();
    if (!HUMmode)
      bzzTimer = millis() + strike_s_time[nowNumber] - FLASH_DELAY;
    else
      humTimer = millis() - 9000 + strike_s_time[nowNumber] - FLASH_DELAY;
    strike_flag = 1;
  }
  if (ACC >= STRIKE_S_THR) {           
    if (!HUMmode) noToneAC();                        
    nowNumber = random(8);             
    strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes[nowNumber])));
    tmrpcm.play(BUFFER);               
    strike_flash();
    if (!HUMmode)
      bzzTimer = millis() + strike_time[nowNumber] - FLASH_DELAY;
    else
      humTimer = millis() - 9000 + strike_time[nowNumber] - FLASH_DELAY;
    strike_flag = 1;
  }
}

void swingTick() {
  if (GYR > 80 && (millis() - swing_timeout > 100) && HUMmode) {
    swing_timeout = millis();
    if (((millis() - swing_timer) > SWING_TIMEOUT) && swing_flag && !strike_flag) {
      if (GYR >= SWING_THR) {      
        nowNumber = random(5);             
        strcpy_P(BUFFER, (char*)pgm_read_word(&(swings[nowNumber])));
        tmrpcm.play(BUFFER);            
        humTimer = millis() - 9000 + swing_time[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
      if ((GYR > SWING_L_THR) && (GYR < SWING_THR)) {
        nowNumber = random(5);            
        strcpy_P(BUFFER, (char*)pgm_read_word(&(swings_L[nowNumber])));
        tmrpcm.play(BUFFER);               
        humTimer = millis() - 9000 + swing_time_L[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
    }
  }
}

void getFreq() {
  if (ls_state) {                                               
    if (millis() - mpuTimer > 500) {                            
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       
      gyroX = abs(gx / 100);
      gyroY = abs(gy / 100);
      gyroZ = abs(gz / 100);
      accelX = abs(ax / 100);
      accelY = abs(ay / 100);
      accelZ = abs(az / 100);
      ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
      ACC = sqrt(ACC);
      GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
      GYR = sqrt((long)GYR);
      COMPL = ACC + GYR;
      freq = (long)COMPL * COMPL / 1500;                        
      freq = constrain(freq, 18, 300);                          
      freq_f = freq * k + freq_f * (1 - k);                     
      mpuTimer = micros();                                      
    }
  }
}

// функция выбора цвета одного светодиода
void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}


void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}


void light_up() {
  for (char i = 0; i <= (NUM_LEDS / 2 - 1); i++) {          
    setPixel(i, red, green, blue);
    setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(25);
  }
}

// плавное выключение меча
void light_down() {
  for (char i = (NUM_LEDS / 2 - 1); i >= 0; i--) {      
    setPixel(i, 0, 0, 0);
    setPixel((NUM_LEDS - 1 - i), 0, 0, 0);
    FastLED.show();
    delay(25);
  }
}

// моргание при ударе
void strike_flash() {
  setAll(255, 255, 255);             
  delay(FLASH_DELAY);                
  setAll(red, blue, green);          
}

// выбор цвета
void setColor(byte color) {
  switch (color) {
    // 0 - красный, 1 - синий, 2 - зелёный, 3 - розовый, 4 - жёлтый, 5 - голубой не хотим играть с тобой
    case 0:
      red = 255;
      green = 0;
      blue = 0;
      break;
    case 1:
      red = 0;
      green = 0;
      blue = 255;
      break;
    case 2:
      red = 0;
      green = 255;
      blue = 0;
      break;
    case 3:
      red = 255;
      green = 0;
      blue = 255;
      break;
    case 4:
      red = 255;
      green = 255;
      blue = 0;
      break;
    case 5:
      red = 0;
      green = 255;
      blue = 255;
      break;
  }
}

// проверка акума
void batteryTick() {
  if (millis() - battery_timer > 30000 && ls_state && BATTERY_SAFE) {
    if (voltage_measure() < 15) {
      ls_chg_state = 1;
    }
    battery_timer = millis();
  }
}


byte voltage_measure() {
  voltage = 0;
  for (int i = 0; i < 10; i++) {    
    voltage += (float)analogRead(VOLT_PIN) * 5 / 1023 * (R1 + R2) / R2;
  }
  voltage = voltage / 10;           
  int volts = voltage / 3 * 100;    
  if (volts > 387)
    return map(volts, 420, 387, 100, 77);
  else if ((volts <= 387) && (volts > 375) )
    return map(volts, 387, 375, 77, 54);
  else if ((volts <= 375) && (volts > 368) )
    return map(volts, 375, 368, 54, 31);
  else if ((volts <= 368) && (volts > 340) )
    return map(volts, 368, 340, 31, 8);
  else if (volts <= 340)
    return map(volts, 340, 260, 8, 0);
}
