LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

PROJECT_ROOT := ../../

LOCAL_SRC_FILES := jni_glue.cpp \
                   App.cpp       \
                   ../../src/tb/tb_bitmap_fragment.cpp \
                   ../../src/tb/tb_blur.cpp \
                   ../../src/tb/tb_clipboard_dummy.cpp \
                   ../../src/tb/tb_color.cpp \
                   ../../src/tb/tb_dimension.cpp \
                   ../../src/tb/tb_editfield.cpp \
                   ../../src/tb/tb_font_renderer.cpp \
                   ../../src/tb/tb_font_renderer_tbbf.cpp \
                   ../../src/tb/tb_geometry.cpp \
                   ../../src/tb/tb_hash.cpp \
                   ../../src/tb/tb_hashtable.cpp \
                   ../../src/tb/tb_id.cpp \
                   ../../src/tb/tb_inline_select.cpp \
                   ../../src/tb/tb_language.cpp \
                   ../../src/tb/tb_layout.cpp \
                   ../../src/tb/tb_linklist.cpp \
                   ../../src/tb/tb_list.cpp \
                   ../../src/tb/tb_message_window.cpp \
                   ../../src/tb/tb_popup_window.cpp \
                   ../../src/tb/tb_str.cpp \
                   ../../src/tb/tb_menu_window.cpp \
                   ../../src/tb/tb_msg.cpp \
                   ../../src/tb/tb_object.cpp \
                   ../../src/tb/tb_renderer.cpp \
                   ../../src/tb/tb_scroller.cpp \
                   ../../src/tb/tb_scroll_container.cpp \
                   ../../src/tb/tb_select.cpp \
                   ../../src/tb/tb_select_item.cpp \
                   ../../src/tb/tb_shape_rasterizer.cpp \
                   ../../src/tb/tb_skin.cpp \
                   ../../src/tb/tb_skin_util.cpp \
                   ../../src/tb/tb_style_edit.cpp \
                   ../../src/tb/tb_style_edit_content.cpp \
                   ../../src/tb/tb_system_android.cpp \
                   ../../src/tb/tb_tab_container.cpp \
                   ../../src/tb/tb_tempbuffer.cpp \
                   ../../src/tb/tb_toggle_container.cpp \
                   ../../src/tb/tb_value.cpp \
                   ../../src/tb/tb_widget_skin_condition_context.cpp \
                   ../../src/tb/tb_widget_value.cpp \
                   ../../src/tb/tb_widgets.cpp \
                   ../../src/tb/tb_widgets_common.cpp \
                   ../../src/tb/tb_widgets_listener.cpp \
                   ../../src/tb/tb_widgets_reader.cpp \
                   ../../src/tb/tb_window.cpp \
                   ../../src/tb/tb_debug.cpp \
                   ../../src/tb/tb_node_tree.cpp \
                   ../../src/tb/tb_node_ref_tree.cpp \
                   ../../src/tb/tb_image_loader_stb.cpp \
                   ../../src/tb/tb_core.cpp \
                   ../../src/tb/animation/tb_animation.cpp \
                   ../../src/tb/animation/tb_widget_animation.cpp \
                   ../../src/tb/image/tb_image_manager.cpp \
                   ../../src/tb/image/tb_image_widget.cpp \
                   ../../src/tb/parser/tb_parser.cpp \
                   ../../src/tb/utf8/utf8.cpp \
                   ../../src/tb/renderers/tb_renderer_batcher.cpp \
                   ../../src/tb/renderers/tb_renderer_gl.cpp \

LOCAL_MODULE     := libTurboBadger
LOCAL_CFLAGS     := -Werror -DANDROID -D__ANDROID__ -DTB_RENDERER_GLES_1 -DTB_ALWAYS_SHOW_EDIT_FOCUS
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../.. $(LOCAL_PATH)/../../src/tb $(LOCAL_PATH)/../../Demo/renderers
LOCAL_LDLIBS     := -llog -landroid -lGLESv1_CM
LOCAL_CPPFLAGS   := -fno-rtti -fno-exceptions -std=c++0x

#release
LOCAL_CFLAGS     += -DNDEBUG -DDEBUG=0
#debug
#LOCAL_CFLAGS     += -D_DEBUG -DDEBUG=1

include $(BUILD_SHARED_LIBRARY)
