SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;D:\ProgramData\Anaconda3;D:\ProgramData\Anaconda3\Library\mingw-w64\bin;D:\ProgramData\Anaconda3\Library\usr\bin;D:\ProgramData\Anaconda3\Library\bin;D:\ProgramData\Anaconda3\Scripts;C:\Perl\bin\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\TortoiseSVN\bin;D:\Program Files\msys64\mingw32\bin;D:\Program Files\msys64\mingw32;C:\Program Files\Git\cmd;C:\Program Files\dotnet\;C:\Program Files (x86)\Agilent\IO Libraries Suite\bin;C:\Program Files\Agilent\IO Libraries Suite\bin;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\agvisa;C:\Program Files\IVI Foundation\VISA\Win64\agvisa;C:\Program Files\IVI Foundation\VISA\Win64\Bin\;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\Bin\;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\ktvisa;C:\Program Files (x86)\IVI Foundation\IVI\bin;C:\Program Files\IVI Foundation\IVI\bin;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\Bin;C:\Users\Susunl\AppData\Local\Microsoft\WindowsApps;C:\Users\Susunl\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\Susunl\.dotnet\tools;C:\MinGW\bin
SET ARMCC5_ASMOPT=--diag_suppress=9931
SET ARMCC5_CCOPT=--diag_suppress=9931
SET ARMCC5_LINKOPT=--diag_suppress=9931
SET CPU_TYPE=ARMCM0
SET CPU_VENDOR=ARM
SET UV2_TARGET=YC3121
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\system.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\main.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\yc_uart.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\yc_gpio.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\yc_timer.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\objects\flash_start._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\objects\startup._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\freertoscommonhooks.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\croutine.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\event_groups.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\list.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\queue.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\tasks.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\timers.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\heap_5.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\port.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\Objects\YC3121_Keil.lnp"
"C:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\Objects\YC3121_Keil.axf" --i32combined --output ".\Objects\YC3121_Keil.hex"
fromelf.exe --text -a -c --output=YC3121_Keil_asm.txt ".\Objects\YC3121_Keil.axf"
