copy C:\Users\Arthur\AppData\Local\Temp\build7329107087120684329.tmp\Datalogger1.cpp.hex .

avrdude -c arduino -b 19200 -p m328p -P com4 -F -U flash:w:Datalogger1.cpp.hex