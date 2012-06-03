project (cutest C)

enable_testing()

add_library (cutest CuTest.c)

add_executable (AllTests AllTests.c CuTestTest.c)
target_link_libraries (AllTests cutest)

add_test (cutest AllTests)
