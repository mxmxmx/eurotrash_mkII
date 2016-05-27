/*
*   eurotrash mk2
*   dual wav player. 'beta'
*
*   - micro SD card should be *class 10* !
*
*   - wav files should be 16 bit mono, 44.1kHz; **file names need to be 8.3** (SFN). filetype: .WAV 
*   - stereo is ok, too;  but there's no mixing down (easy to add, if desired); atm, left resp. right channel data is just discarded.
*   
*   - max files = 128 (can be changed - see the respective #define (MAXFILES)
*
*   - a/the list of valid files will be generated during initialization.
*
*   - files that go onto the flash / bank 2 should be 16 bit mono, 44.1kHz, with *no header*. filetype: .RAW 
*   **file names need to be 8.3** (SFN). push the top tact switch when powering up the module in order to move the files from SD onto the flash;
*
*
*  - things-to-do :

*    - erase flash from within the app (atm, this needs a separate sketch: EraseEverything.ino)
*    - make menu nice (phew)
*    - stay in menu item when switching channels, banks (?)
*    - eof (when using FL)
*/

#include <Audio.h>
#include <i2c_t3.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include <rotaryplus.h> 
#include <SerialFlash.h>
#include <u8g_i2c.h>

#if F_CPU != 120000000
  #error "Please compile eurotrash firmware with CPU speed 120MHz"
#endif

File root;

u8g_t u8g; // display 

/* ---------------------- Audio API ----------------------------- */

AudioPlaySdWav           wav1, wav2, wav3, wav4;
AudioPlaySdWav           *wav[4] = {&wav1, &wav2, &wav3, &wav4};
AudioPlaySerialflashRaw  raw1, raw2, raw3, raw4;
AudioPlaySerialflashRaw  *raw[4] = {&raw1, &raw2, &raw3, &raw4};
AudioEffectFade          fade1, fade2, fade3, fade4, fade1r, fade2r, fade3r, fade4r;
AudioEffectFade          *fade[8] = {&fade1, &fade2, &fade3, &fade4, &fade1r, &fade2r, &fade3r, &fade4r};
AudioMixer4              mixL, mixR;
AudioOutputI2S           pcm5102a;   

AudioConnection          ac_0(wav1, 0, fade1, 0);
AudioConnection          ac_1(wav2, 0, fade2, 0);
AudioConnection          ac_2(wav3, 0, fade3, 0);
AudioConnection          ac_3(wav4, 0, fade4, 0);

AudioConnection          ac_0r(raw1, 0, fade1r, 0);
AudioConnection          ac_1r(raw2, 0, fade2r, 0);
AudioConnection          ac_2r(raw3, 0, fade3r, 0);
AudioConnection          ac_3r(raw4, 0, fade4r, 0);

AudioConnection          ac_4(fade1,   0, mixL, 0);
AudioConnection          ac_5(fade2,   0, mixL, 1);
AudioConnection          ac_6(fade1r,  0, mixL, 2);
AudioConnection          ac_7(fade2r,  0, mixL, 3);
AudioConnection          ac_8(fade3,   0, mixR, 0);
AudioConnection          ac_9(fade4,   0, mixR, 1);
AudioConnection          ac_10(fade3r, 0, mixR, 2);
AudioConnection          ac_11(fade4r, 0, mixR, 3);

AudioConnection          ac_12(mixL, 0, pcm5102a, 0);
AudioConnection          ac_13(mixR, 0, pcm5102a, 1);

/* ----------------------- output channels ------------------------ */

#define CHANNELS 2
#define LEFT  0
#define RIGHT 1
#define INIT_FILE 0

typedef struct audioChannel {
  
    uint16_t    id;            // channel L/R
    uint16_t    file_wav;      // # file select
    uint16_t    state;         // channel state
    uint32_t    enc0;          // encoder file start pos [0-100%]
    uint32_t    encX;          // encoder end pos [0-100%]
    int16_t     cv0;           // start pos CV
    int16_t     cvX;           // end pos CV
    uint32_t    pos0;          // start pos (encoder + CV)
    uint32_t    posX;          // end pos (encoder + CV)
    uint32_t    ctrl_res;      // start pos resolution (in bytes)
    uint32_t    ctrl_res_eof;  // eof resolution  (in ms) 
    float       _gain;         // volume 
    uint32_t    eof;           // end of file (in ms)
    uint16_t    swap;          // ping-pong file (1/2; 3/4)
    uint16_t    bank;          // bank: SD / Flash

} audioChannel;

struct audioChannel *audioChannels[CHANNELS];

/* ----------------------- channel misc ----------------------------- */

const uint16_t FADE_IN  = 1;         // fade in  (adjust to your liking)
const uint16_t FADE_OUT = 50;        // fade out (ditto)
const uint16_t _FADE_F_CHANGE = 300; // fade out / file change 

uint32_t _FADE_TIMESTAMP_F_CHANGE = 0;

uint16_t FADE_LEFT, FADE_RIGHT, _EOF_L_OFF, _EOF_R_OFF;
uint32_t _LCLK_TIMESTAMP, _RCLK_TIMESTAMP, _EOF_L_TIMESTAMP, _EOF_R_TIMESTAMP; // trigger + E-o-F timestamps

uint16_t SPI_FLASH_STATUS = 0x0;

/* ------------------------- pins ----------------------------------- */

#define CLK_L 5 // L - trigger
#define CLK_R 8 // R - trigger

