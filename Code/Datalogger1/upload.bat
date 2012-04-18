rm Datalogger1.cpp.hex

copy C:\Users\Arthur\AppData\Local\Temp\build7126401935950655857.tmp\Datalogger1.cpp.hex .

avrdude -c arduino -b 19200 -p m328p -P com4 -F -U flash:w:Datalogger1.cpp.hex