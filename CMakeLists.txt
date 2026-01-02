cmake_minimum_required(VERSION 3.12)

# partial module - included by src/cmake/CMakeLists.txt

#set(TGT test-${PKG}-cmake)
set( CMAKE_CXX_STANDARD 17 )
if (CMAKE_GENERATOR MATCHES "Visual Studio")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /W3 /wd4061 /wd4100 /wd4820 /wd4514")
else()
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Ofast")
endif()
if(STATIC)
set( BUILD_SHARED_LIBRARIES OFF)
set( Boost_USE_STATIC_LIBS ON )
set( CMAKE_EXE_LINKER_FLAGS "-static")
endif()
add_definitions(-DTYRANT_OPTIMIZER_VERSION="${VERSION}" ${DEBUG})


project(tuo)
file(GLOB TUO_SOURCES "*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/hPMML/include)

enable_language(CXX)
add_executable(tuo ${TUO_SOURCES})

option(WITH_QUEST "With Quests enabled" OFF)
if(NOT WITH_QUEST)
add_definitions(-DNQUEST)
endif()
option(USE_OPENMP "Use OpenMP instead of Boost" OFF)
if(USE_OPENMP)
# src: https://stackoverflow.com/a/12404666
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
endif()
find_package(Boost COMPONENTS system thread filesystem regex timer REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )

target_link_libraries(tuo ${Boost_LIBRARIES} )

install(TARGETS tuo)
#install(TARGETS ${TGT} DESTINATION bin)