#define CV_L1 A2 // L - start
#define CV_L2 A3 // L - eof
#define CV_R1 A6 // R - start
#define CV_R2 A1 // R - eof

#define BUTTON_ENC 0 
#define ENC_1 1
#define ENC_2 2

#define BUTTON_TOP 21 
#define BUTTON_BOT 13

#define CS_SD 10   
#define CS_MEM 6 

#define SPI_FLASH 0x1 // 0x0 = no flash
 
/* ----------------------- encoder ----------------------------------- */

Rotary encoder = {
  ENC_1, ENC_2
}; 

/* ----------------------- timers + ISR stuff ------------------------ */

volatile uint16_t LCLK;   
volatile uint16_t RCLK;

uint32_t _TIMESTAMP_BUTTON;

void FASTRUN CLK_ISR_L() 
{ 
  LCLK = true; 
}

void FASTRUN CLK_ISR_R() 
{ 
  RCLK = true; 
}

IntervalTimer UI_timer, ADC_timer;
volatile uint8_t UI  = false;
volatile uint8_t _ADC = false;

#define UI_RATE  25000     // UI update rate (in us)
#define ADC_RATE 250       // ADC sampling rate (*4)
#define ADC_RES 10         // ADC resolution
extern uint16_t _MIDPOINT[]; // ~ 1.65V offset

void UItimerCallback()  
{ 
  UI = true;  
}
void ADCtimerCallback() 
{ 
  _ADC = true; 
}

/* ------------------------------------------------------ */

void setup() {
  
  //while (!Serial) {;} 
  delay(150); // wait for display ... 
  analogReference(EXTERNAL);
  analogReadRes(ADC_RES);
  analogReadAveraging(32);   
  // clk inputs and switches -- need the pullups 
  pinMode(CLK_L, INPUT_PULLUP);
  pinMode(CLK_R, INPUT_PULLUP);
  pinMode(BUTTON_TOP, INPUT_PULLUP); 
  pinMode(BUTTON_BOT, INPUT_PULLUP); 
  pinMode(BUTTON_ENC, INPUT_PULLUP); 
  
  pinMode(CS_MEM, OUTPUT);    
  digitalWriteFast(CS_MEM, HIGH);
  
  // audio API, SD:
  AudioMemory(35);
  SPI.setMOSI(7);
  SPI.setSCK(14);
  #if F_CPU == 120000000
   /* uncomment to overclock F_bus: */
   //SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(0) | SIM_CLKDIV1_OUTDIV4(4);
  #endif
   // init display:
  u8g_InitComFn(&u8g, &u8g_dev_sh1106_128x64_2x_i2c, u8g_com_hw_i2c_fn);
   // splash screen:
  _hello();
  delay(100);
  
  // init SD: 
  if (!(SD.begin(CS_SD))) {
    while (1) {
      _display("SD error");
      delay(1000);
    }
  }
  // zero volume while we generate the file list 
  mixL.gain(0, 0);
  mixL.gain(1, 0);
  mixL.gain(2, 0);
  mixL.gain(3, 0);
  mixR.gain(0, 0);
  mixR.gain(1, 0);
  mixR.gain(2, 0);
  mixR.gain(3, 0);
   
  // get files from spi flash ?
  if (SPI_FLASH) SPI_FLASH_STATUS = spi_flash_init();
  // update spi flash ? 
  if (!digitalRead(BUTTON_TOP) && SPI_FLASH_STATUS) SPI_FLASH_STATUS = copy_raw(); 
  // extract files from flash, if any 
  if (SPI_FLASH_STATUS) SPI_FLASH_STATUS = extract_flash();
  // and get files on SD: 
  extract_SD();
  
  // ADC + UI timers :
  ADC_timer.begin(ADCtimerCallback, ADC_RATE); 
  UI_timer.begin(UItimerCallback, UI_RATE);
  
  // allocate memory for L/R + init :
  audioChannels[LEFT]  = (audioChannel*)malloc(sizeof(audioChannel));
  audioChannels[RIGHT] = (audioChannel*)malloc(sizeof(audioChannel));
  init_channels(INIT_FILE);
  
  attachInterrupt(CLK_L, CLK_ISR_L, FALLING);
  attachInterrupt(CLK_R, CLK_ISR_R, FALLING);
  attachInterrupt(ENC_1, encoder_ISR, CHANGE);
  attachInterrupt(ENC_2, encoder_ISR, CHANGE);  

  //  calibrate mid point ?
  if (!digitalRead(BUTTON_ENC)) calibrate(); 
  check_signature() ? read_EEPROM() : _default_offset();
  
  // set volume 
  mixL.gain(0, audioChannels[LEFT]->_gain);
  mixL.gain(1, audioChannels[LEFT]->_gain);
  mixL.gain(2, audioChannels[LEFT]->_gain);
  mixL.gain(3, audioChannels[LEFT]->_gain);
  mixR.gain(0, audioChannels[RIGHT]->_gain);
  mixR.gain(1, audioChannels[RIGHT]->_gain);
  mixR.gain(2, audioChannels[RIGHT]->_gain);
  mixR.gain(3, audioChannels[RIGHT]->_gain);
  delay(1000);
  //print_raw_info();
  //print_wav_info();
  init_display();
}

/* main loop, wherein we mainly wait for the clock-flags */

void loop() 
{
  while(1) 
  {
     _loop();
  } 
}

/* ------------------------------------------------------ */
