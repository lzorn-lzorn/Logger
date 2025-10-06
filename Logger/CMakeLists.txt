set(TARGET_NAME Logger)

add_library(${TARGET_NAME} SHARED
	src/Logger.cpp
)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDE_DIR})
