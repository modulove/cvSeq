#include <EEPROM.h>

// config for swapped pins from original design
#define LED_PIN 4
#define BTN_PIN 5
#define CLK_PIN 7
#define ROOT_CV 6
#define PTN_CV 3
#define GATEOUT 8 // EXPANSION D6 now on D8! 
// error was when the expansion header was added and the GATEOUT ommited. 
// You need to Fix with jumper wire needs to be applied to rev 0.1betafirstmodule

//rotary encoder library
#include <Encoder.h>

//display setting
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

bool display_ratch = 0;//1=display reflesh,0=not reflesh
bool DAC_ratch = 0; //1=DAC reflesh,0=not reflesh

//rotery encoder setting
Encoder myEnc(2, 3);//ロータリーエンコーダライブラリ用
float oldPosition  = -999;//ロータリーエンコーダライブラリ用
float newPosition = -999;

//push_sw
bool old_push = 0;
bool push = 0;
unsigned int pushcount = 0;

int mode = 1;//switch by rotery encoder
int select_mode = 1; //selected by push sw
//0 = select content
//1-8 = step ch1 - ch8
//9 root ,10 step_length,11 pattern,12 root_CV,13 SW,14 Reset,15 save

//Gate setting
bool gate[8] = { 1, 1, 1, 1, 1, 1, 1, 1};
bool gate_ratch = 0;
unsigned long trigTimer = 0;



//CV setting
const long cv_qnt[61] = {
 0,  68, 137,  205,  273,  341,  410,  478,  546,  614,  683,  751,
 819,  887,  956,  1024, 1092, 1161, 1229, 1297, 1365, 1434, 1502, 1570,
 1638, 1707, 1775, 1843, 1911, 1980, 2048, 2116, 2185, 2253, 2321, 2389,
 2458, 2526, 2594, 2662, 2731, 2799, 2867, 2935, 3004, 3072, 3140, 3209,
 3277, 3345, 3413, 3482, 3550, 3618, 3686, 3755, 3823, 3891, 3959, 4028, 4095
};
byte cv[8] = {12, 14, 16, 18, 20, 22, 24, 26};
int root = 0;
int root_fix = 0;//reflesh when sw count max
int root_CV = 0; //analogRead
int root_scale = 0; //0=Chromatic scale,1=Tonic and dominant,2=Tonic and dominant and sub dominant

const byte step_length[7] = {
 4, 6, 8, 12, 16, 24, 32
};
int step_select = 1;
int step_select_fix = 1;

const static byte ptn_4[16][4] PROGMEM = { //4step
 {1, 2, 3, 4},   {4, 3, 2, 1},  {1, 1, 2, 3},  {1, 2, 4, 3},  {1, 4, 3, 2},  {1, 1, 4, 4},
 {2, 2, 3, 3},  {2, 1, 3, 4},  {2, 3, 4, 1},  {2, 4, 4, 3},  {2, 1, 1, 3},  {3, 4, 1, 2},
 {3, 2, 1, 4},  {3, 3, 4, 1},  {1, 1, 1, 1},  {1, 2, 1, 2}
};

const static byte ptn_6[16][6] PROGMEM = { // 6steps
  {1, 2, 3, 4, 5, 6},    {6, 5, 4, 3, 2, 1},    {1, 1, 2, 2, 3, 3},    {3, 2, 1, 1, 2, 3},
  {4, 5, 6, 1, 2, 3},    {3, 2, 1, 6, 5, 4},    {1, 2, 1, 2, 1, 2},    {2, 1, 2, 1, 2, 1},
  {1, 3, 5, 2, 4, 6},    {6, 4, 2, 5, 3, 1},    {1, 2, 3, 3, 2, 1},    {3, 2, 1, 1, 2, 3},
  {6, 5, 4, 4, 5, 6},    {1, 1, 4, 4, 1, 1},    {1, 6, 5, 4, 3, 2},    {2, 3, 4, 1, 6, 5}
};

