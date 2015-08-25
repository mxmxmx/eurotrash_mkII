/*
*
*  UI : menu / buttons / encoders
*
*/

// menu global params
uint16_t _MENU_PAGE[CHANNELS] = {0,0}; // active menu item
const uint16_t DISPLAY_LEN = 9;        // 8 (8.3) + /0
const uint16_t NAME_LEN = 13;          // 8.3
uint16_t filedisplay[CHANNELS];        // file on display (#)
char _display_file[] = "            "; // file on display (string) 
uint8_t _REDRAW = 0x0;                 // redraw menu
uint8_t _toggle = 0x0;                 // aux var.

// channel global params
const uint16_t MAXFILES = 128;       // we don't allow more than 128 files (for no particular reason); 
uint16_t FILECOUNT;                  // # SD files
uint16_t RAW_FILECOUNT;              // # flash files
uint16_t _ACTIVE_CHANNEL = 0x0;      // active channel (L / R menu)
const uint16_t _WAIT = 100;          // channel wait state (ms)
float DEFAULT_GAIN = 0.75;           // adjust default volume [0.0 - 1.0]

// file name arrays 
char FILES[MAXFILES*CHANNELS][NAME_LEN];         // file names
char DISPLAYFILES[MAXFILES*CHANNELS][NAME_LEN];  // display names

// file ctrl arrays
uint32_t CTRL_RES[MAXFILES*CHANNELS];
uint32_t CTRL_RES_EOF[MAXFILES*CHANNELS];
const uint16_t CTRL_RESOLUTION = 100; // ctrl resolution (encoders), relative to file size; adjust to your liking (< 9999)
const float   CTRL_RESOLUTION_INV = 1.0f/(float)CTRL_RESOLUTION;

//uint32_t RAW_FILE_ADR[MAXFILES+0x1];           // file adr, SPI
//uint16_t SD_FILE_INDEX[MAXFILES];              // SD file, indices

// buttons, encoders
uint16_t DIR = 0x1;                   // encoder direction
uint16_t _EVENT = 0x0;                // buttons event
uint32_t _TIMESTAMP_BUTTON_EVENT = 0;
const uint16_t DEBOUNCE = 100;
const uint16_t LONGPRESSED = 1000; // in ms

// misc messages 
const char *_SAVE = "    save?";
const char *_OK   = "       OK";
const char *_FLASH_OK       = " FLASH OK";
const char *_FLASH_NOT_OK   = "    ERROR";
const char *_FILES_OK       = " FILES OK";
const char *_ALLGOOD        = "     A-OK";
const char *_DOT            = "         ";
const char *_SD_ERROR       = " SD ERROR";
const char *_ERASE          = "... ERASE";
const char *_FLASHING       = " FLASHING";

const char *_thebanks[0x4] = 
{   
      "L (SD) >",
      "R (SD) >",
      "L (FL) >",
      "R (FL) >"
}; // left, right, SD or flash?

enum { 
   _SD, 
   _FLASH
}; // which bank ?

enum { 
  _STOP,
  _PLAY,
  _PAUSE, 
  _RETRIG
}; // channel state  

enum { 
   FILESELECT,
   STARTPOS,
   ENDPOS,
   CALIBRATE,
   ERASE_FLASH,
   FLASH 
}; // menu items

enum _buttons {
   _NONE,
   _TOP,
   _BOTTOM,
   _ENC,
   _DEBOUNCE_L
}; // which button ? 

enum _button_states {
   READY,
   PRESSED,
   SHORT,
   HOLD,
   DONE 
}; // button events

enum { 
  FLASH_OK,
  FLASH_NOT_OK,
  FILES_OK, 
  ALLGOOD,
  SD_ERROR,
  ERASE,
  FLASHING
}; // SPI flash progress messages
  

/* ----------------------------------------------------------------------- */

void encoder_ISR() 
{
  encoder.process();
}

/* ----------------------------------------------------------------------- */

void _UI() 
{ 
    _toggle = ~_toggle & 1u;
    // process controls or redraw display, if need be:
    if(_toggle)  
    {
        uint16_t _event = _EVENT;
        int16_t _encoder = update_encoder();
        
        // buttons ?
        if (_event) process_buttons(_event); 
        else _EVENT = update_buttons();
        // encoder ?
        if (_encoder) process_encoder(_ACTIVE_CHANNEL, _encoder);
        // redraw menu ? 
    }
    else if (_REDRAW) _do_display();
    UI = false;
}

/* ----------------------------------------------------------------------- */

int16_t update_encoder() 
{  
  return encoder.change() ? encoder.pos() : 0x0;
}  

/* ----------------------------------------------------------------------- */

