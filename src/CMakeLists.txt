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

TARGET_LINK_LIBRARIES(emas emelf)

install(TARGETS emas RUNTIME DESTINATION bin)

# vim: tabstop=4
