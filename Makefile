UNAME := $(shell uname -s)
CXX = g++
CC = gcc
LD = g++
LDFLAGS =
LIBS = 
INCPATH = -I"tinkerbell/src" -I"Demo" -I"Demo/freeglut" -I"."

#release
CFLAGS = -DNDEBUG -fno-rtti -fno-exceptions -O2 $(INCPATH)
CXXFLAGS = -DNDEBUG -fno-rtti -fno-exceptions -O2 $(INCPATH)
#debug
#CFLAGS = -D_DEBUG -g $(INCPATH)
#CXXFLAGS = -D_DEBUG -g $(INCPATH)

ifeq ($(UNAME),Darwin)
 CFLAGS += -DMACOSX -Dnullptr=0
 CXXFLAGS += -DMACOSX -Dnullptr=0
 LIBS +=  -framework GLUT -framework OpenGL
else
 CFLAGS += -DLINUX -Dnullptr=0
 CXXFLAGS += -DLINUX -Dnullptr=0 --std=c++0x
 LIBS += -lglut
endif

TARGET = RunDemo
SRC = tinkerbell/src/tb_layout.cpp \
      tinkerbell/src/tb_scroll_container.cpp \
      tinkerbell/src/tb_skin.cpp \
      tinkerbell/src/tb_bitmap_fragment.cpp \
      tinkerbell/src/tb_select.cpp \
      tinkerbell/src/tb_editfield.cpp \
      tinkerbell/src/tb_system_linux.cpp \
      tinkerbell/src/tb_tab_container.cpp \
      tinkerbell/src/tb_widgets.cpp \
      tinkerbell/src/tb_widgets_common.cpp \
      tinkerbell/src/tb_widgets_reader.cpp \
      tinkerbell/src/tb_widget_value.cpp \
      tinkerbell/src/tb_widget_skin_condition_context.cpp \
      tinkerbell/src/tb_window.cpp \
      tinkerbell/src/tb_message_window.cpp \
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
      tinkerbell/src/tests/test_tb_tempbuffer.cpp \
      tinkerbell/src/tests/test_tb_test.cpp \
      stb_image/tb_image_loader_stb.cpp \
      tbanimation/Animation.cpp \
      tbanimation/tb_animation.cpp \
      Demo/Demo01.cpp \
      Demo/ResourceEditWindow.cpp \
      Demo/port_glut.cpp \
      Demo/tb_renderer_gl.cpp

OBJ=$(SRC:.cpp=.o)
OBJ += $(CSRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

clean:
	\rm -rf $(OBJ) $(TARGET)
	\rm -rf RunDemo

.SUFFIXES: .cpp .c
.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<


