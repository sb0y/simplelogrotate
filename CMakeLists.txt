project(logrotate)
cmake_minimum_required(VERSION 3.3)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
	"${PROJECT_SOURCE_DIR}/logrotate.cpp"
	"${PROJECT_SOURCE_DIR}/logrotate.hpp"
	"${PROJECT_SOURCE_DIR}/main.cpp"
)

include_directories( 
"${PROJECT_SOURCE_DIR}" 
)

add_executable(${PROJECT_NAME} ${SRC})

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror -Wextra -pedantic)

#target_link_libraries(${PROJECT_NAME}
#)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
