
@echo off

call update_assets.bat
call ndk-build

call ant debug
REM call ant release

call adb install -r bin/TinkerbellDemo-debug.apk
call adb shell am start -n com.demo.tinkerbell/.TinkerbellActivity

echo "Success!"
goto :eof
:error
echo "Failed!"
