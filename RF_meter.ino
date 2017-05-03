/*********************************************************************
<><> ExpertRC POWER METER <><>
ExpertRC.COM
Support forum FPV1.RU

PLS do not remove ExpertRC LINK AND LOGO from CODE and BOARD
THX
************************************************
*Based on->
*
  This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

  This example is for a 128x64 size display using I2C to communicate
  3 pins are required to interface (2 I2C and one reset)

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution
*********************************************************************/
byte m1 = 1;
byte m2 = 2;
byte m3 = 30; //30
byte m4 = 0;

float sensorValue = 0;
float sensorValue2 = 0;




#define BUZ_ON   PORTB |= _BV(5);
#define BUZ_OFF  PORTB &= ~_BV(5);

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <EEPROMex.h>
double aa;
double bb;



#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {

  pinMode(9, OUTPUT);

  //BUZ_ON;
  BUZ_OFF;
delay(400);
  Serial.begin(9600);
   //чето не всегда стартует
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  //display.display();
  delay(500);

  // Clear the buffer.
  display.clearDisplay();

  // draw a single pixel
  //display.drawPixel(10, 10, WHITE);
  // Show the display buffer on the hardware.
  // NOTE: You _must_ call display after making any drawing commands
  // to make them visible on the display hardware!




  //display.display();
  //delay(20000);
  //display.clearDisplay();

  // draw many lines
  //testdrawline();
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  // draw rectangles
  //testdrawrect();
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  // draw multiple rectangles
  //testfillrect();
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  // draw mulitple circles
  //testdrawcircle();
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  // draw a white circle, 10 pixel radius
  //display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  //testdrawroundrect();
  //delay(2000);
  //display.clearDisplay();

  //testfillroundrect();
  //delay(2000);
  //display.clearDisplay();

  //testdrawtriangle();
  //delay(2000);
  //display.clearDisplay();

  //testfilltriangle();
  //delay(2000);
  //display.clearDisplay();

  // draw the first ~12 characters in the font
  //testdrawchar();
  //display.display();
  //delay(2000);
  //display.clearDisplay();

  // draw scrolling text
  //testscrolltext();
  //delay(2000);
  //display.clearDisplay();

  // text display tests
  //display.setTextSize(1);
  //display.setTextColor(WHITE);
  //display.setCursor(0,0);
  //display.println("Hello, world!");
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  //display.println(3.141592);
  //display.setTextSize(2);
  //display.setTextColor(WHITE);
  //display.print("0x"); display.println(0xDEADBEEF, HEX);
  //display.display();
  //delay(2000);

  // miniature bitmap display
  //display.clearDisplay();
  //display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
  //display.display();

  // invert the display
  //display.invertDisplay(true);
  //delay(1000);
  //display.invertDisplay(false);
  //delay(1000);

  // draw a bitmap icon and 'animate' movement
  //testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
  display.clearDisplay(); display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.println("ExpertRC");
  //display.setCursor(10,0);
  display.println("PowerMeter");

  display.setTextSize(1);
  display.setCursor(30, 45);
  display.println("v.01 25");

display.setCursor(0, 57);
display.print("serial:a000");
display.println(EEPROM.readInt(0));
//display.println("serial:00001");
//EEPROM.writeInt(0,3);


  display.display();
  //display.invertDisplay(true);
  delay(3000);
  //display.invertDisplay(false);

  //button
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);

  //Ground better
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  pinMode(21, OUTPUT);
  digitalWrite(21, LOW);
  
  
  sensorValue = analogRead(A0);
  sensorValue = 1024;


//call menu
//float a;
//float b;

