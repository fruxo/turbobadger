
all: glfw sdl2 em

glfw:
	[ -d BuildGLFW ] || ./build.sh -glfw -gl
	cd BuildGLFW && $(MAKE)

sdl2:
	[ -d BuildSDL2 ] || ./build.sh -sdl2 -gl3
	cd BuildSDL2 && $(MAKE)

em:
	[ -d BuildEmsc ] || ./build.sh -em -gles2
	cd BuildEmsc && $(MAKE)

xc:
	cd Build-Xcode && xcrun xcodebuild -project "turbobadger.xcodeproj" -target turbobadger

ios:
	cd Build-iOS   && xcrun xcodebuild -project "turbobadger.xcodeproj" -target turbobadger

and:
	cd DemoAndroid && ninja

distclean:
	rm -rf BuildGLFW BuildSDL2 BuildEmsc Build-Xcode/turbobadger.xcodeproj Build-iOS/turbobadger.xcodeproj

#%:
#	cd Build && $(MAKE) $@
#
