**/* BOM for eurotrash wav player, mk 2 */**

// also see notes at bottom of the page // 


0603 resistors:

- 100R :		 4x 
- 220R :		 3x (or used ferrit bead, eg. BLM18BB221SN1D)
- 470R :		 2x
-  1k  :		 3x	
- 10k  :	 	 1x
- 22k  :         2x	
- 33k  :		 8x
- 100k :		 6x
- 200k : 		 4x	

0805 resistors:

- 4k7  : 		 2x (may be 1206 even)

0603 caps:

- 15p  :	     2x (NP0 / C0G, 25V+)
-  1n  :	     4x (NP0 / C0G, 25V+)
- 3n3  :		 2x (NP0 / C0G, 25V+, may be 0805)
- 100n :        11x (25V+)
- 470n :		 2x (16V+)
- 2u2  :         2x (16V+)
- 10uF :         3x (16V+, may be 0805)
  
caps (electrolytic), SMD (5.3 mm) (25V or better):

- 22uf :	  2x
- 33uf : 	  2x
	
ICs/semis:

- MMBT3904 (NPN trans., sot-23) : 2x (mouser # 512-MMBT3904)
- SM5817 diodes : 2x (mouser # 833-SM5817PL-TP)
- **TL072** (SOIC) : 1x (or fancier -- see note below (‡))
- MCP6004 (SOIC)   : 1x
- PCM5102a (TSSOP) : 1x (mouser # 595-PCM5102APWR)

voltage regulators/references:

- ADP150 (3v3 regulator, TSOT): 1x (mouser # 584-ADP150AUJZ-3.3R7)
- LM1117-33 (3v3 LDO reg., SOT-223): 1x (mouser # 511-LD1117S33)
- LM1117-50 (5v0 LDO reg., SOT-223): 1x (mouser # 511-LD1117S50)
- LM4040-10 (10v, sot-23): 1x 

misc:

- teensy3.1 or 3.2 : 1x (**don't forget to cut the usb/power trace**) (http://store.oshpark.com/products/teensy-3-1)
- 2x single row precision ("machined" / "round") pin header and sockets for teensy 3.1 (only the outer pins are used, so we need 14 pin, RM2.54)
- 2x5 RM2.54 pin header (eurorack power connector)
- molex micro SD card socket (# 502774-0891, mouser # 538-502774-0891)
- **winbond W25Q128FV** (SPI flash, SOIC-8) (optional) or **SPI SRAM (23LC1024)** (even more optional) (SOIC-8) (‡‡) 
- jacks: 8 thonkiconn (or kobiconn)
- rotary encoder w/ switch: 1x (†††)
- knobs (to match): 2x (ideally, with small(ish) diameter (12mm or so max) and no indication line. Cliff K88, for instance, are fairly nice/suitable)
- tact switches (multimecs 5E/5G): 2x (e.g. mouser #: 642-5GTH935 or 642-5ETH935) ; cheaper eg. at tme.eu
- + caps (multimecs 1SS09-12.0, 1SS09-15.0 or -16.0): 2x (mouser #: 642-1SS09-12.0, -15.0, or -16.0) (††)
- 1.3" 128x64 i2c oled, SH1106 (†)
- **4 pin header for oled** + 4 pin socket (low profile), RM2.54 (***)

notes: 

(‡) something fancier can be used for the audio output ( = the dual op amp being used for audio output).
i've tried **OPA1662, AD8599, and LME49720** in the audio path - not sure whether i'd notice the difference, YMMV. but it's reassuring, sort of.
     
(‡‡) the spi flash is optional; it's a very useful add-on though, as it allows for lower-latency one-shot triggering compared to streaming from SD card (~4ms vs ~8ms). the winbond ICs are a bit hard to find, i got mine from alibaba. (there's pin/software compatible alternative ICs at mouser and the usual places, which should work, see here for a list of supported devices: https://github.com/PaulStoffregen/SerialFlash (NB: the footprint is soic-8)); alternatively, the SPI SRAM thing may be used (which is supported by the pjrc audio API, but not part of the firmware), cf http://www.pjrc.com/teensy/gui/?info=AudioEffectDelayExternal for details

(†) pinout must be VCC - GND - SCL - SDA. while any OLED w/ SH1106 driver should work, here's two that's confirmed working: [here](http://www.ebay.de/itm/271817542866) and [here](http://www.ebay.de/itm/331132247980); they can also be had from amazon or aliexpress, cf [here](http://www.amazon.com/Serial-128X64-Display-Module-Arduino/dp/B00JM7SWI4/ref=sr_1_2?s=industrial&ie=UTF8&qid=1443270690&sr=1-2&keywords=1.3%22+Serial+128X64+OLED) or [here](http://de.aliexpress.com/item/white-1-3-I2C-IIC-Serial-128X64-12864-OLED-LCD-LED-Display-Module-for-Arduino-51/1669386183.html?isOrigTitle=true) or [here](http://www.alibaba.com/product-detail/1-3-I2C-IIC-Serial-128X64_1982440011.html?spm=a2700.7724838.30.1.5a3TpQ)

(††) depending on your needs: 12 = 12mm, 15 = 15mm, 16 = 16mm total height; if you prefer the switches to be flush with a/the regular aluminium panel, i'd suggest using 12mm : # 642-1SS09-12.0 ; if using 3mm acrylic, 15mm or 16mm is more suitable, etc.

(†††) for instance: mouser # 652-PEC11R-4215F-S24 (15 mm, 'D' shaft); 652-PEC11R-4215K-S24 (15 mm shaft, knurled); 652-PEC11R-4220F-S24 (20 mm, 'D'), 652-PEC11R-4220K-S24 (20 mm, knurled), etc. personally/in this case, I prefer 'smooth' encoders, ie without detent: eg 652-PEC11R-4015K-S24)

