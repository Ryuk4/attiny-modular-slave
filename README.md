# attiny-modular-slave

The modular attiny slave let you implement a DHCP module on your i2c slaves. 
The i2c adress is initialised at 0x03 in eeprom memory and can be changed through i2c commands. The new i2c adress is stored and doesnt delete or change on power cuts.
