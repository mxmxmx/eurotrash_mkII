#BOM for eurotrash wav player, mk 2


### SMD resistors (0603):
| value | #| part | note |
| --- | ---: | --- | --- | 
| 100R |4x| - | - | 
| 220R | 3x | BLM18BB221SN1D | (ferrit bead) |
| 470R | 2x | - | - |
| 1k  | 3x | - | - |	
| 10k  | 1x | - | - |
| 22k  | 2x | - | - |	
| 33k  | 8x | - | - |
| 100k | 6x | - | - |
| 200k | 4x | - | - |	

### SMD resistors (0805):
| value | #| part | note |
| --- | ---: | --- | --- | 
| 4k7  | 2x | - | i2c pull-up (may be 1206 even) |

### caps (0603):
| value | # | part | note |
| --- | ---: | --- | --- | 
| 15p  | 2x | - | NP0 / C0G, 25V+ | 
| 1n  |  4x | - | NP0 / C0G, 25V+ |
| 3n3  | 2x | - | NP0 / C0G, 25V+ (may be 0805) |
| 100n |   11x | - | MLCC (25V+) |
| 470n | 2x | - | MLCC (16V+) |
| 2u2  | 2x | - | MLCC (16V+) |
| 10uF | 3x | - | MLCC, or tantal (16V+) (may be 0805) |
  
### caps (electrolytic):
| value | # | part | note |
| --- | ---: | --- | --- | 
| 22uf | 2x | - | SMD (5.3 mm) (25V or better) | 
| 33uf | 2x | e.g. mouser # 647-UWX1E330MCL1MB) | SMD (5.3 mm) (25V or better) | 
	
###ICs/semis:
| what | # | package | part | note |
| --- | ---: | --- | --- | --- | 
| MMBT3904 | 2x | SOT-23 | mouser # 512-MMBT3904 | - |
| SM5817 | 2x | SOD-123FL | mouser # 833-SM5817PL-TP | reverse voltage protection |
| **TL072** | 1x | SOIC-8 | - | or use sth fancier (‡)| 
| MCP6004 | 1x | SOIC-14 | mouser # 579-MCP6004T-I/SL  | - |
| PCM5102a | 1x | TSSOP | mouser # 595-PCM5102APWR | - |
| SPI flash | 1x | SOIC-8 (wide) | **winbond W25Q128FV** | (‡‡)  | 

###voltage regulators/references:
| what | # | package | part | note |
| --- | ---: | --- | --- | --- | 
| ADP150 | 1x | TSOT | mouser # 584-ADP150AUJZ-3.3R7 | LDO, 3v3_A |
| LM1117-33 | 1x | SOT-223 | mouser # 511-LD1117S33 | LDO, 3v3_D |
| LM1117-50 | 1x | SOT-223 | mouser # 511-LD1117S50 | LDO, 5v |
| LM4040-10 | 1x | SOT-23 | - | prec. voltage reference, 10v| 

###misc:
| what | # | package | part | note |
| --- | ---: | --- | --- | --- |
| teensy3.1 or 3.2 | 1x | 2x14, 2.54mm | [oshpark](http://store.oshpark.com/products/teensy-3-1) | **don't forget to cut the usb/power trace** | 
| connectors, sockets for teensy 3.x | 2x (each)| 2 x 1x14, 2.54mm | - | use **precision / "machined" pins** |
| 2x5 connector | 1x | 2.54mm |  - | eurorack power connector |
| micro SD card socket | 1x | molex 502774-0891 | mouser # 538-502774-0891 | -  | 
| jacks | 8x | - | [PJ301M-12](https://www.thonk.co.uk/shop/3-5mm-jacks/) | thonkiconn (or kobiconn) | 
| encoder | 1x | 9mm | PEC11R | **w/ switch** (†) | 
| knob (to match) | 1x | - | - | ideally, with no indication line. Cliff K88, for instance, are fairly nice/suitable |
| tact switch | 2x |  - | multimecs 5E/5G | mouser #: 642-5GTH935 or 642-5ETH935 |
| + caps  | 2x | 15mm (height) | multimecs 1SS09 | mouser #: 642-1SS09-15.0, or -16.0 |
| 1.3" OLED | 1x | 4 pin (i2c) | 128x64, SH1106 | pinout: `VCC - GND - SCL - SDA` (††) |
| socket for OLED | 1x | 4 pin, 2.54mm | - |  **low profile** |



##notes: 
   
- (‡) something fancier can be used for the **audio** output ( = the dual op amp being used for audio output).
i've tried **OPA1662, AD8599, and LME49720** in the audio path - not sure whether i'd notice the difference, YMMV. but it's reassuring, sort of.

- (‡‡) the winbond ICs are a bit hard to find, i got mine from alibaba. (there's pin/software compatible alternative ICs at mouser and the usual places, which should work, see [here](https://github.com/PaulStoffregen/SerialFlash) for a list of supported devices. alternatively, SPI **SRAM** may be used, ie **23LC1024** (which is pin-compatible and supported by the pjrc audio API, but not part of the eurotrash firmware), cf http://www.pjrc.com/teensy/gui/?info=AudioEffectDelayExternal for details/

- (†) for instance: mouser # 652-PEC11R-4215F-S24 (15 mm, 'D' shaft); 652-PEC11R-4215K-S24 (15 mm shaft, knurled); 652-PEC11R-4220F-S24 (20 mm, 'D'), 652-PEC11R-4220K-S24 (20 mm, knurled), etc. personally/in this case, I prefer 'smooth' encoders, ie without detent: eg 652-PEC11R-4015K-S24)

- (††) while any 4 pin OLED w/ **SH1106** driver and suitable pin-out should work, here's two that's confirmed working: [here](http://www.ebay.de/itm/271817542866) and [here](http://www.ebay.de/itm/331132247980); they can also be had from amazon or aliexpress, cf [here](http://www.amazon.com/Serial-128X64-Display-Module-Arduino/dp/B00JM7SWI4/ref=sr_1_2?s=industrial&ie=UTF8&qid=1443270690&sr=1-2&keywords=1.3%22+Serial+128X64+OLED) or [here](http://de.aliexpress.com/item/white-1-3-I2C-IIC-Serial-128X64-12864-OLED-LCD-LED-Display-Module-for-Arduino-51/1669386183.html?isOrigTitle=true) or [here](http://www.alibaba.com/product-detail/1-3-I2C-IIC-Serial-128X64_1982440011.html?spm=a2700.7724838.30.1.5a3TpQ)