const static byte ptn_8[16][8] PROGMEM = {//8step
 {1, 2, 3, 4, 5, 6, 7, 8},  {8, 7, 6, 5, 4, 3, 2, 1},  {1, 2, 3, 4, 8, 7, 6, 5},  {1, 3, 5, 7, 2, 4, 6, 8},
 {1, 1, 3, 5, 2, 2, 3, 6},  {1, 5, 5, 2, 3, 4, 7, 6},  {1, 1, 1, 6, 2, 2, 7, 8},  {1, 8, 2, 7, 3, 6, 4, 5},
 {1, 2, 2, 1, 5, 7, 8, 6},  {2, 3, 4, 1, 6, 7, 8, 5},  {3, 4, 1, 2, 6, 7, 5, 8},  {8, 8, 7, 1, 1, 2, 3, 6},
 {5, 6, 4, 7, 3, 8, 2, 1},  {2, 2, 4, 6, 7, 8, 3, 1},  {5, 5, 4, 4, 3, 6, 6, 2},  {3, 3, 6, 6, 7, 8, 2, 1}
};
const static byte ptn_12[16][12] PROGMEM = { //12step
 {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4},  {1, 2, 3, 4, 5, 6, 7, 8, 5, 6, 7, 8},  {1, 2, 3, 4, 5, 6, 7, 8, 1, 3, 5, 7},  {8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5},
 {8, 7, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1},  {1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 8},  {1, 1, 3, 3, 2, 2, 4, 4, 5, 6, 7, 8},  {2, 1, 4, 3, 6, 5, 8, 7, 1, 3, 5, 7},
 {1, 3, 5, 7, 2, 4, 6, 8, 8, 8, 7, 4},  {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6},  {8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3},  {2, 5, 2, 5, 3, 6, 7, 8, 4, 5, 2, 2},
 {1, 1, 3, 5, 7, 8, 8, 8, 7, 5, 3, 2},  {4, 5, 5, 3, 6, 2, 2, 1, 7, 8, 6, 5},  {1, 5, 2, 6, 3, 7, 4, 8, 1, 4, 2, 5},  {1, 1, 6, 6, 2, 2, 7, 7, 8, 5, 4, 3}
};
const static byte ptn_16[16][16] PROGMEM = {//16step
 {1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1},  {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8},  {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8},  {8, 7, 6, 5, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8},
 {8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1},  {1, 1, 5, 5, 2, 2, 6, 6, 3, 3, 7, 7, 4, 4, 8, 8},  {1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 5, 6, 7, 8},  {4, 4, 5, 5, 3, 3, 6, 6, 2, 2, 7, 7, 1, 1, 8, 8},  {1, 2, 3, 4, 5, 4, 3, 2, 3, 4, 5, 6, 7, 6, 5, 4},
 {1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 8, 6, 5, 4, 3},  {1, 2, 3, 2, 4, 5, 6, 5, 7, 8, 6, 7, 2, 3, 4, 3},  {4, 4, 5, 5, 7, 8, 6, 3, 2, 2, 1, 1, 2, 5, 6, 7},  {8, 8, 7, 7, 5, 5, 6, 7, 4, 4, 3, 3, 2, 2, 1, 3},  {6, 6, 7, 8, 5, 5, 6, 7, 4, 4, 5, 6, 3, 3, 4, 5},
 {2, 3, 4, 1, 6, 7, 8, 5, 1, 2, 3, 4, 5, 6, 7, 8},  {1, 1, 2, 3, 5, 4, 3, 2, 5, 5, 6, 7, 8, 7, 6, 5}
};
const static byte ptn_24[16][24] PROGMEM = { // 24-step pattern
  {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8},    // Pattern 1
  {8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1},    // Pattern 2
  {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 2, 2, 1, 1, 4, 4, 3, 3},    // Pattern 3
  {4, 4, 3, 3, 6, 6, 5, 5, 8, 8, 7, 7, 2, 2, 1, 1, 4, 4, 3, 3, 6, 6, 5, 5},    // Pattern 4
  {6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 2, 2},    // Pattern 5
  {1, 3, 5, 2, 4, 6, 3, 5, 7, 4, 6, 1, 3, 5, 2, 4, 6, 3, 5, 7, 4, 6, 1, 3},    // Pattern 6
  {3, 1, 5, 6, 4, 2, 7, 5, 3, 1, 5, 6, 4, 2, 7, 5, 3, 1, 5, 6, 4, 2, 7, 5},    // Pattern 7
  {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8},    // Pattern 8
  {8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1},    // Pattern 9
  {1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1},    // Pattern 10
  {5, 6, 6, 7, 7, 8, 8, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 1, 1, 2, 2, 3},    // Pattern 11
  {3, 2, 1, 4, 3, 2, 1, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6},    // Pattern 12
  {6, 5, 8, 7, 2, 1, 4, 3, 6, 5, 8, 7, 2, 1, 4, 3, 6, 5, 8, 7, 2, 1, 4, 3},    // Pattern 13
  {4, 5, 6, 3, 2, 1, 7, 8, 4, 5, 6, 3, 2, 1, 7, 8, 4, 5, 6, 3, 2, 1, 7, 8},    // Pattern 14
  {8, 7, 1, 6, 2, 5, 3, 4, 7, 6, 1, 8, 5, 2, 4, 3, 6, 7, 1, 5, 8, 2, 4, 3},    // Pattern 15
  {2, 4, 6, 1, 3, 5, 7, 8, 4, 2, 6, 3, 1, 5, 8, 7, 2, 4, 6, 1, 3, 5, 7, 8}     // Pattern 16
};

