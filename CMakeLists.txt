cmake_minimum_required(VERSION 2.8.4)
project(emas 
	LANGUAGES C
)

include(GNUInstallDirs)
include(appversion.cmake)

# ---- Libraries ---------------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(emelf REQUIRED)
find_package(emawp REQUIRED)

# ---- Sources -----------------------------------------------------------

set(SOURCES_EMAS
	src/emas.c
	src/lexer_utils.c
	src/parser_utils.c
	src/prog.c
	src/dh.c
	src/st.c
	src/keywords.c
	src/writers.c
)

bison_target(parser
	src/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/parser.c
)
flex_target(lexer
	src/lexer.l
	${CMAKE_CURRENT_BINARY_DIR}/lexer.c
)
add_flex_bison_dependency(lexer parser)

# ---- Target: emas ------------------------------------------------------

add_executable(emas
	${SOURCES_EMAS}
	${BISON_parser_OUTPUTS}
	${FLEX_lexer_OUTPUTS}
)

set_property(TARGET emas PROPERTY C_STANDARD 99)
target_include_directories(emas PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(emas PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(emas INTERFACE SYSTEM ${EMELF_INCLUDE_DIRS})
target_include_directories(emas INTERFACE SYSTEM ${EMAWP_INCLUDE_DIRS})
target_compile_definitions(emas PRIVATE EMAS_VERSION="${APP_VERSION}")
target_compile_options(emas PUBLIC -Wall)

target_link_libraries(emas
	${EMELF_LIBRARIES}
	${EMAWP_LIBRARIES}
)

install(TARGETS emas
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
	${PROJECT_SOURCE_DIR}/asminc/cpu.inc
	${PROJECT_SOURCE_DIR}/asminc/io.inc
	${PROJECT_SOURCE_DIR}/asminc/mega.inc
	${PROJECT_SOURCE_DIR}/asminc/multix.inc
	DESTINATION ${CMAKE_INSTALL_DATADIR}/emas/include
	COMPONENT dev
)

# vim: tabstop=4
