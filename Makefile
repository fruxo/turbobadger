
glfw:
	cd BuildGLFW && $(MAKE)

sdl:
	cd BuildSDL && $(MAKE)

xc:
	cd Build-Xcode && xcrun xcodebuild -project "turbobadger.xcodeproj" -target turbobadger

ios:
	cd Build-iOS   && xcrun xcodebuild -project "turbobadger.xcodeproj" -target turbobadger

%:
	cd Build && $(MAKE) $@

