
@echo off

REM set the current directory to where the bat file is
cd /d %~dp0

copy tinkerbell.natvis "%USERPROFILE%\Documents\Visual Studio 11\Visualizers\"
