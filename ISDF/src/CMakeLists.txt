set(SRC_LIST DAG.cpp input.cpp main.cpp Rational.cpp SDF.cpp StaticSchedule.cpp)
add_executable(isdf ${SRC_LIST})

install(TARGETS isdf
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION bin
	RUNTIME DESTINATION bin)