uint16_t update_buttons() {
    
        uint16_t _event = _NONE; 
        
        if (!digitalReadFast(BUTTON_TOP) && (millis() - _TIMESTAMP_BUTTON > DEBOUNCE))  { 
            _event = _TOP;
           _TIMESTAMP_BUTTON = millis(); // ?
        }
        else if (!digitalReadFast(BUTTON_BOT) && (millis() - _TIMESTAMP_BUTTON > DEBOUNCE))  {
            _event = _BOTTOM;
            _TIMESTAMP_BUTTON = millis(); 
        }
        else if (!digitalReadFast(BUTTON_ENC) && (millis() - _TIMESTAMP_BUTTON > DEBOUNCE))  {
            _event = _ENC;
            _TIMESTAMP_BUTTON = millis(); 
        }
        return _event;
}

/* ----------------------------------------------------------------------- */

void process_buttons(uint16_t _event) {
  
  uint16_t _channel = _ACTIVE_CHANNEL;
  
  switch (_event) {
    
    case _TOP: {
      
      uint16_t _b = digitalReadFast(BUTTON_TOP);
      
      if (_b) { // short press: 
            _go_to_item( _channel, STARTPOS); 
            _TIMESTAMP_BUTTON = millis(); 
            _EVENT = _NONE; 
      } 
      else if (!_b && (millis() - _TIMESTAMP_BUTTON > LONGPRESSED)) { 
            switch_banks(LEFT);
            _TIMESTAMP_BUTTON = millis(); 
            _EVENT = _DEBOUNCE_L; 
      } 
      break;
    }
    
    case _BOTTOM: {
      
      uint16_t _b = digitalReadFast(BUTTON_BOT);
      
      if (_b) {  // short press: 
             _go_to_item( _channel, ENDPOS);  
             _TIMESTAMP_BUTTON = millis();
             _EVENT = _NONE;
      } 
      else if (!_b && (millis() - _TIMESTAMP_BUTTON > LONGPRESSED)) {
             switch_banks(RIGHT);  
             _TIMESTAMP_BUTTON = millis(); 
             _EVENT = _DEBOUNCE_L; 
      }
      break;
    }
    
    case _ENC: {
      
      uint16_t _b = digitalReadFast(BUTTON_ENC);
      
      if (_b) {  // short press: 
             if (_MENU_PAGE[ _channel] == FILESELECT) {
                  if (filedisplay[ _channel] != audioChannels[_channel]->file_wav) update_channel(audioChannels[_channel]);
             }
             else _go_to_item(_channel, FILESELECT);  
             _TIMESTAMP_BUTTON = millis(); 
             _EVENT = _NONE;      
             _REDRAW = 0x1;     
      } 
      else if (!_b && (millis() - _TIMESTAMP_BUTTON > LONGPRESSED)) { 
              // switch channel : 
              _ACTIVE_CHANNEL = ~_channel & 1u;
              switch_channels(_ACTIVE_CHANNEL); 
             _TIMESTAMP_BUTTON = millis(); 
             _EVENT = _DEBOUNCE_L; 
      }
      break;
    }
    
    case _DEBOUNCE_L: { 
      
      if (millis() - _TIMESTAMP_BUTTON > LONGPRESSED) _EVENT = _NONE; // resume
      break;
    }
    
    default:  
      break;
  }  
}

/* ----------------------------------------------------------------------- */

void switch_channels(uint16_t _channel) {
  
     _ACTIVE_CHANNEL = _channel; 
     _MENU_PAGE[_channel] = FILESELECT;
     uint16_t _file = audioChannels[_channel]->file_wav;
     uint16_t _bank = audioChannels[_channel]->bank;
     encoder.setPos(_file);
     
     _display_file[0x0] = '\xb7';
     memcpy(_display_file+0x2, DISPLAYFILES[_file + _bank*MAXFILES], DISPLAY_LEN);
     _REDRAW = 0x1;  
}

/* ----------------------------------------------------------------------- */

void switch_banks(uint16_t _channel) {
  
  if (SPI_FLASH_STATUS+SPI_FLASH) { 
            
       uint16_t _bank = audioChannels[_channel]->bank;
       uint16_t _voice_num = _bank*0x04 + CHANNELS*audioChannels[_channel]->id;
       
       // fade out channel
       fade[_voice_num]->fadeOut(FADE_OUT);
       _voice_num++;
       fade[_voice_num]->fadeOut(FADE_OUT);
       
        // update + reset channel 
       audioChannels[_channel]->bank = ~_bank & 1u;
       audioChannels[_channel]->file_wav = 0x0; 
       encoder.setPos(0x0);
       _MENU_PAGE[_channel] = FILESELECT;
       
       // pause channel         
       audioChannels[_channel]->state = _STOP; 
       _FADE_TIMESTAMP_F_CHANGE = millis();
                
       if (_channel == LEFT) {
          _EOF_L_OFF = false;
          FADE_LEFT = true;
       } 
       else {
          _EOF_R_OFF = false;
          FADE_RIGHT = true;
       }
       // menu 
       _display_file[0x0] = '\xb7';
        memcpy(_display_file+0x2, DISPLAYFILES[0x0 + _bank*MAXFILES], DISPLAY_LEN);
       _REDRAW = 0x1;
   }  
}

