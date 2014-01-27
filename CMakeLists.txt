CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(emas C)

set(EMAS_VERSION 0.1.0)

find_package(BISON)
find_package(FLEX)

ADD_DEFINITIONS(-Wall -std=c99 -D_XOPEN_SOURCE=700 -O2 -DEMAS_VERSION="${EMAS_VERSION}" -g)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(lexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

ADD_EXECUTABLE(emas
emas.c
${BISON_parser_OUTPUTS}
${FLEX_lexer_OUTPUTS}
lexer_utils.c
parser_utils.c
prog.c
dh.c
st.c
keywords.c
writers.c
)

install(TARGETS emas RUNTIME DESTINATION bin)

# vim: tabstop=4
