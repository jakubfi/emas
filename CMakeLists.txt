cmake_minimum_required(VERSION 3.0)
project(emas 
	DESCRIPTION "MERA-400 assembler with modern syntax"
	LANGUAGES C
)

include(GNUInstallDirs)
include(cmake/appversion.cmake)

add_compile_options(-Wall)

# ---- Target: emas parser/lexer -----------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

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

find_package(emawp 3.0 REQUIRED)

add_executable(emas
	src/emas.c
	src/lexer_utils.c
	src/lexer_utils.h
	src/parser_utils.c
	src/parser_utils.h
	src/prog.c
	src/prog.h
	src/dh.c
	src/dh.h
	src/st.c
	src/st.h
	src/keywords.c
	src/keywords.h
	src/writers.c
	src/writers.h
	${BISON_parser_OUTPUTS}
	${FLEX_lexer_OUTPUTS}
)

target_link_libraries(emas emawp)
if(WIN32)
target_link_libraries(emas ws2_32)
endif(WIN32)

set(EMAS_ASM_INCLUDES_DIR ${CMAKE_INSTALL_PREFIX}/share/emas/include)

set_property(TARGET emas PROPERTY C_STANDARD 99)
target_include_directories(emas PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(emas PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(emas PRIVATE EMAS_VERSION="${APP_VERSION}")
target_compile_definitions(emas PRIVATE EMAS_ASM_INCLUDES="${EMAS_ASM_INCLUDES_DIR}")

install(TARGETS emas RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
	${PROJECT_SOURCE_DIR}/asminc/cpu.inc
	${PROJECT_SOURCE_DIR}/asminc/io.inc
	${PROJECT_SOURCE_DIR}/asminc/mega.inc
	${PROJECT_SOURCE_DIR}/asminc/multix.inc
	DESTINATION ${EMAS_ASM_INCLUDES_DIR}
	COMPONENT dev
)

# vim: tabstop=4
