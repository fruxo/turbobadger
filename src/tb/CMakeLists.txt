cmake_minimum_required(VERSION 2.8)

configure_file(${CMAKE_CURRENT_LIST_DIR}/tb_config.h.in ${CMAKE_CURRENT_LIST_DIR}/tb_config.h)

set(LOCAL_SRCS "")
aux_source_directory(. LOCAL_SRCS)
aux_source_directory(image LOCAL_SRCS)
aux_source_directory(animation LOCAL_SRCS)
aux_source_directory(parser LOCAL_SRCS)
aux_source_directory(renderers LOCAL_SRCS)
aux_source_directory(tests LOCAL_SRCS)
aux_source_directory(utf8 LOCAL_SRCS)

add_library(TurboBadgerLib ${LOCAL_SRCS})

if(TB_USE_FREETYPE)
	find_package(Freetype REQUIRED)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(TurboBadgerLib ${FREETYPE_LIBRARIES})
endif()
