UNAME := $(shell uname -s)
CXX = g++
CC = gcc
LD = g++
LDFLAGS =
LIBS =
INCPATH = -I"tinkerbell/src" -I"Demo" -I"Demo/glfw/include" -I"."

ifeq ($(DEBUG),YES)
 CFLAGS = -D_DEBUG -g $(INCPATH)
 CXXFLAGS = -D_DEBUG -g $(INCPATH)
else
 CFLAGS = -DNDEBUG -fno-exceptions -O2 $(INCPATH)
 CXXFLAGS = -DNDEBUG -fno-rtti -fno-exceptions -O2 $(INCPATH)
endif

ifeq ($(UNAME),Darwin)
 CFLAGS += -DMACOSX -Dnullptr=0 -D_GLFW_COCOA
 CXXFLAGS += -DMACOSX -Dnullptr=0 -D_GLFW_COCOA
 LIBS +=  -framework OpenGL -framework AppKit -framework IOKit -lobjc
else
 CFLAGS += -DLINUX -Dnullptr=0 -D_GLFW_X11
 CXXFLAGS += -DLINUX -Dnullptr=0 --std=c++0x -D_GLFW_X11
 LIBS += -lGL -lX11 -lXxf86vm -lpthread -lXrandr -lXi
endif

CFLAGS += -D_GLFW_USE_OPENGL -D_GLFW_GLX
CXXFLAGS += -D_GLFW_USE_OPENGL -D_GLFW_GLX

TARGET = RunDemo
SRC = tinkerbell/src/tb_layout.cpp \
      tinkerbell/src/tb_scroller.cpp \
      tinkerbell/src/tb_scroll_container.cpp \
      tinkerbell/src/tb_skin.cpp \
      tinkerbell/src/tb_skin_util.cpp \
      tinkerbell/src/tb_bitmap_fragment.cpp \
      tinkerbell/src/tb_inline_select.cpp \
      tinkerbell/src/tb_select.cpp \
      tinkerbell/src/tb_select_item.cpp \
      tinkerbell/src/tb_editfield.cpp \
      tinkerbell/src/tb_system_linux.cpp \
      tinkerbell/src/tb_clipboard_glfw.cpp \
      tinkerbell/src/tb_tab_container.cpp \
      tinkerbell/src/tb_widgets.cpp \
      tinkerbell/src/tb_widgets_common.cpp \
      tinkerbell/src/tb_toggle_container.cpp \
      tinkerbell/src/tb_widgets_reader.cpp \
      tinkerbell/src/tb_widget_value.cpp \
      tinkerbell/src/tb_widget_skin_condition_context.cpp \
      tinkerbell/src/tb_window.cpp \
      tinkerbell/src/tb_debug.cpp \
      tinkerbell/src/tb_message_window.cpp \
      tinkerbell/src/tb_popup_window.cpp \
      tinkerbell/src/tb_menu_window.cpp \
      tinkerbell/src/tb_addon.cpp \
      tinkerbell/src/tinkerbell.cpp \
      tinkerbell/src/tb_tempbuffer.cpp \
      tinkerbell/src/tb_hash.cpp \
      tinkerbell/src/tb_hashtable.cpp \
      tinkerbell/src/tb_value.cpp \
      tinkerbell/src/tb_linklist.cpp \
      tinkerbell/src/tb_list.cpp \
      tinkerbell/src/tb_style_edit.cpp \
      tinkerbell/src/tb_style_edit_content.cpp \
      tinkerbell/src/tb_language.cpp \
      tinkerbell/src/tb_msg.cpp \
      tinkerbell/src/tb_object.cpp \
      tinkerbell/src/tb_widgets_listener.cpp \
      tinkerbell/src/tb_renderer.cpp \
      tinkerbell/src/tb_font_renderer.cpp \
      tinkerbell/src/tb_font_renderer_tbbf.cpp \
      tinkerbell/src/parser/TBNodeTree.cpp \
      tinkerbell/src/parser/TBParser.cpp \
      tinkerbell/src/utf8/utf8.cpp \
      tinkerbell/src/addons/tbimage/tb_image_manager.cpp \
      tinkerbell/src/addons/tbimage/tb_image_widget.cpp \
      tinkerbell/src/tests/tb_test.cpp \
      tinkerbell/src/tests/test_tb_style_edit.cpp \
      tinkerbell/src/tests/test_tb_space_allocator.cpp \
      tinkerbell/src/tests/test_tb_widget_value.cpp \
      tinkerbell/src/tests/test_tb_linklist.cpp \
      tinkerbell/src/tests/test_tb_object.cpp \
      tinkerbell/src/tests/test_tb_parser.cpp \
      tinkerbell/src/tests/test_tb_tempbuffer.cpp \
      tinkerbell/src/tests/test_tb_test.cpp \
      tinkerbell/src/tests/test_tb_color.cpp \
      tinkerbell/src/tests/test_tb_dimension.cpp \
      stb_image/tb_image_loader_stb.cpp \
      tbanimation/Animation.cpp \
      tbanimation/tb_animation.cpp \
      Demo/demo01/Demo01.cpp \
      Demo/demo01/ListWindow.cpp \
      Demo/demo01/ResourceEditWindow.cpp \
      Demo/platform/Application.cpp \
      Demo/platform/port_glfw.cpp \
      Demo/platform/glfw_extra_linux.cpp \
      Demo/renderers/tb_renderer_gl.cpp

CSRC = Demo/glfw/src/clipboard.c \
       Demo/glfw/src/context.c \
       Demo/glfw/src/gamma.c \
       Demo/glfw/src/init.c \
       Demo/glfw/src/input.c \
       Demo/glfw/src/joystick.c \
       Demo/glfw/src/monitor.c \
       Demo/glfw/src/time.c \
       Demo/glfw/src/window.c

ifeq ($(UNAME),Darwin)
MSRC = Demo/glfw/src/cocoa_clipboard.m \
       Demo/glfw/src/cocoa_gamma.m \
       Demo/glfw/src/cocoa_init.m \
       Demo/glfw/src/cocoa_joystick.m \
       Demo/glfw/src/cocoa_monitor.m \
       Demo/glfw/src/cocoa_time.m \
       Demo/glfw/src/cocoa_window.m \
       Demo/glfw/src/nsgl_context.m
else
CSRC += Demo/glfw/src/glx_context.c \
        Demo/glfw/src/x11_clipboard.c \
        Demo/glfw/src/x11_gamma.c \
        Demo/glfw/src/x11_init.c \
        Demo/glfw/src/x11_joystick.c \
        Demo/glfw/src/x11_monitor.c \
        Demo/glfw/src/x11_time.c \
        Demo/glfw/src/x11_unicode.c \
        Demo/glfw/src/x11_window.c
endif

.SUFFIXES: .cpp .c .m

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.m.o:
	$(CC) -c $(CFLAGS) -o $@ $<

OBJ = $(SRC:.cpp=.o)
OBJ += $(CSRC:.c=.o)
OBJ += $(MSRC:.m=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

clean:
	\rm -rf $(OBJ) $(TARGET)
	\rm -rf RunDemo
