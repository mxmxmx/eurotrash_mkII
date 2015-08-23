/*
*
* SD utilities
* &
* SPI flash utilities. (basically, those are github.com/PaulStoffregen/SerialFlash/examples)
*
*/

#define MAX_LEN 0x8  // file name size

/*  ======================================== */

uint8_t spi_flash_init() 
{ 
  return SerialFlash.begin(CS_MEM);
}

/*  ======================================== */

void print_raw_info() {
  
  Serial.println(" ");
  int x = 0;
  while (x < RAW_FILECOUNT) {
    Serial.print(DISPLAYFILES[MAXFILES+x]);
    Serial.print(" <-- ");
    Serial.print(FILES[MAXFILES+x]);
    Serial.print(" / ");
    Serial.print(CTRL_RES[MAXFILES+x], DEC);
    Serial.print(" / ");
    Serial.println(CTRL_RES_EOF[MAXFILES+x], DEC);
    x++;
  }
  Serial.println("");
  Serial.print("total: # "); Serial.println(RAW_FILECOUNT);
}

void print_wav_info() {
  
  Serial.println(" ");
  int x = 0;
  while (x < FILECOUNT) {
    Serial.print(DISPLAYFILES[x]);
    Serial.print(" <-- ");
    Serial.print(FILES[x]);
    Serial.print(" / ");
    Serial.println(CTRL_RES_EOF[x], DEC);
    x++;
  }
  Serial.println("");
  Serial.print("total: # "); Serial.println(FILECOUNT);
}

/*  ======================================== */

uint16_t copy_raw(){ // copy files from SD : 
  
    uint16_t _status = 0x0;
    
    _status = sd_to_flash();
    // return to normal:
    _MENU_PAGE[LEFT]  = FILESELECT; 
    _MENU_PAGE[RIGHT] = FILESELECT; 
    delay(2000);
    _TIMESTAMP_BUTTON = millis(); 
    return _status; 
}

/*  ======================================== */

uint16_t sd_to_flash() {

  int _file_num = 0, _errors = 0;

  _MENU_PAGE[RIGHT] = FLASH;
  
  delay(500);
  
  if (SPI_FLASH_STATUS) { 
       _display("FLASH ... ok");
       delay(1000);
  }
  else { 
      _display("FLASH ... error");
      delay(1000);
      return 0x0;
  }
  
  _display("looking for files ...");
  File rootdir = SD.open("/");
  while (1) {
  
      File f = rootdir.openNextFile();
      if (!f) { 
          if (!_file_num) { 
             
                _display("no files ...");
                delay(2000);
          }
          else { 
                 _display("all ok...");
                delay(500);
          }
          break;
      }
      
      char *filename = f.name();
      // raw ?
      uint32_t len = name_len(filename); 
      if  (!strcmp(&filename[len-2], "~1.RAW"))  {f.close(); continue; }
      else if  (filename[0] == '_')              {f.close(); continue; }
      else if  (!strcmp(&filename[len], ".WAV")) {f.close(); continue; }
      else if (!strcmp(&filename[len], ".RAW")) 
      {
         unsigned long length = f.size();
        
         // check if this file is already on the Flash chip :
         if (SerialFlash.exists(filename)) {
                
                Serial.println("  already exists on the Flash chip");
                SerialFlashFile ff = SerialFlash.open(filename);
                if (ff && ff.size() == f.size()) {
                        Serial.println("  size is the same, comparing data...");
                        if (compareFiles(f, ff) == true) {
                            Serial.println("  files are identical...");
                            f.close();
                            ff.close();
                            _file_num++;
                            //update_display(RIGHT, _file_num, 0x0);
                            continue;  // advance to next file
                        } else {
                            Serial.println("  files are different");
                        }
                } else {
                    Serial.print("  size is different, ");
                    Serial.print(ff.size());
                    Serial.println(" bytes");
                }
            // delete the copy on the Flash chip, if different
          Serial.println("  delete file from Flash chip");
          SerialFlash.remove(filename);
        }
      
      // create the file on the Flash chip and copy data
      if (SerialFlash.create(filename, length)) {
            SerialFlashFile ff = SerialFlash.open(filename);
            
            _display("... flashing");
            delay(500);
            _display(filename);
            delay(500);
            if (ff) {
                  // copy data loop
                  unsigned long count = 0;
                  while (count < length) {
                    
                      char buf[256];
                      unsigned int n;
                      n = f.read(buf, 256);
                      ff.write(buf, n);
                      count = count + n;
                      Serial.print(".");
                  }
                  _file_num++;
                  ff.close();
                  Serial.println();
           } 
           else {
                  Serial.println("  error opening freshly created file!");
                  _errors++;
                  _display("file error .. ");
                  delay(1000);
           }
    } 
    else { 
          Serial.println("  unable to create file");
          _errors++;
          _display("file error .. ");
          delay(1000);
    }
    f.close();
    delay(50);
    }
  }
 rootdir.close();
 return _file_num;
}

