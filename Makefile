UNAME := $(shell uname -s)
CXX = g++
CC = gcc
LD = g++
LDFLAGS =
LIBS =
INCPATH = -I"src/tb" -I"Demo" -I"Demo/thirdparty/glfw/include" -I"."

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
SRC = src/tb/tb_layout.cpp \
      src/tb/tb_scroller.cpp \
      src/tb/tb_scroll_container.cpp \
      src/tb/tb_skin.cpp \
      src/tb/tb_skin_util.cpp \
      src/tb/tb_bitmap_fragment.cpp \
      src/tb/tb_inline_select.cpp \
      src/tb/tb_select.cpp \
      src/tb/tb_select_item.cpp \
      src/tb/tb_editfield.cpp \
      src/tb/tb_system_linux.cpp \
      src/tb/tb_clipboard_glfw.cpp \
      src/tb/tb_tab_container.cpp \
      src/tb/tb_widgets.cpp \
      src/tb/tb_widgets_common.cpp \
      src/tb/tb_toggle_container.cpp \
      src/tb/tb_widgets_reader.cpp \
      src/tb/tb_widget_value.cpp \
      src/tb/tb_widget_skin_condition_context.cpp \
      src/tb/tb_window.cpp \
      src/tb/tb_debug.cpp \
      src/tb/tb_message_window.cpp \
      src/tb/tb_popup_window.cpp \
      src/tb/tb_menu_window.cpp \
      src/tb/tb_addon.cpp \
      src/tb/tb_core.cpp \
      src/tb/tb_dimension.cpp \
      src/tb/tb_id.cpp \
      src/tb/tb_str.cpp \
      src/tb/tb_color.cpp \
      src/tb/tb_tempbuffer.cpp \
      src/tb/tb_hash.cpp \
      src/tb/tb_hashtable.cpp \
      src/tb/tb_value.cpp \
      src/tb/tb_linklist.cpp \
      src/tb/tb_list.cpp \
      src/tb/tb_style_edit.cpp \
      src/tb/tb_style_edit_content.cpp \
      src/tb/tb_language.cpp \
      src/tb/tb_msg.cpp \
      src/tb/tb_object.cpp \
      src/tb/tb_widgets_listener.cpp \
      src/tb/tb_renderer.cpp \
      src/tb/tb_font_renderer.cpp \
      src/tb/tb_font_renderer_tbbf.cpp \
      src/tb/tb_geometry.cpp \
      src/tb/tb_node_tree.cpp \
      src/tb/tb_node_ref_tree.cpp \
      src/tb/tb_image_loader_stb.cpp \
      src/tb/parser/tb_parser.cpp \
      src/tb/utf8/utf8.cpp \
      src/tb/animation/tb_animation.cpp \
      src/tb/animation/tb_widget_animation.cpp \
      src/tb/addons/tbimage/tb_image_manager.cpp \
      src/tb/addons/tbimage/tb_image_widget.cpp \
      src/tb/renderers/tb_renderer_batcher.cpp \
      src/tb/renderers/tb_renderer_gl.cpp \
      src/tb/tests/tb_test.cpp \
      src/tb/tests/test_tb_style_edit.cpp \
      src/tb/tests/test_tb_space_allocator.cpp \
      src/tb/tests/test_tb_widget_value.cpp \
      src/tb/tests/test_tb_linklist.cpp \
      src/tb/tests/test_tb_object.cpp \
      src/tb/tests/test_tb_parser.cpp \
      src/tb/tests/test_tb_node_ref_tree.cpp \
      src/tb/tests/test_tb_value.cpp \
      src/tb/tests/test_tb_tempbuffer.cpp \
      src/tb/tests/test_tb_test.cpp \
      src/tb/tests/test_tb_color.cpp \
      src/tb/tests/test_tb_dimension.cpp \
      src/tb/tests/test_tb_geometry.cpp \
      Demo/demo01/Demo01.cpp \
      Demo/demo01/ListWindow.cpp \
      Demo/demo01/ResourceEditWindow.cpp \
      Demo/platform/Application.cpp \
      Demo/platform/port_glfw.cpp \
      Demo/platform/glfw_extra_linux.cpp

CSRC = Demo/thirdparty/glfw/src/clipboard.c \
       Demo/thirdparty/glfw/src/context.c \
       Demo/thirdparty/glfw/src/gamma.c \
       Demo/thirdparty/glfw/src/init.c \
       Demo/thirdparty/glfw/src/input.c \
       Demo/thirdparty/glfw/src/joystick.c \
       Demo/thirdparty/glfw/src/monitor.c \
       Demo/thirdparty/glfw/src/time.c \
       Demo/thirdparty/glfw/src/window.c

ifeq ($(UNAME),Darwin)
MSRC = Demo/thirdparty/glfw/src/cocoa_clipboard.m \
       Demo/thirdparty/glfw/src/cocoa_gamma.m \
       Demo/thirdparty/glfw/src/cocoa_init.m \
       Demo/thirdparty/glfw/src/cocoa_joystick.m \
       Demo/thirdparty/glfw/src/cocoa_monitor.m \
       Demo/thirdparty/glfw/src/cocoa_time.m \
       Demo/thirdparty/glfw/src/cocoa_window.m \
       Demo/thirdparty/glfw/src/nsgl_context.m
else
CSRC += Demo/thirdparty/glfw/src/glx_context.c \
        Demo/thirdparty/glfw/src/x11_clipboard.c \
        Demo/thirdparty/glfw/src/x11_gamma.c \
        Demo/thirdparty/glfw/src/x11_init.c \
        Demo/thirdparty/glfw/src/x11_joystick.c \
        Demo/thirdparty/glfw/src/x11_monitor.c \
        Demo/thirdparty/glfw/src/x11_time.c \
        Demo/thirdparty/glfw/src/x11_unicode.c \
        Demo/thirdparty/glfw/src/x11_window.c
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
