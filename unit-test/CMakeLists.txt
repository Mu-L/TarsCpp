project(unit-test)


include_directories(${servant_SOURCE_DIR}/protocol/framework)
include_directories(${servant_SOURCE_DIR}/protocol/servant)

include_directories(${CMAKE_BINARY_DIR}/src/gtest/include)
link_directories(${CMAKE_BINARY_DIR}/src/gtest/lib)
link_directories(${CMAKE_BINARY_DIR}/src/gtest/lib64)
include_directories(./)
include_directories(../)

# set(ENABLE_SHARED OFF)
build_tars_server("unit-test" "")

add_definitions(-DCMAKE_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-test ${LIB_GTEST} tarsmock)

if(TARS_MYSQL)
    target_link_libraries(unit-test ${LIB_MYSQL})
endif()

add_dependencies(unit-test FRAMEWORK-PROTOCOL tarsmock)
