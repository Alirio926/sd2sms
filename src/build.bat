cls

@echo off

tools\folder2c gfx gfx
move gfx.h inc\

echo *** SDCC compiling asm ***
sdasz80.exe -o reset_a.s
if %errorlevel% NEQ 0 goto :EOF

move reset_a.rel objs\

echo *** SDCC compiling C ***

REM comentario

REM sdcc --debug --std-sdcc11 -c -mz80 _heap.c -D HEAP_SIZE=2048
REM sdcc --std-sdcc11 -c -mz80 gfx.c -o objs\
REM sdcc -c -mz80 --peep-file peep-rules.txt  --codeseg BANK1 cpld_spi.c -o objs\

REM sdcc -c --std-sdcc11 -mz80 --codeseg BANK1 fat.c -o objs\
sdcc -c --std-sdcc11 -mz80 fat32.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 gfx.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 graphics.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 cpld_spi.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 util.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 sdcard_spi.c -o objs\
if %errorlevel% NEQ 0 goto :EOF

sdcc -c --std-sdcc11 -mz80 main.c -o objs\   
if %errorlevel% NEQ 0 goto :EOF


echo *** SDCC linking ***
REM sdcc -o objs\output.ihx -mz80 --no-std-crt0 --peep-file peep-rules.txt --data-loc 0xC000 -Wl-b_BANK1=0x14000 objs\crt0_sms.rel objs\main.rel objs\reset_a.rel objs\cpld_spi.rel objs\gfx.rel objs\util.rel objs\pff.rel objs\sdcard_spi.rel lib\SMSlib.lib
REM -->> works sdcc -o objs\output.ihx -mz80 --no-std-crt0 --data-loc 0xC000 -Wl-b_BANK1=0x14000 objs\crt0_sms.rel objs\main.rel objs\reset_a.rel objs\cpld_spi.rel objs\gfx.rel objs\util.rel objs\fat.rel objs\graphics.rel objs\sdcard_spi.rel lib\SMSlib.lib
sdcc -o objs\output.ihx -mz80 --no-std-crt0 --data-loc 0xC000 objs\crt0_sms.rel objs\main.rel objs\reset_a.rel objs\cpld_spi.rel objs\gfx.rel objs\util.rel objs\fat32.rel objs\graphics.rel objs\sdcard_spi.rel lib\SMSlib.lib
if %errorlevel% NEQ 0 goto :EOF

echo *** Converting to SMS ROM ***
REM Use this for 16k rom
tools\makesms objs\output.ihx out\output.sms
REM Use this for 32 or more kb of rom
REM tools\ihx2sms objs\output.ihx out\output.sms
if %errorlevel% NEQ 0 goto :EOF

REM del *.ihx > nul
REM del *.lk > nul
REM del *.lst > nul
REM del *.map > nul
REM del *.noi > nul
REM del *.sym > nul

D:\VHDL\Emulicious\Emulicious.exe out\output.sms

copy out\output.sms z:\menu.sms