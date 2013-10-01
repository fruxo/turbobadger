LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

PROJECT_ROOT := ../../

LOCAL_SRC_FILES := jni_glue.cpp \
                   App.cpp       \
                   ../../stb_image/tb_image_loader_stb.cpp \
                   ../../Demo/renderers/tb_renderer_gl.cpp \
                   ../../tinkerbell/src/tb_addon.cpp \
                   ../../tinkerbell/src/tb_bitmap_fragment.cpp \
                   ../../tinkerbell/src/tb_clipboard_dummy.cpp \
                   ../../tinkerbell/src/tb_editfield.cpp \
                   ../../tinkerbell/src/tb_font_renderer.cpp \
                   ../../tinkerbell/src/tb_font_renderer_tbbf.cpp \
                   ../../tinkerbell/src/tb_geometry.cpp \
                   ../../tinkerbell/src/tb_hash.cpp \
                   ../../tinkerbell/src/tb_hashtable.cpp \
                   ../../tinkerbell/src/tb_inline_select.cpp \
                   ../../tinkerbell/src/tb_language.cpp \
                   ../../tinkerbell/src/tb_layout.cpp \
                   ../../tinkerbell/src/tb_linklist.cpp \
                   ../../tinkerbell/src/tb_list.cpp \
                   ../../tinkerbell/src/tb_message_window.cpp \
                   ../../tinkerbell/src/tb_popup_window.cpp \
                   ../../tinkerbell/src/tb_menu_window.cpp \
                   ../../tinkerbell/src/tb_msg.cpp \
                   ../../tinkerbell/src/tb_object.cpp \
                   ../../tinkerbell/src/tb_renderer.cpp \
                   ../../tinkerbell/src/tb_scroller.cpp \
                   ../../tinkerbell/src/tb_scroll_container.cpp \
                   ../../tinkerbell/src/tb_select.cpp \
                   ../../tinkerbell/src/tb_select_item.cpp \
                   ../../tinkerbell/src/tb_skin.cpp \
                   ../../tinkerbell/src/tb_skin_util.cpp \
                   ../../tinkerbell/src/tb_style_edit.cpp \
                   ../../tinkerbell/src/tb_style_edit_content.cpp \
                   ../../tinkerbell/src/tb_system_android.cpp \
                   ../../tinkerbell/src/tb_tab_container.cpp \
                   ../../tinkerbell/src/tb_tempbuffer.cpp \
                   ../../tinkerbell/src/tb_toggle_container.cpp \
                   ../../tinkerbell/src/tb_value.cpp \
                   ../../tinkerbell/src/tb_widget_skin_condition_context.cpp \
                   ../../tinkerbell/src/tb_widget_value.cpp \
                   ../../tinkerbell/src/tb_widgets.cpp \
                   ../../tinkerbell/src/tb_widgets_common.cpp \
                   ../../tinkerbell/src/tb_widgets_listener.cpp \
                   ../../tinkerbell/src/tb_widgets_reader.cpp \
                   ../../tinkerbell/src/tb_window.cpp \
                   ../../tinkerbell/src/tb_debug.cpp \
                   ../../tinkerbell/src/tb_node_tree.cpp \
                   ../../tinkerbell/src/tinkerbell.cpp \
                   ../../tinkerbell/src/addons/tbimage/tb_image_manager.cpp \
                   ../../tinkerbell/src/addons/tbimage/tb_image_widget.cpp \
                   ../../tinkerbell/src/parser/TBParser.cpp \
                   ../../tinkerbell/src/utf8/utf8.cpp \

LOCAL_MODULE     := libTinkerbell
LOCAL_CFLAGS     := -Werror -DANDROID -D__ANDROID__ -DUSE_GLES -DTB_ALWAYS_SHOW_EDIT_FOCUS
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../.. $(LOCAL_PATH)/../../tinkerbell/src $(LOCAL_PATH)/../../Demo/renderers
LOCAL_LDLIBS     := -llog -landroid -lGLESv1_CM
LOCAL_CPPFLAGS   := -fno-rtti -fno-exceptions -std=c++0x

#release
LOCAL_CFLAGS     += -DNDEBUG
#debug
#LOCAL_CFLAGS     += -D_DEBUG

include $(BUILD_SHARED_LIBRARY)