if (digitalRead(4) == LOW && digitalRead(6) == LOW){
display.setCursor(0, 0);
display.clearDisplay();display.println("Call Menu");display.display();

char junk = ' ';
while(1){

  Serial.println("MENU"); //Prompt User for input
  while (Serial.available()==0) {             //Wait for user input
  }
  junk = Serial.read() ;

//serial
//Serial.println(EEPROM.getAddress(sizeof(int)));//0
//menu
//Serial.println(EEPROM.getAddress(sizeof(byte)));//2
//Serial.println(EEPROM.getAddress(sizeof(byte)));//3
//Serial.println(EEPROM.getAddress(sizeof(byte)));//4
//Serial.println(EEPROM.getAddress(sizeof(byte)));//5
//call start
//Serial.println(EEPROM.getAddress(sizeof(double)));//6

Serial.println("------------------");
aa=EEPROM.readDouble(10);Serial.print("Reg 10 a27=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(15);Serial.print("Reg 15 b27=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(20);Serial.print("Reg 20 a144=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(25);Serial.print("Reg 25 b144=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(30);Serial.print("Reg 30 a433=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(35);Serial.print("Reg 35 b433=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(40);Serial.print("Reg 40 a868=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(45);Serial.print("Reg 45 b868=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(50);Serial.print("Reg 50 a915=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(55);Serial.print("Reg 55 b915=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(60);Serial.print("Reg 60 a1200=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(65);Serial.print("Reg 65 b1200=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(70);Serial.print("Reg 70 a2400=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(75);Serial.print("Reg 75 b2400=");printDouble2(bb,5);Serial.println("");

aa=EEPROM.readDouble(80);Serial.print("Reg 80 a5800=");printDouble2(aa,5);Serial.println("");
bb=EEPROM.readDouble(85);Serial.print("Reg 85 b5800=");printDouble2(bb,5);Serial.println("");




Serial.println("------------------");
  
Serial.println("Enter Reg");
while (Serial.available()==0)  {}
int tmp1=Serial.parseInt();
junk = Serial.read() ;

    
Serial.print("reg =");Serial.println(tmp1, DEC);

  
  Serial.println("Enter call=");      //Prompt User for input
  while (Serial.available()==0)  {}
  aa=Serial.parseFloat();
  EEPROM.writeDouble(tmp1,aa);   
  Serial.print("= "); Serial.println(aa, DEC);
      while (Serial.available() > 0)  // .parseFloat() can leave non-numeric characters
    { junk = Serial.read() ; }      // clear the keyboard buffer





}
}


  // Fill Mode
  if (digitalRead(5) == LOW) {
    display.clearDisplay(); display.display();

    while (1) {
      for (int i = 0; i <= 128; i++) {
        display.drawFastVLine (i, (analogRead(A0) / 9), 64, 1); //10
        display.display();
        //delay(10);
      }
      display.clearDisplay(); display.display();
    }
  }


  /////buz mode
  if (digitalRead(4) == LOW) {
    display.clearDisplay(); 
      display.setTextSize(2);
    display.setCursor(10, 10);
  display.println("Find Mode");
      display.display();
      
    word i2;
    word d=50;
    word dd;
    while (1) {
      
      word i = analogRead(A0);
      dd=150;

      if (i<i2){
        d=i2-i;
      if (d > 100){d=100;}
      //d=101-d;
      d=d/10;
      //if (i < 120) {
     //   i = 120;
     // }
      //i = i - 120;
      //i = i / 5;
      //i=i2=i;
      if (d==1){dd=80;}
      if (d==2){dd=40;}
      if (d==3){dd=20;}
      if (d==4){dd=10;}
      if (d==5){dd=6;}
      if (d==6){dd=5;}
      if (d==7){dd=4;}
      if (d==8){dd=3;}
      if (d==9){dd=2;}
      if (d==10){dd=1;}
      
      }
      display.setCursor(40, 30);
      display.setTextColor(WHITE, BLACK);
      display.print(1023-i);
      if (digitalRead(5) == LOW) {
        i2=i;
        display.display();
      }
      
      BUZ_ON;
      delay(10);
      BUZ_OFF;
      //if i
      delay(dd);
    }
  }

  //raw mode
  if (digitalRead(6) == LOW) {
    analogReference(INTERNAL);
    
    display.setTextSize(3);
    while (1) {
      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(0, 0);
      //display.print(analogRead(A0));
sensorValue=1023;
      for (int i = 0; i <= 800; i++) {
        //delay(150);
        sensorValue2 = analogRead(A0);
        if (sensorValue2 < sensorValue){sensorValue=sensorValue2;}
        }
       display.print(sensorValue);

      display.setCursor(0, 32);
      display.print(analogRead(A0));
    
    display.setTextSize(1);
    display.setCursor(0, 57);
    display.print("raw max/raw");
      delay(350);
      display.display();
    }
  }




}


void loop() {
  readee();
  readcal();
  display.clearDisplay(); display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  if (m1 == 0) {
    display.print("27mhz  ");
  }
  if (m1 == 1) {
    display.print("144mhz ");
  }
  if (m1 == 2) {
    display.print("433mhz ");
  }
  if (m1 == 3) {
    display.print("868mhz ");
  }
  if (m1 == 4) {
    display.print("915mhz ");
  }
  if (m1 == 5) {
    display.print("1200mhz");
  }
  if (m1 == 6) {
    display.print("2400mhz");
  }
  if (m1 == 7) {
    display.print("5800mhz");
  }

  display.setCursor(100, 0);
  if (m2 == 0) {
    display.print("CM");
  }
  if (m2 == 1) {
    display.print("AM");
  }
  if (m2 == 2) {
    display.print("MM");
  }


  display.setCursor(0, 18);
  display.print("Attn dBm");
  display.print(m3);


//refresh only end line
  while (1) {
    display.setTextSize(2);
    display.setCursor(0, 39);
    //display.print("dB=");
    if (m4 == 1) {
      display.print("mW=");
    }
    if (m4 == 0) {
      display.print("dBm");
    }

    display.setTextColor(WHITE, BLACK); // 'inverted' text
    display.setCursor(40, 39); display.println("       "); //display.display();
    display.setCursor(40, 39);
    //float sensorValue = analogRead(A0);// read the input on analog pin 0:

    analogReference(INTERNAL);
    sensorValue2 = analogRead(A0);// read the input on analog pin 0:

    //m2=1;

    // Continuous Mode
    if (m2 == 0) {
      sensorValue = sensorValue2;
    }
    // Avarage Mode
    if (m2 == 1) {
      sensorValue = sensorValue + ((sensorValue2 - sensorValue) / 10);
    }

    // Maximum Mode
    if (m2 == 2) {
      sensorValue=1023;
      
      for (int i = 0; i <= 800; i++) {
        //delay(150);
        sensorValue2 = analogRead(A0);
        
        if (sensorValue2 < sensorValue){sensorValue=sensorValue2;}
        }
      
      //if (sensorValue2 < sensorValue) {
      //  sensorValue = sensorValue2;
      //}
    }



    //float voltage1 = sensorValue * (3.327 / 1023.0);
    //voltage1=voltage1+10;
    //double dBm=(-1-(40*voltage1)+8); // calculate dBm
    double dBm;//=((85*voltage1)*-1); // calculate dBm 22

    //27 
    if (m1 == 0) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }
    
    //144 
    if (m1 == 1) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }
    
    //433 
    if (m1 == 2) {
      dBm = (sensorValue / aa);//9.0333
      dBm = (dBm * -1) + bb;// 15.9
    }

    //868 
    if (m1 == 3) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }

    //915 
    if (m1 == 4) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }

    //1200 
    if (m1 == 5) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }

    //2400 
    if (m1 == 6) {
      dBm = (sensorValue / aa);//9.0333
      dBm = (dBm * -1) + bb;// 15.9
    }

    //5800 
    if (m1 == 7) {
      dBm = (sensorValue / aa);
      dBm = (dBm * -1) + bb;
    }

    //dBm=dBm+15;
    dBm = dBm + m3;
    double Pm = pow( 10.0, (dBm) / 10.0); //pwr in mW
    double Pw = pow( 10.0, (dBm - 30) / 10.0); //pwr in W

    //display.println(dBm);
    //display.println(Pm);
    //  if (digitalRead(4)== LOW) {display.println(Pm);}
    //  if (digitalRead(4)== HIGH) {display.println(dBm);}

    if (m4 == 1) {
      display.print(Pm);
    }
    if (m4 == 0) {
      display.print(dBm);
    }
    //if (m4==0){analogReference(INTERNAL); display.print((analogRead(A0)*25.6)/1023);}
    //if (m4==0){analogReference(INTERNAL); display.print(analogRead(A0));delay(350);}


    if (digitalRead(5) == LOW) {
      menu();
      menu2();
      menu3();
      menu4();
      writeee();
      break;
    }

    display.setTextSize(1);
    display.setCursor(0, 57);
    
    if (m4 == 0) {
      display.print("Pm=");display.print(Pm);
    }
    if (m4 == 1) {
      display.print("dBm");display.print(dBm);
    }
    
    if (sensorValue2>600)
    {display.print(" to low");} //627}
    else if (sensorValue2 <10)
    {display.print(" to low");}
    else
    {display.print("          ");}

    
    
    display.display();
    delay(50);
  }



}





void menu(void) {
  sensorValue2 = 0; //.fix выход из меню
  sensorValue = 1023;
  display.clearDisplay(); display.display();
  display.setCursor(0, 45);
  display.print("Set Freq");
  delay (150);
  while (1) {
    delay (50);

    display.setCursor(0, 0);
    //display.print(m1);
    display.setTextColor(BLACK, WHITE);
 
  
  if (m1 == 0) {
    display.print("27mhz  ");
  }
  if (m1 == 1) {
    display.print("144mhz ");
  }
  if (m1 == 2) {
    display.print("433mhz ");
  }
  if (m1 == 3) {
    display.print("868mhz ");
  }
  if (m1 == 4) {
    display.print("915mhz ");
  }
  if (m1 == 5) {
    display.print("1200mhz");
  }
  if (m1 == 6) {
    display.print("2400mhz");
  }
  if (m1 == 7) {
    display.print("5800mhz");
  }



    //display.print(m1);

    display.display();
    if (digitalRead(4) == LOW) {
      m1++;
    }
    if (digitalRead(6) == LOW) {
      if (m1 > 0) {
        m1--;
      }
    }
    if (m1 > 7) {
      m1 = 7;
    }
    if (digitalRead(5) == LOW) {
      break;
    }

  }


}




void menu2(void) {
  display.clearDisplay(); display.display();
  display.setCursor(0, 45);
  display.print("Set Mode");
  delay (150);
  while (1) {
    delay (50);

    display.setCursor(100, 0);
    //display.print(m1);
    display.setTextColor(BLACK, WHITE);
    if (m2 == 0) {
      display.print("CM");
    }
    if (m2 == 1) {
      display.print("AM");
    }
    if (m2 == 2) {
      display.print("MM");
    }


    //display.print(m1);

    display.display();
    if (digitalRead(4) == LOW) {
      m2++;
    }
    if (digitalRead(6) == LOW) {
      if (m2 > 0) {
        m2--;
      }
    }
    if (m2 > 2) {
      m2 = 2;
    }
    if (digitalRead(5) == LOW) {
      break;
    }

  }


}





void menu3(void) {
  display.clearDisplay(); display.display();
  display.setCursor(0, 45);
  display.print("Set Attn");
  delay (150);

  while (1) {
    delay (50);
    display.setCursor(100, 20);
    display.setTextColor(BLACK, WHITE);
    display.print("  ");
    display.setCursor(100, 20); display.print(m3);


    display.display();
    if (digitalRead(4) == LOW) {
      m3++;
    }
    if (digitalRead(6) == LOW) {
      if (m3 > 0) {
        m3--;
      }
    }
    if (m3 > 100) {
      m3 = 100;
    }
    if (digitalRead(5) == LOW) {
      break;
    }

  }


}







//db mw
void menu4(void) {
  display.clearDisplay(); display.display();
  display.setCursor(0, 10);
  display.print("Set dB/mW");
  delay (150);
  while (1) {
    delay (50);

    display.setCursor(0, 45);
    //display.print(m1);
    display.setTextColor(BLACK, WHITE);
    if (m4 == 0) {
      display.print("dB");
    }
    if (m4 == 1) {
      display.print("mW");
    }

    display.display();
    if (digitalRead(4) == LOW) {
      m4++;
    }
    if (digitalRead(6) == LOW) {
      if (m4 > 0) {
        m4--;
      }
    }
    if (m4 > 1) {
      m4 = 1;
    }
    if (digitalRead(5) == LOW) {
      break;
    }

  }


}














void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f = 0; f < NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    delay(200);

    // then erase it + move it
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i = 0; i < display.height(); i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i = 0; i < display.height() / 2; i += 3) {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5) {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5) {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

//void testdrawrect(void) {
//  for (int16_t i = 0; i < display.height() / 2; i += 2) {
//    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, WHITE);
//    display.display();
//  }
//}

void testdrawline() {
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(0, 0, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(display.width() - 1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  delay(250);
}



void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * precision;
   else
       frac = (int(val)- val ) * precision;
   Serial.print(frac,DEC) ;
}

void readee(){
 m1 = EEPROM.readByte(2);
 m2 = EEPROM.readByte(3);
 m3 = EEPROM.readByte(4);
 m4 = EEPROM.readByte(5);
//byte m2 = 2;
//byte m3 = 30; //30
//byte m4 = 0;  
}
  
void writeee(){
  EEPROM.writeByte(2,m1);
  EEPROM.writeByte(3,m2);
  EEPROM.writeByte(4,m3);
  EEPROM.writeByte(5,m4);
//byte m2 = 2;
//byte m3 = 30; //30
//byte m4 = 0;  
}

void readcal(){
//27
if (m1==0){aa=EEPROM.readDouble(10);}
if (m1==0){bb=EEPROM.readDouble(15);}
//144
if (m1==1){aa=EEPROM.readDouble(20);}
if (m1==1){bb=EEPROM.readDouble(25);}
//433
if (m1==2){aa=EEPROM.readDouble(30);}
if (m1==2){bb=EEPROM.readDouble(35);}
//868
if (m1==3){aa=EEPROM.readDouble(40);}
if (m1==3){bb=EEPROM.readDouble(45);}
//915
if (m1==4){aa=EEPROM.readDouble(50);}
if (m1==4){bb=EEPROM.readDouble(55);}
//1200
if (m1==5){aa=EEPROM.readDouble(60);}
if (m1==5){bb=EEPROM.readDouble(65);}
//2400
if (m1==6){aa=EEPROM.readDouble(70);}
if (m1==6){bb=EEPROM.readDouble(75);}
//5800
if (m1==7){aa=EEPROM.readDouble(80);}
if (m1==7){bb=EEPROM.readDouble(85);}

  
}




void printDouble2( double val, byte precision){
 // prints val with number of decimal places determine by precision
 // precision is a number from 0 to 6 indicating the desired decimial places
 // example: lcdPrintDouble( 3.1415, 2); // prints 3.14 (two decimal places)

 if(val < 0.0){
   Serial.print('-');
   val = -val;
 }

 Serial.print (int(val));  //prints the int part
 if( precision > 0) {
   Serial.print("."); // print the decimal point
   unsigned long frac;
   unsigned long mult = 1;
   byte padding = precision -1;
   while(precision--)
 mult *=10;

   if(val >= 0)
frac = (val - int(val)) * mult;
   else
frac = (int(val)- val ) * mult;
   unsigned long frac1 = frac;
   while( frac1 /= 10 )
padding--;
   while(  padding--)
Serial.print("0");
   Serial.print(frac,DEC) ;
 }
}
