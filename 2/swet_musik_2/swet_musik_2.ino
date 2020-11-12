#include <FastLED.h>
#include <SoftwareSerial.h>
#include "GyverButton.h"


///////////////////////////
#define ADC_PIN A0
#define PIN_IR A1

// Указываем, какое количество пикселей у нашей ленты.
#define LED_COUNT 60
// Указываем, к какому порту подключен вход ленты DIN.
#define LED_PIN 4
// Создаем переменную strip для управления нашей лентой.


#define sample 300 //выборка для анализа
CRGB strip[LED_COUNT];

///////////////переменные//////////
int val = 0;
int max_max = 0;
int min_min = 0;
float fil_max = 0, fil_min = 0;
int count = 0;
int inkr = 0; //переменная для смены цвета
int inkr_1 = 85; //переменная для смены цвета
int inkr_2 = 170; //переменная для смены цвета

//////////////////////////////////
char flag_rezhim_1 = 0; //переменная для запуска умного переключения режимов
int ircomand = 0; //режим работы
////////Настройка///////
int gLedPin = 13;
int gRxPin = 12;
int gTxPin = 10;
int sens_butt_pin = 8;
//GButton butt1(8);
SoftwareSerial BTSerial(gRxPin, gTxPin);


/*
    массив который отвечает за режим работы лампы
    0й элемент - Вкл/Выкл
    1й элемент - Режим работы
    2й элемент - Яркость (0...1)
    3й элемент - Цвет R
    4й элемент - Цвет G
    5й элемент - Цвет B
*/
int mode[6];
int pars_bluet();
void setup() {
  // butt1.setDebounce(0);

  mode[0] = 1; //включаем лампу
  mode[1] = 0; //режим работы
  mode[2] = 100; //яркость

  // Добавляем ленту
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(strip, LED_COUNT);
  delay (20);
  ircomand = random(1, 4);
  flag_rezhim_1 = 1;

  pinMode(sens_butt_pin, INPUT);
 pinMode(ADC_PIN, INPUT);


  Serial.begin (9600);

  //  # 38400 - для метода №1, 9600 - для метода №2
  pinMode(gRxPin, INPUT);
  pinMode(gTxPin, OUTPUT);
  BTSerial.begin(38400);
  delay(500);
}
/////////////////////

byte counter;
byte counter_click = 0;
int dfg = 0;
int flag_rezim = 0;

byte count_skatch = 0; //как часто звук опускается
byte count_skatch_fil = 0; //как часто звук опускается после фильтра

char flag_skatch = 0; //пока звук внизу, что бы не былт инкремента

//переменные для сенсорного управления
int sens_test = 0;
int test_increment = 0;
bool click_1 = 0;
bool click_2 = 0;
bool click_long = 0;
////////////////////////
//двойное нажатие
static int double_click = 0;


