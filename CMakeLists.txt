project (cutest C)

FILE (GLOB LIB_HDR *.h)
add_library (${PROJECT_NAME} CuTest.c ${LIB_HDR})

add_executable (CuTestTest AllTests.c CuTestTest.c)
target_link_libraries (CuTestTest cutest)

enable_testing()
add_test (cutest CuTestTest)

set (CUTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "CuTest headers")
set (CUTEST_LIBRARIES cutest CACHE INTERNAL "CuTest libraries")
