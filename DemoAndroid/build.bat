
@echo off

call update_assets.bat

set INSTALL=adb install -r bin/TBDemo-debug.apk
set RUN=adb shell am start -n com.fiffigt.tb.demo/.TBActivity

ndk-build -j 4 && ant debug && %INSTALL% && %RUN%
