// Code for the ATtiny85
#include <EEPROM.h>
#include <TinyWireS.h>

byte byteRcvd;
void initADC()
{
  /* this function initialises the ADC 

        ADC Prescaler Notes:
  --------------------

     ADC Prescaler needs to be set so that the ADC input frequency is between 50 - 200kHz.
  
           For more information, see table 17.5 "ADC Prescaler Selections" in 
           chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
          (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

           Valid prescaler values for various clock speeds
  
       Clock   Available prescaler values
           ---------------------------------------
             1 MHz   8 (125kHz), 16 (62.5kHz)
             4 MHz   32 (125kHz), 64 (62.5kHz)
             8 MHz   64 (125kHz), 128 (62.5kHz)
            16 MHz   128 (125kHz)

           Below example set prescaler to 128 for mcu running at 8MHz
           (check the datasheet for the proper bit values to set the prescaler)
  */

  // 8-bit resolution
  // set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
  // then, only reading ADCH is sufficient for 8-bit results (256 values)

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            (1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 64, bit 2 
            (1 << ADPS1) |     // set prescaler to 64, bit 1 
            (0 << ADPS0);      // set prescaler to 64, bit 0  
}

#define PWM_OUTPUT_PIN            1                                   //hardware pin#6 on the Attiny85

#define DEFAULT_I2C_DEVICE_ID 0x04 // Default Address of the slave

//===========================================================================

namespace EEPROM_DATA {
  enum {
    device_id_address   =     0x10,
    default_device_id   =     DEFAULT_I2C_DEVICE_ID
  };
  bool device_id_valid(uint8_t id) { return (id > 0x04 && id < 0x7E); }
  bool store_device_id(uint8_t new_device_id) {
    if (!device_id_valid(new_device_id)) return false;                //cancel if out of bounds. Assume corrupt data.
    EEPROM.write(EEPROM_DATA::device_id_address, new_device_id);      //Set new i2c address into EEPROM
    return true;
  }
  uint8_t get_device_id() {
    uint8_t _id = EEPROM.read(EEPROM_DATA::device_id_address);
    if (!device_id_valid(_id)) {
      _id = EEPROM_DATA::default_device_id;
      store_device_id(_id); //update EEPROM with a valid/default device id.
    }
    return _id;
  }  
}

//===========================================================================

namespace COMMANDS {
  enum {
    assign_new_device_id    =     0xC1,
    set_pwm_output          =     0xC2
  };
  bool do_command(uint8_t cmd, uint16_t cmd_data) {
  
    //interpret command.
    switch (cmd) {
      case COMMANDS::assign_new_device_id:                              //set new i2c slave adress. ATTENTION: After command is performed, the chip will need a hard reset.
        uint8_t new_device_id;
        new_device_id = uint8_t(cmd_data & 0x007F);                     //ensure the address is 7-bit.
        return EEPROM_DATA::store_device_id(new_device_id);             //return true of saved, false if invalid address.
        USICR=0;
        TinyWireS.begin(new_device_id);
        break;
  
      case COMMANDS::set_pwm_output:                                    // set pwm level for pwm output pin. Valid values are 0 - 255.
        if (cmd_data > 0x00FF) break;                                   //ignore out of range data. Assume corrupt packet.
//        TX_DATA::buffer[TX_DATA::pwm_data] = cmd_data;
//        analogWrite(PWM_OUTPUT_PIN, TX_DATA::buffer[TX_DATA::pwm_data] & 0x00FF);
        break;
  
      case 0xC3:                                                        //??? - not yet used.
        break;
  
      case 0xC4:                                                        //??? - not yet used.
        break;
  
    }
  }
}

//===========================================================================
 
int var=0;
 
void setup()
{
    initADC();
    uint8_t _device_id = EEPROM_DATA::get_device_id();
    TinyWireS.begin(_device_id); // join i2c network
    TinyWireS.onReceive(receiveEvent); // not using this
    TinyWireS.onRequest(requestEvent);
 
    // Turn on LED when program starts
    pinMode(1, OUTPUT);
    digitalWrite(1, HIGH);
}
 
void loop()
{
    // This needs to be here
    TinyWireS_stop_check();
}
 
// Gets called when the ATtiny receives an i2c request
void requestEvent()
{
    ADCSRA |= (1 << ADSC);
    var = ADCH;
    TinyWireS.send(ADCH);
}

void receiveEvent(uint8_t byte_count)
{
    /*
     * Gets called when the ATtiny recieves an i2c message THAT CONTAINS DATA from another device.
     */
    /*if (!TinyWireS.available() || byte_count != 3 ) return;             //qualify/validate call. first byte is command, next 2 bytes is data for command. Total must equal 3 bytes.
  
    //retrieve the command and 16-bit data. Incomming data is ordered CMD,Data-MSB,Data-LSB.
    uint8_t cmd = TinyWireS.receive();
    uint16_t cmd_data = (TinyWireS.receive() << 8);
    cmd_data += TinyWireS.receive();
    
    uint8_t cmd_data = TinyWireS.receive();
    COMMANDS::do_command(cmd, cmd_data);
    */
  if (TinyWireS.available()){           // si on revoit quelque chose sur le bus I2C
    byteRcvd = TinyWireS.receive();     // on l'enregistre
    //Blink(LED_PIN);// on blink un coup pour montrer que l'on est content
    switch(byteRcvd){
      case 0xC1:{
        byteRcvd = TinyWireS.receive();
        USICR=0;
        TinyWireS.begin(byteRcvd);
        break;
      }
      case 0x01:{
        //digitalWrite(WATER_PIN, LOW);
        break;
      }
      case 0x02:{
        //digitalWrite(WATER_PIN, HIGH);
        break;
      }
      case 0x10:{
        //digitalWrite(ONOFF_READER, HIGH);
        //val = analogRead(READER_PIN);
        //TinyWireS.send(val);
        //digitalWrite(ONOFF_READER, LOW);
        break;
      }
      byteRcvd = 0;
      _delay_ms(10);
    }
  }
}
