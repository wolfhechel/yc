echo on
cls

perl .\..\..\..\hex_to_bin\hex2rom.pl 1000000 1ffffff  32 .\Objects\YC3121_keil.hex  .\..\..\..\hex_to_bin\app_flash.rom
perl .\..\..\..\hex_to_bin\hex2rom.pl 1000000 1ffffff  32 .\..\..\..\Boot\prj\MDK\Objects\YC3121_keil.hex  .\..\..\..\hex_to_bin\boot_flash.rom
cd .\..\..\..\hex_to_bin
f_updata_rom_to_bin 128
call genapp.bat