//////////////////Цикл/////////////
//Частота оптытным путем примерно 25Гц
void loop() {
  //обработка нажатий на сенсорную кнопку
  test_increment++;
  //if (test_increment==100) test_increment=0;
  if (digitalRead(sens_butt_pin)) {
    sens_test++;
  }
  else if (0 < sens_test && sens_test < 5) {
    if (2 < test_increment && test_increment < 6)
    {
      click_2 = 1;
      if (click_2) {
        Serial.println("double_click");
        //        click_2 = 0;
      }
      sens_test = 0;
    }
    else if (15 < test_increment ) {
      click_1 = 1;
      if (click_1) {
        Serial.println("click");
        //click_1 = 0;
      }
      sens_test = 0;
      test_increment = 0;
    }
  }
  else if (5 < sens_test) {
    click_long = 1;
    if (click_long) {
      Serial.println("click_long");
      //click_long = 0;
    }
    sens_test = 0;
  }
  //////////////////////////

  //Вывод блютуз сообщений

  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }

  /////////////////////////////


  //
  int max_now = 0;
  int min_now = 1023;
  int numb_LED = 0;
  double granic = 0; //определяем границы громкости
  double volume = 0; //разница м/у нижн. границей и текущим звуком
  double yark = 0;//яркость свечения светодиода 0,,255
  static double yark_fil = 0;


  int delta_PI = 0;
  int grad = 0;
  if (count == 49) dfg = random(0, 6);

  ///работа с микро и определение количеств азаженных диодов
  for (int i = 0;  i < sample; i++)
  {
    val = analogRead(ADC_PIN);
    if ( val > max_now) max_now = val;
    if ( val < min_now) min_now = val;
  }
  count++;
  inkr++;
  inkr_1++;
  inkr_2++;
  if (inkr > 255)inkr = random(0, 255);
  if (inkr_1 > 255)inkr_1 = random(0, 255);
  if (inkr_2 > 255)inkr_2 = random(0, 255);

  if (count == 50)
  {
    max_max = 0;
    min_min = 1023;
    count = 0;
  }
  if ( max_now > max_max) max_max = max_now;
  if ( max_now < min_min) min_min = max_now;
  fil_max += (max_max - fil_max) * 0.05;
  fil_min += (min_min - fil_min) * 0.05;



  granic = fil_max - fil_min; //находим разницу между верхом и низом границ громкости
  volume = max_now - fil_min; //находим разницу между текущим сигналом и низом границы, для дальнейшего сравнения с разницей границ
  yark = (volume / granic);
   // Serial.println (fil_max);
    
    //Serial.println (val); 
   /// Serial.println (yark);

  if (yark < 0.01) yark = 0.01;
  if (yark > 1) yark = 1;
  yark_fil += (yark - yark_fil) * 0.4;
  yark = yark_fil * 1.2;
  if (yark < 0.01) yark = 0.01;
  if (yark > 1) yark = 1;

  if (yark < 0.2) //если звук опустился ниже 0,2 от максимума, то менять цвет
  {
    if (flag_skatch == 0)
    {
      inkr = random(0, 255);
      inkr_1 = random(0, 255);
      inkr_2 = random(0, 255);
      count_skatch++;
      flag_skatch++;
    }
  }
  else flag_skatch = 0;
  static int fgh = 0;
  //////////////////////////
  static float yark_LED;
  yark_LED = (float)mode[2] / 100.0;
  // yark_LED = (yark_LED / 100);
  if (BTSerial.available()) {

    pars_bluet();
    //    Serial.println (yark_LED);
    Serial.println (double_click);
  }

  if (mode[0] == 1) {
    mode[2] = 100;


  } else if (mode[0] == 0) {
    mode[2] = 0;
  }

  /// Блок с режимами работы светодиодной ленты
  //переключение режимов при нажатии на сенсорную кнопку
  //длительное нажатие
  if (click_long == 1) //при длительном нажатии либо включаем, либо выключаем лампу
  {
    if (mode[0]) {
      mode[2] = 0;
      mode[0] = 0;
      //      Serial.println("off");
      //      Serial.println(mode[2]);
      click_long = 0;
    } else {
      mode[2] = 100;
      mode[0] = 1;
      //      Serial.println("on");
      //      Serial.println(mode[2]);

      click_long = 0;
    }
  }// готово

  if (click_2 == 1) //при двойном нажатии меняем режим работы лампы
  {
    double_click++;
    if (double_click == 4) {
      double_click = 0;
      ircomand=1;
    }
    Serial.println (double_click);
    click_2 = 0;
  } // готово
  //нажатие (подумать с задержкой для двойного нажатия)
  if (click_1 == 1) //при длительном нажатии либо включаем, либо выключаем лампу
  {
    ircomand++;
    if (ircomand > 5) ircomand = 1;
    counter_click += 10;
    click_1 = 0;
  }
  switch (double_click)
  {
    case (0):
      flag_rezhim_1 = 0;
      break;
    case (1):
      flag_rezhim_1 = 1;
      break;
    case (2):
      flag_rezhim_1 = 0;
      ircomand = 6;
      break;
    case (3):
      flag_rezhim_1 = 0;
      ircomand = 7;
      break;
  }

  numb_LED = yark * LED_COUNT;
  // Включаем все светодиоды



  if (flag_rezhim_1 == 1)
  {

    if (flag_rezim < 1300 / 5) //примерно 5 раз в минуту сюда заходит
    {
      //раз в две секунды - раз в три: первый режим
      // раз в четыре секунды - раз в пять: второй режим
      // раз в пять секунды - раз в семь: третий(радуга) режим
      flag_rezim++;
      if (count_skatch_fil > 55 / 3) ircomand = 1;
      if ( count_skatch_fil > 25 / 3 && count_skatch_fil < 55 / 3) ircomand = 2;
      if ( count_skatch_fil > 15 / 3 && count_skatch_fil < 25 / 3)ircomand = 4 ;
      if (  count_skatch_fil < 15 / 3)  ircomand = 3;
    }
    else {

      flag_rezim = 0;
      count_skatch_fil += (count_skatch - count_skatch_fil) * 0.7;
      //      Serial.print (count_skatch);
      //      Serial.print (',');
      //      Serial.println (count_skatch_fil);
      count_skatch = 0;
    }
  }
  //градиент из середины
  if (ircomand == 4) // при тишине горит при звуках тухнет
  {
    for (int i = LED_COUNT / 2; i < LED_COUNT - numb_LED / 2; i++)
    {
      //int cvet = 30-i * 2;//рандомная двухцветная хуета
      int cvet = (i - LED_COUNT / 2) * 2.5;
      //      strip[i] = CHSV(inkr_2, 255, 255);  // HSV. Увеличивать HUE (цвет)

      //////ПРИКОЛЬНЫЙ ГРАДИЕНТ
      switch (dfg) {
        case 0: //красный
          strip[i] = SetColor(255 * yark_LED ,  cvet * yark_LED ,  cvet * yark_LED ); // Красный цвет.
          //  Serial.println (SetColor(255,  cvet,  cvet), HEX); //синий.
          break;
        case 1: //зеленый
          strip[i] = SetColor( cvet * yark_LED , 255 * yark_LED  ,  cvet * yark_LED  );
          //        Serial.println (SetColor( cvet, 255,  cvet), HEX); //синий
          break;
        case 2: //синий
          strip[i] = SetColor(cvet * yark_LED  ,  cvet * yark_LED  , 255 * yark_LED  );
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 3: //бирюзлвый
          strip[i] = SetColor(cvet * yark_LED  ,  255 * yark_LED  , 255 * yark_LED  );
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 4: //фиолетовый
          strip[i] = SetColor( 255 * yark_LED ,   cvet * yark_LED , 255 * yark_LED  );
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 5: //желтый
          strip[i] = SetColor(255 * yark_LED  , 255 * yark_LED   ,  cvet * yark_LED  );
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 6: //оранжевый
          strip[i] = SetColor(255 * yark_LED  , (45 + 0.5 * i) * yark_LED  ,  (0 + 0.5 * i) * yark_LED );
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
      }
    }
    //
    for (int i = LED_COUNT / 2; i > numb_LED / 2; i--)
    {
      //  int cvet =30- i * 2;
      int cvet = (LED_COUNT / 2 - i) * 2.5;
      //   strip[i] = CHSV(inkr_2, 255, 255);  // HSV. Увеличивать HUE (цвет)
      //////ПРИКОЛЬНЫЙ ГРАДИЕНТ
      switch (dfg) {
        case 0: //красный
          strip[i] = SetColor(255 * yark_LED,  cvet * yark_LED,  cvet * yark_LED); // Красный цвет.
          //  Serial.println (SetColor(255,  cvet,  cvet), HEX); //синий.
          break;
        case 1: //зеленый
          strip[i] = SetColor( cvet * yark_LED, 255 * yark_LED,  cvet * yark_LED);
          //        Serial.println (SetColor( cvet, 255,  cvet), HEX); //синий
          break;
        case 2: //синий
          strip[i] = SetColor(cvet * yark_LED,  cvet * yark_LED, 255 * yark_LED);
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 3: //бирюзлвый
          strip[i] = SetColor(cvet * yark_LED,  255 * yark_LED, 255 * yark_LED);
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 4: //фиолетовый
          strip[i] = SetColor( 255 * yark_LED,   cvet * yark_LED, 255 * yark_LED);
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 5: //желтый
          strip[i] = SetColor(255 * yark_LED, 255 * yark_LED ,  cvet * yark_LED);
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
        case 6: //оранжевый
          strip[i] = SetColor(255 * yark_LED, (45 + 0.5 * i) * yark_LED ,  (0 + 0.5 * i) * yark_LED);
          //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
          break;
      }
    }

  }

  if (ircomand < 4)
  {
    if (ircomand == 1) {
      for (int i = 0; i < numb_LED; i++)
      {
        /////////////////ПРОСТО ПЛАВНЫЙ СМЕН ЦВЕТА
        strip[i] = CHSV(inkr_2, 255, 255 * yark_LED); // HSV. Увеличивать HUE (цвет)

      }
    }

    if ( ircomand == 2) {
      for (int i = numb_LED; i > -1; i--)
      {
        int cvet = numb_LED * 2 - (i * 2); //самый яркий элемент градиента "бегает"
        /////////////////////////////////////
        //////ПРИКОЛЬНЫЙ ГРАДИЕНТ
        switch (dfg) {
          case 0: //красный
            strip[i] = SetColor(255 * yark_LED,  cvet * yark_LED,  cvet * yark_LED); // Красный цвет


            //  Serial.println (SetColor(255,  cvet,  cvet), HEX); //синий.
            break;
          case 1: //зеленый
            strip[i] = SetColor( cvet * yark_LED, 255 * yark_LED,  cvet * yark_LED);
            //        Serial.println (SetColor( cvet, 255,  cvet), HEX); //синий
            break;
          case 2: //синий
            strip[i] = SetColor(cvet * yark_LED,  cvet * yark_LED, 255 * yark_LED);
            //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
            break;
          case 3: //бирюзлвый
            strip[i] = SetColor(cvet * yark_LED,  255 * yark_LED, 255 * yark_LED);
            //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
            break;
          case 4: //фиолетовый
            strip[i] = SetColor( 255 * yark_LED,   cvet * yark_LED, 255 * yark_LED);
            //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
            break;
          case 5: //желтый
            strip[i] = SetColor(255 * yark_LED, 255 * yark_LED ,  cvet * yark_LED);
            //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
            break;
          case 6: //оранжевый
            strip[i] = SetColor(255 * yark_LED, (45 + 0.5 * i) * yark_LED ,  (0 + 0.5 * i) * yark_LED);
            //        Serial.println (SetColor(cvet,  cvet, 255), HEX); //синий
            break;
        }
      }
    }


    /////Радуга
    if (ircomand == 3) {
      for (int i = 0; i < numb_LED; i++)
      {
        strip[i] = CHSV(counter + i * 4, 255, 255 * yark_LED);  // HSV. Увеличивать HUE (цвет)
      }
    }

  }
  if (ircomand == 5) { //просто переливающиеся цвета
    for (int i = 0; i < LED_COUNT; i++)
    {
      strip[i] = CHSV(counter, 255, 255 * yark_LED);  // HSV. Увеличивать HUE (цвет)
    }
  }
  counter++;

  if (ircomand == 6) { //свечение лампы одним цветом, который меняется по нажатию
    for (int i = 0; i < LED_COUNT; i++)
    {
      strip[i] = CHSV(counter_click, 255, 255 * yark_LED);  // HSV. Увеличивать HUE (цвет)
    }
  }
  if (ircomand == 7) { //радуга переливающаяся
    for (int i = 0; i < LED_COUNT; i++)
    {
      strip[i] = CHSV(counter + i * 4, 255, 255 * yark_LED);  // HSV. Увеличивать HUE (цвет)
    }
  }
  if (ircomand == 8) { //постоянный свет с блютуза
    for (int i = 0; i < LED_COUNT; i++)
    {
      strip[i] = CHSV(mode [3] , mode [4] , mode [5]* yark_LED*0.8 );   // HSV. Увеличивать HUE (цвет)
    }
  }
  // counter меняется от 0 до 255 (тип данных byte)
  // Передаем цвета ленте.
