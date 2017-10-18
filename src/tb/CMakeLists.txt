cmake_minimum_required(VERSION 2.8)

set (LOCAL_SRCS 
  tb_bitmap_fragment.cpp
  tb_clipboard_dummy.cpp
  tb_clipboard_glfw.cpp
  tb_clipboard_sdl2.cpp
  tb_clipboard_win.cpp
  tb_color.cpp
  tb_core.cpp
  tb_debug.cpp
  tb_dimension.cpp
  tb_editfield.cpp
  tb_file_posix.cpp
  tb_font_renderer.cpp
  tb_font_renderer_freetype.cpp
  tb_font_renderer_stb.cpp
  tb_font_renderer_tbbf.cpp
  tb_geometry.cpp
  tb_hash.cpp
  tb_hashtable.cpp
  tb_id.cpp
  tb_image_loader_stb.cpp
  tb_inline_select.cpp
  tb_language.cpp
  tb_layout.cpp
  tb_linklist.cpp
  tb_list.cpp
  tb_menu_window.cpp
  tb_message_window.cpp
  tb_msg.cpp
  tb_node_ref_tree.cpp
  tb_node_tree.cpp
  tb_object.cpp
  tb_popup_window.cpp
  tb_renderer.cpp
  tb_scroll_container.cpp
  tb_scroller.cpp
  tb_select.cpp
  tb_select_item.cpp
  tb_skin.cpp
  tb_skin_util.cpp
  tb_str.cpp
  tb_style_edit.cpp
  tb_style_edit_content.cpp
  tb_system_android.cpp
  tb_system_linux.cpp
  tb_system_sdl.cpp
  tb_system_win.cpp
  tb_tab_container.cpp
  tb_tempbuffer.cpp
  tb_toggle_container.cpp
  tb_value.cpp
  tb_widget_skin_condition_context.cpp
  tb_widget_value.cpp
  tb_widgets.cpp
  tb_widgets_common.cpp
  tb_widgets_listener.cpp
  tb_widgets_reader.cpp
  tb_window.cpp
  image/tb_image_manager.cpp
  image/tb_image_widget.cpp
  animation/tb_animation.cpp
  animation/tb_widget_animation.cpp
  image/tb_image_manager.cpp
  image/tb_image_widget.cpp
  parser/tb_parser.cpp
  renderers/tb_renderer_batcher.cpp
  renderers/tb_renderer_gl.cpp
  utf8/utf8.cpp
  )

if(TB_BUILD_DEMO_GLFW OR TB_BUILD_DEMO_SDL2)
 file(GLOB SRCS tests/*.cpp)
 set(LOCAL_SRCS ${LOCAL_SRCS} ${SRCS})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set_property(SOURCE tb_image_loader_stb.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-unused-function")
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
add_library(TurboBadgerLib STATIC ${LOCAL_SRCS})

if (NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install (TARGETS TurboBadgerLib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif ()