/*  ======================================== */

bool compareFiles(File &file, SerialFlashFile &ffile) {
  
  file.seek(0);
  ffile.seek(0);
  unsigned long count = file.size();
  while (count > 0) {
    char buf1[128], buf2[128];
    unsigned long n = count;
    if (n > 128) n = 128;
    file.read(buf1, n);
    ffile.read(buf2, n);
    if (memcmp(buf1, buf2, n) != 0) return false; // differ
    count = count - n;
  }
  return true;  // all data identical
}

/*  ======================================== */

uint16_t extract_flash(void)
{
  SerialFlash.opendir();
  uint16_t num_files = 0;
  
  while (1) {
    
    char filename[64];
    char tmp[DISPLAY_LEN];
    unsigned long filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
        
        // copy filenames
        memcpy(FILES[MAXFILES + num_files], filename, NAME_LEN);
        // copy length / get eof / etc
        raw1.play(filename);
        CTRL_RES[MAXFILES + num_files] = filesize * CTRL_RESOLUTION_INV;
        filesize = raw1.lengthMillis();
        CTRL_RES_EOF[MAXFILES + num_files] = filesize * CTRL_RESOLUTION_INV;
        raw1.stop();
        // copy display name // repair....
        int i = 0; 
        while (i < DISPLAY_LEN) {  
           tmp[i] = filename[i];
           if (tmp[i] == '.') { tmp[i] = ' '; tmp[i+1] = ' '; tmp[i+2] = ' '; tmp[i+3] = ' '; break; }
           else if (tmp[i] >= 'A' && tmp[i]  <= 'Z' ) tmp[i] = tmp[i] + 'a' - 'A';
           i++;               
        } 
        memcpy(DISPLAYFILES[MAXFILES + num_files], tmp, sizeof(tmp));
        num_files++;
     } 
     else break; // no more files
   }
   RAW_FILECOUNT = num_files;
   return num_files;
 }

/* =============================================== */

void extract_SD() {  // get files from SD
  
  uint16_t len;
  uint32_t file_len, file_len_ms;
  char tmp[DISPLAY_LEN];
  File thisfile;
  root = SD.open("/");
  thisfile = root.openNextFile();
  
  while (thisfile && FILECOUNT < MAXFILES) {
    
              char* _name = thisfile.name();            
              // wav files ?  
              len = name_len(_name); // length minus '.WAV'

              if  (!strcmp(&_name[len-2], "~1.WAV")) delay(1); // skip crap
              else if  (_name[0] == '_') delay(1);             // skip crap
              else if (!strcmp(&_name[len], ".WAV")) {
                      
                      memcpy(FILES[FILECOUNT], _name, NAME_LEN);
                      //uint16_t _index = SD.returnFileIndex(_name);
                      //SD_FILE_INDEX[FILECOUNT] = _index;
                      
                      wav1.play(_name);
                      delay(5); // delay until audio update();
                      file_len = (float)wav1.lengthBytes() * 0.9f;
                      CTRL_RES[FILECOUNT]  = file_len * CTRL_RESOLUTION_INV;       // ctrl resolution pos0/bytes
                      
                      file_len_ms = wav1.lengthMillis();
                      CTRL_RES_EOF[FILECOUNT] = file_len_ms * CTRL_RESOLUTION_INV; // ctrl resolution posX/bytes
                      wav1.stop();
                      
                      // for the display, get rid of .wav extension
                      int i = 0; 
                      while (i < DISPLAY_LEN) {  
                          tmp[i] = _name[i];
                          if (tmp[i] == '.') { tmp[i] = ' '; tmp[i+1] = ' '; tmp[i+2] = ' '; tmp[i+3] = ' '; break; }
                          else if (tmp[i] >= 'A' && tmp[i]  <= 'Z' ) tmp[i] = tmp[i] + 'a' - 'A';
                          i++;
                          
                      } 
                      memcpy(DISPLAYFILES[FILECOUNT], tmp, sizeof(tmp));
                      FILECOUNT++;
               }
               thisfile.close();
               thisfile = root.openNextFile(); 
  }   
  root.rewindDirectory(); 
  root.close();
}
  
/* =============================================== */

uint16_t name_len(const char *_n) {
  
    uint16_t _l = 0x0;
    
    while(_l < 0x8) {
      
       if (_n[_l] == '.' || _n[_l] == '\0') break;
       _l++;
    }
    return _l;
}

