TARGET=TurboBadgerDemo

All: glfw sdl2 lib osx ios and em em-glfw

glfw:
	[ -d BuildGLFW ] || ./build.sh -glfw -gl
	cd BuildGLFW && $(MAKE) package

sdl2:
	[ -d BuildSDL2 ] || ./build.sh -sdl2 -gl3
	cd BuildSDL2 && $(MAKE) package

em-sdl2:
	[ -d BuildEmsc ] || ./build.sh -gles2 -sdl2 -em
	cd BuildEmsc && $(MAKE)

em-glfw:
	[ -d BuildEmscGl ] || ./build.sh -gles2 -glfw -em -o BuildEmscGl
	cd BuildEmscGl && $(MAKE)

em: em-sdl2 em-glfw

Build-osx:
	cmake . -G Xcode -BBuild-osx -DTB_RENDERER=GL3 -DTB_BUILD_DEMO=SDL2 || rm -rf Build-osx

osx: Build-osx
	cd Build-osx && xcrun xcodebuild -project "TurboBadger.xcodeproj" -target ${TARGET}

Build-ios:
	cmake . -G Xcode -BBuild-ios -DCMAKE_TOOLCHAIN_FILE=cmake/iOS.cmake || rm -rf Build-ios

ios: Build-ios
	cd Build-iOS   && xcrun xcodebuild -project "TurboBadger.xcodeproj" -target ${TARGET}

lib:
	[ -d BuildLib ] || ./build.sh -o BuildLib -gl3
	cd BuildLib && $(MAKE) package

and:
	cd DemoAndroid2 && ./gradlew build

distclean:
	rm -rf Build*

#%:
#	cd Build && $(MAKE) $@
#
