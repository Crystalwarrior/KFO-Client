@echo off
if exist .\updatescript.ps1 (
    powershell.exe -executionpolicy bypass .\updatescript.ps1 -prerelease -autostart
) else (
    echo Error: updatescript.ps1 is missing! Cannot proceed with automatic updates.
    pause
)