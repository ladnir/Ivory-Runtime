
#project(ivory)


#############################################
#                 Link NTL                  #
#############################################
set(NTL_Lib_Dirs "${NTL_Dirs}/src/")
set(NTL_Include_Dirs "${NTL_Dirs}/include/")

# default location for NTL
set(NTL_LIB "${NTL_Lib_Dirs}libntl.a")


if(NOT EXISTS "${NTL_LIB}")
  # we failed to find it, try using find_library
  find_library(
    NTL_LIB 
    NAMES ntl  
    HINTS NTL_Lib_Dirs)
  
  # if we still cant fint it, throw an error
  if(NOT NTL_LIB)
    Message(${NTL_LIB})
    message(FATAL_ERROR "Failed to find NTL at ${NTL_Lib_Dirs} or at system locations." )
  endif()
else()
  include_directories(${NTL_Include_Dirs})
  #message(" ----  NTL -----  ${NTL_Include_Dirs}")
endif()


#############################################
#                 Link libOTe               #
#############################################

if(NOT IS_DIRECTORY ${libOTe_Dirs}) 
  message(FATAL_ERROR "Bad libOTe location. libOTe_Dirs=${libOTe_Dirs}" )
endif()

set(libOTe_Include_Dirs "${libOTe_Dirs}/libOTe/")
#set(libOTe_LIB "${libOTe_Dirs}/lib/liblibOTe.a")


find_library(
	libOTe_LIB
	NAMES libOTe
	HINTS "${libOTe_Dirs}/lib/")


if(NOT EXISTS "${libOTe_LIB}")
  message(FATAL_ERROR "Failed to find libOTe at ${libOTe_LIB}" )
else()
  include_directories(${libOTe_Include_Dirs}) 
endif()


#############################################
#              Build ivory                  #
#############################################

file(GLOB_RECURSE SRCS *.cpp)
 
include_directories(${CMAKE_SOURCE_DIR}/ivory)

add_library(ivory ${SRCS})

target_link_libraries(ivory ${libOTe_LIB}  cryptoTools ${NTL_LIB})

