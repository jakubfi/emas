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

include_directories(${EMELF_INCLUDE_DIRS} ${EMAWP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(emas ${EMELF_LIBRARIES} ${EMAWP_LIBRARIES})

install(TARGETS emas
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# vim: tabstop=4