const static byte ptn_32[16][32] PROGMEM = {//32step
 {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8},  {1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 5, 6, 7, 8, 2, 3, 1, 4, 2, 3, 1, 4, 6, 7, 5, 8, 6, 7, 5, 8},
 {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 2, 2, 4, 4, 1, 1, 3, 3, 6, 6, 8, 8, 5, 5, 7, 7},  {1, 3, 2, 4, 5, 7, 6, 8, 1, 3, 2, 4, 5, 7, 6, 8, 1, 3, 2, 4, 5, 7, 6, 8, 1, 1, 3, 3, 5, 5, 7, 7},
 {1, 1, 3, 3, 4, 4, 2, 2, 5, 5, 7, 7, 8, 8, 6, 6, 1, 3, 4, 5, 2, 5, 6, 7, 8, 6, 5, 4, 5, 3, 2, 1},  {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8},
 {8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1},  {8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1},
 {8, 8, 6, 6, 4, 4, 2, 2, 1, 1, 3, 3, 5, 5, 7, 7, 8, 8, 6, 6, 4, 4, 2, 2, 1, 1, 3, 3, 5, 5, 7, 7},  {1, 1, 3, 4, 2, 2, 4, 5, 3, 3, 5, 6, 4, 4, 6, 7, 5, 5, 7, 8, 6, 6, 7, 8, 8, 8, 7, 4, 4, 2, 3, 4},
 {5, 5, 6, 7, 4, 4, 5, 6, 3, 3, 4, 5, 6, 7, 8, 3, 1, 1, 2, 3, 2, 2, 3, 4, 5, 6, 7, 8, 5, 4, 3, 2},  {5, 5, 6, 6, 7, 8, 4, 3, 2, 2, 3, 4, 1, 1, 3, 4, 5, 5, 6, 7, 3, 2, 1, 5, 6, 7, 8, 6, 4, 3, 2, 3},
 {1, 2, 3, 2, 4, 5, 6, 5, 7, 8, 6, 7, 1, 2, 3, 2, 4, 5, 6, 5, 7, 8, 6, 7, 4, 5, 6, 5, 2, 3, 4, 3},  {1, 2, 3, 4, 5, 6, 5, 6, 8, 7, 6, 5, 8, 7, 6, 5, 4, 3, 4, 3, 2, 1, 3, 4, 2, 3, 4, 5, 2, 3, 4, 6},
 {1, 4, 3, 2, 5, 8, 7, 6, 1, 4, 3, 2, 5, 8, 7, 6, 2, 5, 4, 6, 2, 4, 3, 5, 3, 6, 5, 7, 3, 6, 5, 7},  {5, 5, 3, 2, 4, 4, 2, 1, 6, 6, 4, 3, 8, 7, 6, 8, 2, 2, 6, 7, 4, 4, 1, 2, 8, 7, 6, 5, 7, 6, 5, 8}
};