/* --------------------------------------------------------------- */

void _go_to_item(uint16_t _channel, uint16_t _item) {
  
  uint16_t _ch, _it;
 
  _ch = _channel;
  _it = _item;
  _MENU_PAGE[_ch] = _it;
   
  switch (_it) {
    
      case FILESELECT: { // resume from active file:
              
              encoder.setPos(audioChannels[_ACTIVE_CHANNEL]->file_wav); 
              break;
      }
      
      case STARTPOS: { // start pos:
 
              encoder.setPos(audioChannels[_ch]->enc0);
              // make sure we display the active file
              uint16_t _file = audioChannels[_ACTIVE_CHANNEL]->file_wav;
              uint16_t _bank = audioChannels[_ACTIVE_CHANNEL]->bank;
              _display_file[0x0] = '\xb7';
              memcpy(_display_file+0x2, DISPLAYFILES[_file + _bank*MAXFILES], DISPLAY_LEN);
              break; 
      } 
      
      case ENDPOS: { // end pos:
      
              encoder.setPos(audioChannels[_ch]->encX);
              // make sure we display the active file
              uint16_t _file = audioChannels[_ACTIVE_CHANNEL]->file_wav;
              uint16_t _bank = audioChannels[_ACTIVE_CHANNEL]->bank;
              _display_file[0x0] = '\xb7';
              memcpy(_display_file+0x2, DISPLAYFILES[_file + _bank*MAXFILES], DISPLAY_LEN);
              break; 
      }           
      default: break;  
  } 
  _REDRAW = 0x1; 
}

/* --------------------------------------------------------------- */

void update_channel(struct audioChannel* _ch) {
        
         uint16_t _id, _bank, _file;
       
         _id   = _ch->id;          // L or R ?
         _bank = _ch->bank;
         _file = filedisplay[_id]; // file #
        
        _ch->file_wav = _file;             // select file
         _display_file[0x0] = '\xb7';      // update display
        _ch->state = _STOP;                // pause channel
        
        if (!_id) {
             
             _EOF_L_OFF = false;
             FADE_LEFT = true;
        } 
        else {
          
             _EOF_R_OFF = false;
             FADE_RIGHT = true;
        }
        fade[_id*CHANNELS + _bank*0x4]->fadeOut(_FADE_F_CHANGE);
        fade[_id*CHANNELS + _bank*0x4 + 0x1]->fadeOut(_FADE_F_CHANGE);
        _FADE_TIMESTAMP_F_CHANGE = millis();
}  

/* --------------------------------------------------------------- */

void process_encoder(uint8_t _channel, int16_t _newval) {
 
  int16_t tmp = _newval;
  uint16_t _bank = audioChannels[_channel]->bank;
  
  _REDRAW = 0x1;
  
  switch (_MENU_PAGE[_channel]) {
    
    case FILESELECT: { // file
     
           uint16_t max_f = (_bank == _SD) ? (FILECOUNT-1) : (RAW_FILECOUNT-1);
        
           if (tmp < 0x0 && DIR)  { 
                 tmp = 0x0; 
                 encoder.setPos(tmp); 
                 DIR = 0x0;
             }  
           else if (tmp < 0x0)  {
                 tmp = max_f; 
                 encoder.setPos(tmp); 
                 DIR = 0x1;
             }
           else if (tmp > max_f) {
                tmp = 0x0; 
                encoder.setPos(tmp); 
             }
           
           memcpy(_display_file+0x2, DISPLAYFILES[tmp + _bank*MAXFILES], DISPLAY_LEN);
           
           // decorate the selected file: --> move this to _do_display
           if (tmp == audioChannels[_channel]->file_wav)  _display_file[0x0] = '\xb7';
           else _display_file[0x0] = ' ';
           filedisplay[_channel] = tmp;  
           break;
     }
      case STARTPOS: {  
        
            if (tmp < 0) { 
                 tmp = 0; encoder.setPos(0x0); 
              }
            else if (tmp > CTRL_RESOLUTION) { 
                 tmp = CTRL_RESOLUTION; 
                 encoder.setPos(tmp);
               }
            audioChannels[_channel]->enc0 = tmp;
            break;
          
     }
     case ENDPOS: {
           if (tmp < 1)  { 
                tmp = 1; 
                encoder.setPos(0x1); 
              }
           else if (tmp > CTRL_RESOLUTION) {
                tmp = CTRL_RESOLUTION; 
                encoder.setPos(tmp);
              }
           audioChannels[_channel]->encX = tmp;
           break;
     }  
  }
}

/* --------------------------------------------------------------- */
