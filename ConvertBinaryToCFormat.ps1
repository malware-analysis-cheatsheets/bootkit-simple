$CurrentPath = Convert-Path .;
$DriverPath = $CurrentPath + "\..\x64\Debug\driver.sys";
$HeaderPath = $CurrentPath + "\..\bootkit\BinDriver.h";
[byte[]] $BinDriver = Get-Content $DriverPath -Encoding Byte;

$WriteData = "#pragma once`r`n#include <efi.h>`r`n`r`nstatic UINT8 DriverBinary[] =`r`n{`r`n";

$IsIndent = $true;
$i = 0;
Foreach ($e in $BinDriver) {
    If ($IsIndent) {
        $WriteData += "    ";
        $IsIndent = $false;
    }
    $WriteData += "0x" + [System.String]::Format("{0:X2}", [System.Convert]::ToUInt32($e));

    If (($i % 0x10) -eq 0x0F) {
        $WriteData += ",`r`n";
        $IsIndent = $true;
    } Else {
        $WriteData += ", ";
    }

    $i++;
}

$WriteData += "};`r`n";

Set-Content $HeaderPath -Value $WriteData -Encoding utf8;
