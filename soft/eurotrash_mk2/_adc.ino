/*
*
*  ADC / start + eof CV
*
*/

#define numADC 0x4          // # of CV channels
#define _RES_SCALE 0x3      // CV scaling: 0x3 => 128; default CTRL_RES is just 100, ADC_RES 10;

uint16_t _ADC_cnt;          // counter, we only read one ADC at a time 
uint16_t _MIDPOINT[numADC]; // offsets, ~ 1.65V
const uint8_t  _cv_map[numADC] = {A2, A6, A3, A1}; // == L - srt,  R - srt, L - eof, R - eof

enum _CV_ {
  L_SRT,
  R_SRT,
  L_EOF,
  R_EOF
};

const uint8_t sig[numADC] = {0x42, 0xD, 0x2A, 0x80}; // 4 byte signature (for EEPROM)

extern uint8_t _REDRAW;

/* --------------------------------------------------------------- */

void _adc() 
{
  uint8_t _adc_num = _ADC_cnt;                     // get counter
  _adc_num = (_adc_num++ >= 0x3) ? 0x0 : _adc_num; // reset counter
  
  // offset - CV; then left shift : ~ -/+ 50
  int16_t _cv = 0x1 + ((*(_MIDPOINT+_adc_num) - analogRead(*(_cv_map+_adc_num))) >> _RES_SCALE); 
  
  switch (_adc_num) {
   
       case L_SRT: { 
       
         audioChannels[LEFT]->cv0 = _cv;
         break; 
       } 
       case R_SRT: { 
       
         audioChannels[RIGHT]->cv0 = _cv;
         break; 
       } 
       case L_EOF: { 
       
         audioChannels[LEFT]->cvX = _cv;
         update_eof(LEFT);
         break; 
       } 
       case R_EOF: { 
       
         audioChannels[RIGHT]->cvX = _cv;
         update_eof(RIGHT);
         break; 
       } 
       default: break;   
  }

  _ADC_cnt = _adc_num;
  _ADC = false; 
  //_REDRAW = 0x1;
}
 
/* ---------------------------- offset calibration misc--------------------------------- */

// if not calibrated, default to theory :

void _default_offset() 
{ 
    _MIDPOINT[0] = _MIDPOINT[1] = _MIDPOINT[2] = _MIDPOINT[3] = pow(2,ADC_RES-1)-1; 
}

// check for signature :

uint16_t check_signature() 
{  
   if (EEPROM.read(0x0) == sig[0] && EEPROM.read(0x1) == sig[1] && EEPROM.read(0x2) == sig[2] && EEPROM.read(0x3) == sig[3]) return 0x1;
   else return 0x0;
}

// save offset values : 

void write_EEPROM() 
{   
    uint8_t byte0, byte1, adr;
    uint16_t _val;
    
    // write signature
    EEPROM.write(0x0, sig[0]);
    EEPROM.write(0x1, sig[1]);
    EEPROM.write(0x2, sig[2]);
    EEPROM.write(0x3, sig[3]);
   
    // save offset values  
    adr = 0x4;  
    
    for (int i = 0; i < numADC; i++) {
     
        _val = _MIDPOINT[i];  
        byte0 = (uint8_t)(_val >> 0x8);
        byte1 = (uint8_t)_val;
        
        // display some stuff
        _display("... saving ");
        delay(500);
        _display_num(_val);
        delay(500);
        
        // and write to EEPROM :
        EEPROM.write(adr, byte0);
        adr++;
        EEPROM.write(adr, byte1);
        adr++;
    }
    _display("done.");
    delay(500);
}

// read from eeprom; values start at 0x4

void read_EEPROM()
{  
    uint8_t byte0, byte1, adr = 0x4;
    
    for (int i = 0; i < numADC; i++) { 
      
        byte0 = EEPROM.read(adr);
        adr++;
        byte1 = EEPROM.read(adr);
        adr++;
     
        _MIDPOINT[i] = ((uint16_t)byte0 << 8) + byte1;
    }  
}

// display things while figuring out the 4 offset values; 

void calibrate() {
  
       float average; 
       uint16_t i, k, save;
      
       _display("calibrate ... ");
       delay(500);
     
       k = save = 0x0;
       
       while (k < numADC) {
        
          average = 0.0f;
          
          for (i = 0; i < 200; i++) {
               average +=  analogRead(_cv_map[k]);
               delay(3);
          }
          *(_MIDPOINT+k) = average / 200.0f;
          k++;
       }
      
      _display_calibration();
      
      delay(500);
      
      // save value?
      while(digitalRead(BUTTON_ENC)) {
        
           if (!digitalRead(BUTTON_TOP) && !save) { 
             
                 save = 0x1; 
                 write_EEPROM();
                 break;
            }   
      }

      _TIMESTAMP_BUTTON = millis(); 
      _REDRAW = 0x1;
} 

/* --------------------------------------------------------------- */
