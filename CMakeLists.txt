project (cutest C)

add_library (${PROJECT_NAME} CuTest.c)

add_executable (AllTests AllTests.c CuTestTest.c)
target_link_libraries (AllTests cutest)

enable_testing()
add_test (cutest AllTests)

find_package (MSVC QUIET)
if (${MSVC_FOUND})
    MSVC_CRT_SECURE_NO_WARNINGS (cutest)
    MSVC_CRT_SECURE_NO_WARNINGS (AllTests)
endif (${MSVC_FOUND})

set (CUTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "CuTest headers")
set (CUTEST_LIBRARIES cutest CACHE INTERNAL "CuTest libraries")
