#include <Arduino.h>
#include <u8glib.h>
#include <i2c_t3.h>

const uint16_t SLAVE_ADR = 0x3C;    // == 0x78
const uint16_t DATA_MODE = 0x40;    // i2c data command
const uint16_t COMMAND_MODE = 0x00; // i2c command command; 0x80 -- either seems to work

uint16_t CTRL_CMD = 0; 

uint8_t u8g_com_hw_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
    case U8G_COM_MSG_STOP:
      //STOP THE DEVICE
    break;

    case U8G_COM_MSG_INIT:

      Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_3000);
		
    break;

    case U8G_COM_MSG_ADDRESS:  
      //SWITCH FROM DATA TO COMMAND MODE (arg_val == 0) for command mode
    if (arg_val == 0)
      {
    	  CTRL_CMD = COMMAND_MODE;
      }
      else
      {
    	  CTRL_CMD = DATA_MODE;
      }
    break;

    case U8G_COM_MSG_RESET:
      // there's no reset pin .. ~ U8G_I2C_OPT_NO_ACK 
    break;

    case U8G_COM_MSG_WRITE_BYTE:
      //WRITE BYTE TO DEVICE
    
      Wire.beginTransmission(SLAVE_ADR);    // slave addr
      Wire.write(CTRL_CMD);                 // cmd
      Wire.write(arg_val);                  // data
      Wire.endTransmission();  
     
    break;

    case U8G_COM_MSG_WRITE_SEQ:
    case U8G_COM_MSG_WRITE_SEQ_P:
	 {
    //WRITE A SEQUENCE OF BYTES TO THE DEVICE
    register uint8_t *ptr = static_cast<uint8_t *>(arg_ptr);
    
    Wire.beginTransmission(SLAVE_ADR);
    Wire.write(CTRL_CMD); 
    while(arg_val > 0){
		    Wire.write(*ptr++);
		    arg_val--;
      }
    Wire.endTransmission();
  }
    break;

  }
  return 1;
}