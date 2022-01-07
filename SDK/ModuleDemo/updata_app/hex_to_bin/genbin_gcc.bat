echo on
cls

perl hex2rom.pl 1000000 1ffffff  32 ..\app\prj\GCC\output\out.hex  .\app_flash.rom
perl hex2rom.pl 1000000 1ffffff  32 ..\Boot\prj\GCC\output\out.hex  .\boot_flash.rom

f_updata_rom_to_bin 128