/*  
*
* main loop  
*
*/ 

void _loop() 
{ 
     leftright();
     
     // fade out voice ?
     if (!FADE_LEFT) 
        eof_left();  

     leftright();
     
     // fade out voice ?
     if (!FADE_RIGHT) 
        eof_right(); 
     
     leftright();
   
     if (UI) _UI();                 
   
     leftright();
   
     if (_ADC) _adc();             
   
     leftright();
     
     // end-of-file / pause file ? 
     if (_EOF_L_OFF) 
        _PAUSE_EOF_L();   
   
     leftright();
     
     // end-of-file / pause file ?  
     if (_EOF_R_OFF) 
        _PAUSE_EOF_R();  
     
     leftright();
     
     // new file ?
     if (!audioChannels[LEFT]->state)  
        _open_new(audioChannels[LEFT]); 
     
     leftright();
     
     // new file ?
     if (!audioChannels[RIGHT]->state) 
        _open_new(audioChannels[RIGHT]);  
}
