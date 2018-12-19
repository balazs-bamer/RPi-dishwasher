cmake_minimum_required (VERSION 3.7)
project (dishwash)

add_compile_options(-std=c++17)

SET(BOOST_INCLUDEDIR "/usr/include/boost")
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

# EASTL resides in /usr/local/include and /usr/local/lib
include_directories(src ${Boost_INCLUDE_DIRS} /usr/local/include/)

set(TEST_SOURCES src/test-main.cpp src/base.cpp src/test-input.cpp src/staticerror.cpp src/logic.cpp src/test-display.cpp src/automat.cpp src/test-output.cpp src/dishwash.cpp)
set(PROD_SOURCES src/main.cpp src/base.cpp src/input.cpp src/staticerror.cpp src/logic.cpp src/display.cpp src/automat.cpp src/output.cpp src/dishwash.cpp)
set(ALL_HEADERS src/dishwash-config.h src/base.h src/input.h src/staticerror.h src/logic.h src/display.h src/automat.h src/output.h src/dishwash.h)

add_executable(test-dishwash src/test-main.cpp)
target_sources(test-dishwash PRIVATE ${TEST_SOURCES})
target_link_libraries(test-dishwash Threads::Threads ${Threads_LIBRARIES} EASTL ${CURSES_LIBRARIES})

#add_executable(dishwash src/main.cpp)
#target_sources(dishwash PRIVATE ${PROD_SOURCES})
#target_link_libraries(dishwash Threads::Threads ${Threads_LIBRARIES} EASTL)

add_custom_target(all_sources $TEST_SOURCES $PROD_SOURCES $ALL_HEADERS)