static int asd_count=0;


  FastLED.show();

  for (int i = 0; i < LED_COUNT; i++)
  {
    strip[i] = CRGB::Black; // Красный цвет.
  }
  delay(10);


}
///////////////////////////


unsigned long SetColor(unsigned long  Red, unsigned long  Green, unsigned long  blue)
{
  unsigned long RGB_16;


  blue = blue & 0xFF;
  Green = (Green << 8) & 0xFFFF;
  Red = (Red << 16) & 0xFFFFFF;

  RGB_16 = Red + Green + blue;
  return RGB_16;

}
int pars_bluet() {
  /*
    массив который отвечает за режим работы лампы
    0й элемент - Вкл/Выкл
    1й элемент - Режим работы
    2й элемент - Яркость (0...1)
    3й элемент - Цвет R
    4й элемент - Цвет G
    5й элемент - Цвет B
  */
  int buff[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  buff [0] =  BTSerial.read() - '0' ;
  buff [1] =  BTSerial.read() - '0' ;
  buff [2] =  BTSerial.read() - '0' ;
  buff [3] =  BTSerial.read() - '0' ;
  buff [4] =  BTSerial.read() - '0' ;
  buff [5] =  BTSerial.read() - '0' ;
  buff [6] =  BTSerial.read() - '0' ;
  buff [7] =  BTSerial.read() - '0' ;
  buff [8] =  BTSerial.read() - '0' ;
  buff [9] =  BTSerial.read() - '0' ;
  buff [10] =  BTSerial.read() - '0' ;

  if (buff[0] == 1) { //вкл/выкл лампы
    mode [0] = buff [1];
  }
  if (buff[0] == 2) { //изменение режима работы
    if (buff[1] == 1) {
      double_click = 0;
      switch (buff [2]) {
        case 1: //смена цвета при опускании звука
          flag_rezhim_1 = 0;
          ircomand = 1;
          break;
        case 2://Градиент
          flag_rezhim_1 = 0;
          ircomand = 2;
          break;
        case 3://Радуга
          flag_rezhim_1 = 0;
          ircomand = 3;
          break;
        case 4://ГРадиент из центра
          flag_rezhim_1 = 0;
          ircomand = 4;
          break;
        case 5://Переливание цвета
          flag_rezhim_1 = 0;
          ircomand = 5;
          break;
        case 6://Радуга
          flag_rezhim_1 = 0;
          ircomand = 7;
          break;
      }
    }
    if (buff[1] == 2) { //умный режим светомузыки
      double_click = 1;
      flag_rezhim_1 = 1;

    }
    if (buff[1] == 3) { //горит один цвет, который собираем по блютузу
      double_click = 0;
      flag_rezhim_1 = 0;
      ircomand = 8;
      mode [3] = buff [2] * 100 + buff [3] * 10 + buff [4] * 1;
      mode [4] = buff [5] * 100 + buff [6] * 10 + buff [7] * 1;
      mode [5] = buff [8] * 100 + buff [9] * 10 + buff [10] * 1;
    }


  }
      Serial.println (buff [0]);
    Serial.println (  mode [3]);
  if (buff[0] == 3) { //изменение яркости
    mode [0] = 2;
    mode [2] = buff [1] * 10 + buff [2] * 1;

  }

}
/////////////////////////////////////////
////РАНДОМНЫЕ БЛИКИ
//    switch (dfg) {
//      case 0:
//        strip[i] = SetColor(inkr, 255 - i * 9, 255 - i * 9); // Красный цвет.
//        break;
//      case 20:
//        strip[i] = SetColor(255 - i * 9, inkr, 255 - i * 9); // Красный цвет.
//        break;
//      case 40:
//        strip[i] = SetColor(255 - i * 9, 255 - i * 9, inkr); // Красный цвет.
//        break;
//  }
