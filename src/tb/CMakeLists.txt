cmake_minimum_required (VERSION 3.4.3)

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
  tb_file_android.cpp
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

if (TB_BUILD_DEMO STREQUAL GLFW OR TB_BUILD_DEMO STREQUAL SDL2)
  set (LOCAL_SRCS ${LOCAL_SRCS}
    tests/tb_test.cpp
    tests/test_tb_color.cpp
    tests/test_tb_dimension.cpp
    tests/test_tb_geometry.cpp
    tests/test_tb_linklist.cpp
    tests/test_tb_node_ref_tree.cpp
    tests/test_tb_object.cpp
    tests/test_tb_parser.cpp
    tests/test_tb_space_allocator.cpp
    tests/test_tb_style_edit.cpp
    tests/test_tb_tempbuffer.cpp
    tests/test_tb_test.cpp
    tests/test_tb_value.cpp
    tests/test_tb_widget_value.cpp
    )
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_property (SOURCE tb_image_loader_stb.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-unused-function")
endif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

if (ANDROID)
  add_library (TurboBadgerLib SHARED ${LOCAL_SRCS})
  target_link_libraries (TurboBadgerLib PUBLIC log atomic android)
  if (TB_RENDERER MATCHES GLES_1)
    target_link_libraries (TurboBadgerLib PUBLIC EGL GLESv1_CM)
  endif (TB_RENDERER MATCHES GLES_1)
  if (TB_RENDERER MATCHES GLES_2)
    target_link_libraries (TurboBadgerLib PUBLIC EGL GLESv2)
  endif (TB_RENDERER MATCHES GLES_2)
else (ANDROID)
  # not android, just link a static
  add_library (TurboBadgerLib STATIC ${LOCAL_SRCS})
endif (ANDROID)

target_compile_features (TurboBadgerLib PRIVATE cxx_constexpr)

if (TB_BACKEND STREQUAL SDL2)
  target_link_libraries (TurboBadgerLib PUBLIC SDL2-interface)
endif (TB_BACKEND STREQUAL SDL2)
if (TB_BACKEND STREQUAL GLFW)
  target_link_libraries (TurboBadgerLib PUBLIC glfw)
endif (TB_BACKEND STREQUAL GLFW)
if (TB_SYSTEM STREQUAL WINDOWS)
  target_link_libraries (TurboBadgerLib PUBLIC GLEW)
endif (TB_SYSTEM STREQUAL WINDOWS)
if (TB_FONT_RENDERER STREQUAL FREETYPE)
  target_link_libraries (TurboBadgerLib PUBLIC freetype)
endif (TB_FONT_RENDERER STREQUAL FREETYPE)
if (TB_RENDERER MATCHES GL AND NOT EMSCRIPTEN AND NOT EMSCRIPTEN AND NOT ANDROID AND NOT IOS)
  set (OpenGL_GL_PREFERENCE GLVND)
  find_package (OpenGL REQUIRED)
  target_link_libraries (TurboBadgerLib PUBLIC OpenGL::GL)
endif (TB_RENDERER MATCHES GL AND NOT EMSCRIPTEN AND NOT EMSCRIPTEN AND NOT ANDROID AND NOT IOS)

target_include_directories (TurboBadgerLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories (TurboBadgerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL AND FALSE)
  install (TARGETS TurboBadgerLib
    COMPONENT library
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include/tb
    COMPONENT library
    PATTERN "*.cpp" EXCLUDE
    PATTERN "*.mm" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    PATTERN "test files" EXCLUDE
    PATTERN "*.h")
  install (
    FILE ${CMAKE_CURRENT_BINARY_DIR}/tb_config.h
    DESTINATION include/tb
    COMPONENT library)
endif ()
