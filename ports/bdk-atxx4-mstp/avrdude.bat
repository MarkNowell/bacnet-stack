@echo off

rem Will need to set avrdude paths and port for current setup (simplest way to get these is via Arduino IDE upload with verbose output)
set port=COM16

set avrdude=C:\Users\markn\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17/bin/avrdude
set conf=C:\Users\markn\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17/etc/avrdude.conf
set baud=115200
set verbose=-v -V
rem set prog=arduino
set prog=wiring

rem Following args are provided by AS7 "Custom Programming Tool" args
set hexfile=%~1
set device=%~2

set lf=0xff
set hf=0xc8
set ef=0xfd

echo Running AVRDude With Command:
echo "%avrdude%" -C"%conf%" -p%device% -c%prog% -b%baud% -D %verbose% -Uflash:w:"%hexfile%":i -U lfuse:w:%lf%:m -U hfuse:w:%hf%:m -U efuse:w:%ef%:m -P%port% 2>&1

"%avrdude%" -C"%conf%" -p%device% -c%prog% -b%baud% -D %verbose% -Uflash:w:"%hexfile%":i -U lfuse:w:%lf%:m -U hfuse:w:%hf%:m -U efuse:w:%ef%:m -P%port% 2>&1