int ptn = 0;
int ptn_fix = 0;
int ptn_CV = 0;//ptn CV

bool clk_in = 0;
bool old_clk_in = 0;
byte play_step = 7;

//SWsetting
int sw_freq[8] = {  1, 2, 3, 4, 8, 16, 32, 64};
int sw_select = 0;
int sw = 1;

void setup() {
 
 load_data();
 
 display.begin(SSD1306_SWITCHCAPVCC);
 display.clearDisplay();

 pinMode(CLK_PIN, INPUT); // Originally the Clock Input --> Now on D7 (CLK_PIN)
 pinMode(BTN_PIN, INPUT_PULLUP);
 pinMode(GATEOUT, OUTPUT); // Was on D4 GATE-OUT but forgotten in the design when an afterthought (expansion header) was added. damn featurecreep

 
 //DAC I2C通信
 Wire.begin();

}

void loop() {
  
 old_clk_in = clk_in;
 old_push = push;

 push = digitalRead(BTN_PIN);
 //--------------------rotery encoder------------------------
 newPosition = myEnc.read();
 if ( (newPosition - 3) / 4  > oldPosition / 4) { //ロータリーエンコーダの分解能4で割る
   oldPosition = newPosition;
   display_ratch = 1;
   switch (select_mode) {
     case 0:
       mode = mode - 1;
       if (mode <= 0) {
         mode = 15;
       }
       break;

     case 1:
       cv[0] --;
       break;

     case 2:
       cv[1] --;
       break;

     case 3:
       cv[2] --;
       break;

     case 4:
       cv[3] --;
       break;

     case 5:
       cv[4] --;
       break;

     case 6:
       cv[5] --;
       break;

     case 7:
       cv[6] --;
       break;

     case 8:
       cv[7] --;
       break;

     case 9:
       root --;
       if (root <= 0) {
         root = 0;
       }
       break;

     case 10:
       step_select --;
       if (step_select <= 0) {
         step_select = 0;
       }
       break;

     case 11:
       ptn --;
       if (ptn <= 0) {
         ptn = 0;
       }
       if (ptn >= 15) {
         ptn = 15;
       }
       break;

     case 12:
       root_scale --;
       if (root_scale <= 0) {
         root_scale = 0;
       }
       break;

     case 13:
       sw_select --;
       if (sw_select <= 0) {
         sw_select = 0;
       }
       break;
   }
 }

 else if ( (newPosition + 3) / 4  < oldPosition / 4 ) { //ロータリーエンコーダの分解能4で割る
   oldPosition = newPosition;
   display_ratch = 1;
   switch (select_mode) {
     case 0:
       mode = mode + 1;
       if (mode >= 16) {
         mode = 1;
       }
       break;

     case 1:
       cv[0] ++;
       break;

     case 2:
       cv[1] ++;
       break;

     case 3:
       cv[2] ++;
       break;

     case 4:
       cv[3] ++;
       break;

     case 5:
       cv[4] ++;
       break;

     case 6:
       cv[5] ++;
       break;

     case 7:
       cv[6] ++;
       break;

     case 8:
       cv[7] ++;
       break;

     case 9:
       root ++;
       //if (root >= 61 - 30) {//61=cv_qnt amount , 30 = set cv range
       //  root = 61 - 30;
       if (root >= 61 - (61 - 12 + 1)) {
        root = 61 - (61 - 12 + 1);
       }
       break;

     case 10:
       step_select ++;
       if (step_select >= 6) {
         step_select = 6;
       }
       break;

     case 11:
       ptn ++;
       if (ptn >= 15) {
         ptn = 15;
       }
       break;

     case 12:
       root_scale ++;
       if (root_scale >= 9) {
         root_scale = 9;
       }
       break;

     case 13:
       sw_select ++;
       if (sw_select >= 7) {
         sw_select = 7;
       }
       break;

   }
 }
 cv[0] = constrain(cv[0], 0, 30); //countermeasure overflow
 cv[1] = constrain(cv[1], 0, 30);
 cv[2] = constrain(cv[2], 0, 30);
 cv[3] = constrain(cv[3], 0, 30);
 cv[4] = constrain(cv[4], 0, 30);
 cv[5] = constrain(cv[5], 0, 30);
 cv[6] = constrain(cv[6], 0, 30);
 cv[7] = constrain(cv[7], 0, 30);

 //---------------gate on/off switch--------------------
 if (push == 0) {
   pushcount ++;
 }
 if (mode >= 1 && mode <= 8) {
   if (pushcount >= 4000 && pushcount < 10000) {
     gate[mode - 1] = !gate[mode - 1];
     pushcount = 10000;//if long term push , do not change select_mode
     display_ratch = 1;
   }
 }

 //---------------mode select--------------------
 if (pushcount <= 4000 && old_push == 0 && push == 1 && select_mode == 0) {
   select_mode = mode;
   display_ratch = 1;
   if (select_mode == 14) { //reset
     select_mode = 0;
     play_step = 0;
   }
   else if (select_mode == 15) { //save
     select_mode = 0;
     save_data();
   }
 }
 else if (pushcount <= 4000 && old_push == 0 && push == 1 && select_mode != 0) {
   select_mode = 0;
   display_ratch = 1;
 }
 if (old_push == 0 && push == 1) {
   pushcount = 0;
   display_ratch = 1;
 }

 //------------------clk judge---------------------
 clk_in = digitalRead(CLK_PIN);
 if (clk_in == 1 && old_clk_in == 0) {
   play_step ++;
   if (play_step >= step_length[step_select_fix] ) {
     play_step = 0;
     sw ++;
     if (sw > sw_freq[sw_select]) {
       AD();
       step_select_fix = step_select;//setting reflesh when sw count max.
       root_fix = root;//setting reflesh when sw count max.
       ptn_fix = ptn + ptn_CV;//setting reflesh when sw count max.
       if (ptn_fix >= 15) {
         ptn_fix = 15;
       }
       sw = 1;
     }
   }
   gate_ratch = 1;
   DAC_ratch = 1;
   display_ratch = 1;
 }

 //--------------------CV out---------------------
 if (DAC_ratch == 1) {
   DAC_ratch = 1;
   switch (step_select_fix) {
     case 0:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_4[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;

     case 1:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_6[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;

     case 2:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_8[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;

     case 3:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_12[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;

     case 4:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_16[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;
       
     case 5:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_24[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;
       
     case 6:
       DAC(cv_qnt[cv[(pgm_read_byte(&(ptn_32[ptn_fix][play_step]))) - 1] + root_fix + root_CV]);
       break;
   };

 };

 //--------------------gate out---------------------
 switch (step_select_fix) {
   case 0:
     if (gate[(pgm_read_byte(&(ptn_4[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;

   case 1:
     if (gate[(pgm_read_byte(&(ptn_6[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;

   case 2:
     if (gate[(pgm_read_byte(&(ptn_8[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;

   case 3:
     if (gate[(pgm_read_byte(&(ptn_12[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;

   case 4:
     if (gate[(pgm_read_byte(&(ptn_16[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;
     
   case 5:
     if (gate[(pgm_read_byte(&(ptn_24[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;
     
   case 6:
     if (gate[(pgm_read_byte(&(ptn_32[ptn_fix][play_step]))) - 1] == 1 && gate_ratch == 1) {
       trigTimer = millis();
       gate_ratch = 0;
     }
     break;
 };


 if ((millis() - trigTimer <= 5)) {//torigger output 5msec
   digitalWrite(8, HIGH);
 }
 else if (millis() - trigTimer > 5) {
   digitalWrite(8, LOW);
 }

 //--------------------display reflesh---------------------
 if (display_ratch == 1) {
   display_ratch = 0;
   set_display();
 }
}


void set_display() {
 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(WHITE);

 for (int j = 0; j <= 7 ; j++) {
   if (gate[j] == 1) {//no gate no display
     display.drawRect(2 + j * 16, 30 - cv[j], 12, cv[j], WHITE); //x,y,width,hight,color
     switch (step_select_fix) {
       case 0:
         if (j  == (pgm_read_byte(&(ptn_4[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;

       case 1:
         if (j  == (pgm_read_byte(&(ptn_6[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;

       case 2:
         if (j  == (pgm_read_byte(&(ptn_8[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;

       case 3:
         if (j  == (pgm_read_byte(&(ptn_12[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;
       case 4:
         if (j  == (pgm_read_byte(&(ptn_16[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;
       case 5:
         if (j  == (pgm_read_byte(&(ptn_24[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
         }
         break;

       case 6:
         if (j  == (pgm_read_byte(&(ptn_32[ptn_fix][play_step]))) - 1) {
           display.fillRect(2 + j  * 16, 30 - cv[j], 12, cv[j], WHITE);
           break;
         };

     };
   };

   display.setCursor(5 + j * 16, 32);
   switch (cv[j] % 12) {
     case 0://C
       display.setTextColor(WHITE);
       display.print(1);
       break;

     case 1://C#
       display.setTextColor(BLACK, WHITE);
       display.print(1);
       break;

     case 2://D
       display.setTextColor(WHITE);
       display.print(2);
       break;

     case 3://D#
       display.setTextColor(BLACK, WHITE);
       display.print(2);
       break;

     case 4://E
       display.setTextColor(WHITE);
       display.print(3);
       break;

     case 5://F
       display.setTextColor(WHITE);
       display.print(4);
       break;

     case 6://F#
       display.setTextColor(BLACK, WHITE);
       display.print(4);
       break;

     case 7://G
       display.setTextColor(WHITE);
       display.print(5);
       break;

     case 8://G#
       display.setTextColor(BLACK, WHITE);
       display.print(5);
       break;

     case 9://A
       display.setTextColor(WHITE);
       display.print(6);
       break;

     case 10://A#
       display.setTextColor(BLACK, WHITE);
       display.print(6);
       break;

     case 11://B
       display.setTextColor(WHITE);
       display.print(7);
       break;
   }
 }

 if ( mode <= 8) {
   display.drawTriangle((mode - 1) * 16, 32, (mode - 1) * 16, 38, 3 + (mode - 1) * 16, 35, WHITE); //x0, y0, x1, y1, x2, y2, color
   display.fillTriangle((select_mode - 1) * 16, 32, (select_mode - 1) * 16, 38, 3 + (select_mode - 1) * 16, 35, WHITE); //x0, y0, x1, y1, x2, y2, color
 }

 display.setTextColor(WHITE);

 if (mode == 9) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(0, 44);
 display.print("ROT");
 display.setTextColor(WHITE);
 display.setCursor(18, 44);
 display.print(":");
 display.setCursor(24, 44);
 display.print(root);

 if (mode == 10) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(48, 44);
 display.print("ST");
 display.setTextColor(WHITE);
 display.setCursor(60, 44);
 display.print(":");
 display.setCursor(66, 44);
 display.print(step_length[step_select]);

 if (mode == 11) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(84, 44);
 display.print("PTN");
 display.setTextColor(WHITE);
 display.setCursor(102, 44);
 display.print(":");
 display.setCursor(108, 44);
 display.print(ptn + 1);

 if (mode == 12) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(0, 56);
 display.print("SCL");
 display.setTextColor(WHITE);
 display.setCursor(18, 56);
 display.print(":");
 display.setCursor(24, 56);
 if (root_scale == 0) {
   display.print("CRM");
 }
 else  if (root_scale == 1) {
   display.print("TD ");
 }
 else  if (root_scale == 2) {
   display.print("TSD");
 }
 else  if (root_scale == 3) {
  display.print("MAJ");
 }
 else  if (root_scale == 4) {
  display.print("MIN");
 }
 else  if (root_scale == 5) {
  display.print("PEN");
 }
 else  if (root_scale == 6) {
  display.print("BLU");
 }
 else  if (root_scale == 7) {
  display.print("DOR");
 }
 else  if (root_scale == 8) {
  display.print("MIX");
 }
 else  if (root_scale == 9) {
  display.print("PHR");
 }

 if (mode == 13) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(48, 56);
 display.print("SW");
 display.setTextColor(WHITE);
 display.setCursor(60, 56);
 display.print(":");
 display.setCursor(66, 56);
 display.print(sw);
 display.setCursor(78, 56);
 display.print("/");
 display.setCursor(84, 56);
 display.print(sw_freq[sw_select]);

 if (mode == 14) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(100, 56);
 display.print("RS");

 if (mode == 15) {
   display.setTextColor(BLACK, WHITE);
 }
 else {
   display.setTextColor(WHITE);
 }
 display.setCursor(116, 56);
 display.print("SV");

 //select mode under bar
 if (select_mode == 9) {
   display.drawLine(24, 51, 36, 51, WHITE);//start x,start y,end x,end y
 }
 if (select_mode == 10) {
   display.drawLine(66, 51, 78, 51, WHITE);//start x,start y,end x,end y
 }
 if (select_mode == 11) {
   display.drawLine(108, 51, 120, 51, WHITE);//start x,start y,end x,end y
 }
 if (select_mode == 12) {
   display.drawLine(24, 63, 42, 63, WHITE);//start x,start y,end x,end y
 }
 if (select_mode == 13) {
   display.drawLine(66, 63, 94, 63, WHITE);//start x,start y,end x,end y
 }

 //step bar
 display.drawLine(0, 0, (play_step + 1) * 128 / step_length[step_select_fix], 0, WHITE); //start x,start y,end x,end y

 //for development
 //    display.setCursor(0, 1);
 //  display.print(ptn_fix);
 display.display();
 
}

void DAC(long CV_OUT) {
 Wire.beginTransmission(0x60);
 Wire.write((CV_OUT >> 8) & 0x0F);
 Wire.write(CV_OUT);
 Wire.endTransmission();
}

void AD() {
 ptn_CV = analogRead(PTN_CV) / 64; //ptn cv

 switch (root_scale) {
   case 0:
     root_CV = map(analogRead(ROOT_CV), 0, 1023, 0, 61); //cromatic scale
     break;

   case 1:
     root_CV = analogRead(ROOT_CV);//tonic and dominant
     if (root_CV / 102 == 1 || root_CV / 102 == 3 || root_CV / 102 == 5 || root_CV / 102 == 7 || root_CV / 102 == 9) {//102 is divide 1V to 2 area
       root_CV = 7;//dominant
     }
     else {
       root_CV = 0;//tonic
     };
     break;

   case 2:
     root_CV = analogRead(ROOT_CV);//tonic and dominant and sub dominant
     if (root_CV / 68 == 1 || root_CV / 68 == 4 || root_CV / 68 == 7 || root_CV / 68 == 10 || root_CV / 68 == 13) {//68 is divide 1V(204LSB) to 3 area
       root_CV = 6;//sub dominant
     }
     else if (root_CV / 68 == 2 || root_CV / 68 == 5 || root_CV / 68 == 8 || root_CV / 68 == 11 || root_CV / 68 == 14) {//68 is divide 1V(204LSB) to 3 area
       root_CV = 7;//dominant
     }
     else {
       root_CV = 0;//tonic
     }
     break;
     
    case 3:
      root_CV = analogRead(ROOT_CV); // major scale
      root_CV = map(root_CV, 0, 1023, 0, 48); // map the input to a range of 0-48

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 2 || root_CV == 4 || root_CV == 5 || root_CV == 7 || root_CV == 9 || root_CV == 11) {
        root_CV = 7; // dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

    case 4:
      root_CV = analogRead(ROOT_CV); // minor scale
      root_CV = map(root_CV, 0, 1023, 0, 48); // map the input to a range of 0-48

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 2 || root_CV == 3 || root_CV == 5 || root_CV == 7 || root_CV == 8 || root_CV == 10) {
        root_CV = 6; // sub dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

        case 5:
      root_CV = analogRead(ROOT_CV); // pentatonic scale 
      root_CV = map(root_CV, 0, 1023, 0, 30); // map the input to a range of 0-30

      // Determine the scale based on the root CV value
      root_CV = root_CV % 10;
      if (root_CV == 0 || root_CV == 2 || root_CV == 4 || root_CV == 7 || root_CV == 9) {
        root_CV = 7; // dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

    case 6:
      root_CV = analogRead(ROOT_CV); // blues scale
      root_CV = map(root_CV, 0, 1023, 0, 36); // map the input to a range of 0-36

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 3 || root_CV == 5 || root_CV == 6 || root_CV == 7 || root_CV == 10) {
        root_CV = 7; // dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

    case 7:
      root_CV = analogRead(ROOT_CV); // Dorian mode
      root_CV = map(root_CV, 0, 1023, 0, 48); // map the input to a range of 0-48

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 2 || root_CV == 3 || root_CV == 5 || root_CV == 7 || root_CV == 9 || root_CV == 10) {
        root_CV = 7; // dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

    case 8:
      root_CV = analogRead(ROOT_CV); // Mixolydian mode
      root_CV = map(root_CV, 0, 1023, 0, 48); // map the input to a range of 0-48

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 2 || root_CV == 4 || root_CV == 5 || root_CV == 7 || root_CV == 9 || root_CV == 10) {
        root_CV = 7; // dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;

    case 9:
      root_CV = analogRead(ROOT_CV); // Phrygian mode
      root_CV = map(root_CV, 0, 1023, 0, 48); // map the input to a range of 0-48

      // Determine the scale based on the root CV value
      root_CV = root_CV % 12;
      if (root_CV == 0 || root_CV == 1 || root_CV == 3 || root_CV == 5 || root_CV == 7 || root_CV == 8 || root_CV == 10) {
        root_CV = 6; // sub dominant
      }
      else {
        root_CV = 0; // tonic
      }
      break;
  }
}

void save_data() {
 display.clearDisplay();
 delay(1000);
 EEPROM.update(1, cv[0]);
 EEPROM.update(2, cv[1]);
 EEPROM.update(3, cv[2]);
 EEPROM.update(4, cv[3]);
 EEPROM.update(5, cv[4]);
 EEPROM.update(6, cv[5]);
 EEPROM.update(7, cv[6]);
 EEPROM.update(8, cv[7]);
 EEPROM.update(9, gate[0]);
 EEPROM.update(10, gate[1]);
 EEPROM.update(11, gate[2]);
 EEPROM.update(12, gate[3]);
 EEPROM.update(13, gate[4]);
 EEPROM.update(14, gate[5]);
 EEPROM.update(15, gate[6]);
 EEPROM.update(16, gate[7]);
 EEPROM.update(17, root);
 EEPROM.update(18, step_select);
 EEPROM.update(19, ptn);
 EEPROM.update(20, root_scale);
 EEPROM.update(21, sw_select);
 set_display();
}

void load_data() {
 cv[0] = EEPROM.read(1);
 cv[1] = EEPROM.read(2);
 cv[2] = EEPROM.read(3);
 cv[3] = EEPROM.read(4);
 cv[4] = EEPROM.read(5);
 cv[5] = EEPROM.read(6);
 cv[6] = EEPROM.read(7);
 cv[7] = EEPROM.read(8);
 gate[0] = EEPROM.read(9);
 gate[1] = EEPROM.read(10);
 gate[2] = EEPROM.read(11);
 gate[3] = EEPROM.read(12);
 gate[4] = EEPROM.read(13);
 gate[5] = EEPROM.read(14);
 gate[6] = EEPROM.read(15);
 gate[7] = EEPROM.read(16);
 root = EEPROM.read(17);
 step_select = EEPROM.read(18);
 ptn = EEPROM.read(19);
 root_scale = EEPROM.read(20);
 sw_select = EEPROM.read(21);
}
