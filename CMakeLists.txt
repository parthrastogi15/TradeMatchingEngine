cmake_minimum_required(VERSION 3.5)
project(OptimusPrime_OrderMatchingEngine CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

set(BOOST_ROOT "/usr/local/Cellar/boost/1.82.0_1/include")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
    ${SRC_DIR}/include/FeedErrorStats.cpp
    ${SRC_DIR}/main.cpp
)

set(TEST_SOURCES
    ${SRC_DIR}/include/FeedErrorStats.cpp
    ${SRC_DIR}/test.cpp
)

set(EXECUTABLE TradingEngine)
set(TEST_EXECUTABLE TradingEngineTester)
set(LIBRARY libTradingEngine.so)

include_directories(${BOOST_ROOT})

link_directories(${BOOST_ROOT}/boost/spirit/include)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_TEST "${CMAKE_CXX_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_PERFTEST "${CMAKE_CXX_FLAGS} -O3 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -DPROFILE")
set(CMAKE_CXX_FLAGS_LIB "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -shared")

add_executable(${EXECUTABLE} ${SOURCES})
target_link_libraries(${EXECUTABLE} PRIVATE ${LIBS})

add_executable(${TEST_EXECUTABLE} ${TEST_SOURCES})
target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${LIBS})

add_library(${LIBRARY} SHARED ${SOURCES})

add_custom_target(clean-all COMMAND ${CMAKE_BUILD_TOOL} clean)
