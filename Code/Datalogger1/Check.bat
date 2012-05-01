REM The Latest version of avrdude won't talk to the microcontroller,
REM but the old versions won't reset the microcontroller, and activate the bootloader.

avrdude -c arduino -b 57600 -p m328p -P com8
avrdude-old -vvv -c arduino -b 57600 -p m328p -P com8 -C C:\WinAVR-20100110\bin\avrdude-old.conf