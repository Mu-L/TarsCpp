

set(TARS_VERSION "3.0.21")
add_definitions(-DTARS_VERSION="${TARS_VERSION}")

set(CMAKE_VERBOSE_MAKEFILE off)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "set build type to release default")
IF (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
ENDIF()


# 设置一个选项，用于控制是否定义 TARS_STD_SHARED_PTR 宏
option(TARS_STD_SHARED_PTR "Use std::shared_ptr instead of tars::AutoPtr" OFF)
if(TARS_STD_SHARED_PTR)
    add_definitions(-DTARS_STD_SHARED_PTR)
endif()


#编译的可执行程序输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
endforeach()   

option(ONLY_LIB "option for only lib" ON)
option(ENABLE_SHARED "option for enable shared library" OFF)

# option(TARS_OPENTRACKING "option for open tracking" OFF)

# if (TARS_OPENTRACKING)
#     add_definitions(-DTARS_OPENTRACKING=1)
#     set(OPENTRACKING_INC "/usr/local/include")
# endif ()

# set(TARS_OPENTRACKING $ENV{TARS_OPENTRACKING})
# if(TARS_OPENTRACKING)
# set(OPENTRACKING_INC "/usr/local/include")
# add_definitions(-D_USE_OPENTRACKING=${TARS_OPENTRACKING})
# endif()

#-------------------------------------------------------------

IF (UNIX)
    set(INSTALL_PREFIX "/usr/local/tars/cpp" CACHE STRING "set install path" )
ELSE()
    set(INSTALL_PREFIX "c:/tars/cpp" CACHE STRING "set install path")
ENDIF()

#工程INSTALL目录
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE STRING "set install path" FORCE)
endif()

#-------------------------------------------------------------
IF (APPLE)
link_libraries(iconv)
ENDIF(APPLE)

IF (WIN32)

ELSE()
    link_libraries(pthread dl)
ENDIF()

#-------------------------------------------------------------

set(PLATFORM)
IF (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fsigned-char")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fsigned-char -std=c++11  -Wno-deprecated -fno-strict-aliasing -Wno-overloaded-virtual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-builtin-macro-redefined -D__FILE__='\"$(notdir $(abspath $<))\"'")
    
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall -fno-strict-aliasing")

    set(PLATFORM "linux")
    IF(APPLE)
        set(PLATFORM "mac")
        SET(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
        SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
        SET(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
        SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    ENDIF(APPLE)

ELSEIF (WIN32)
    set(PLATFORM "window")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /wd4101 /wd4244 /wd4996 /wd4091 /wd4503 /wd4819 /wd4200 /wd4800 /wd4267 /wd4251 /wd4275 /bigobj")

    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

    #Release生成PDB
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:ICF /OPT:REF /DEBUG")
    SET(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /DEBUG")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:ICF /OPT:REF /DEBUG")

    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")

ELSE ()
    MESSAGE(STATUS "================ ERROR: This platform is unsupported!!! ================")
ENDIF (UNIX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")

IF (WIN32)
    add_definitions(-DNOMINMAX)
ENDIF ()

#-------------------------------------------------------------
set(TARS2CPP "${CMAKE_BINARY_DIR}/bin/tars2cpp" CACHE STRING "set tars2cpp")

message("----------------------------------------------------")
message("CMAKE_SOURCE_DIR:          ${CMAKE_SOURCE_DIR}")
message("CMAKE_BINARY_DIR:          ${CMAKE_BINARY_DIR}")
message("PROJECT_SOURCE_DIR:        ${PROJECT_SOURCE_DIR}")
message("CMAKE_BUILD_TYPE:          ${CMAKE_BUILD_TYPE}")
message("PLATFORM:                  ${PLATFORM}")
message("CMAKE_INSTALL_PREFIX:      ${CMAKE_INSTALL_PREFIX}")
message("BIN:                       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}") 
message("TARS2CPP:                  ${TARS2CPP}") 
#message("TARS_OPENTRACKING:         ${TARS_OPENTRACKING}")
message("ONLY_LIB:                  ${ONLY_LIB}" )
message("TARS_STD_SHARED_PTR:       ${TARS_STD_SHARED_PTR}" )
message("ENABLE_SHARED:             ${ENABLE_SHARED}" )
#-------------------------------------------------------------

