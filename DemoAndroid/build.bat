
@echo off

call update_assets.bat

set INSTALL=adb install -r bin/TinkerbellDemo-debug.apk
set RUN=adb shell am start -n com.demo.tinkerbell/.TinkerbellActivity

ndk-build && ant debug && %INSTALL% && %RUN%
