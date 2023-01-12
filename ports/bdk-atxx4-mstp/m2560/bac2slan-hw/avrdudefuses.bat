@echo off

rem Will need to set avrdude paths and port for current setup (simplest way to get these is via Arduino IDE upload with verbose output)
set port=COM6

set avrdude=C:\Users\markn\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17/bin/avrdude
set conf=C:\Users\markn\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17/etc/avrdude.conf
set baud=115200
set verbose=-v -V
rem set prog=arduino
rem set prog=wiring
set prog=stk500

rem Following args are provided by AS7 "Custom Programming Tool" args
rem set hexfile=%~1
set device=ATmega2560

set lf=0xff
set hf=0x98
set ef=0xff

echo Running AVRDude With Command:
echo "%avrdude%" -C"%conf%" -p%device% -c%prog% -b%baud% -D %verbose% -U lfuse:w:%lf%:m -U hfuse:w:%hf%:m -U efuse:w:%ef%:m -P%port% 2>&1

"%avrdude%" -C"%conf%" -p%device% -c%prog% -b%baud% -D %verbose% -U lfuse:w:%lf%:m -U hfuse:w:%hf%:m -U efuse:w:%ef%:m -P%port% 2>&1

