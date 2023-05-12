
#project(ivory)

#############################################
#              Build ivory                  #
#############################################

file(GLOB_RECURSE SRCS *.cpp)
add_library(ivory ${SRCS})
target_include_directories(ivory PUBLIC ${CMAKE_SOURCE_DIR}/ivory)



###########################################################################  
#                        Link external libraries                          #
#                        -----------------------                          #
#                                                                         #
#  Define the expected location for miracl and boost.                     #
#  Boost will be found using the findBoost  module in CMake               #
#  It should look in the location specified and then look elsewhere       # 
#                                                                         #
###########################################################################  


#                 Link libOTe               
###########################################################################

if(NOT IS_DIRECTORY ${libOTe_Dirs}) 
  message(FATAL_ERROR "Bad libOTe location. libOTe_Dirs=${libOTe_Dirs}" )
endif()


find_package(libOTe REQUIRED HINTS ${libOTe_Dirs})

#                 Link NTL                  
###########################################################################
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
  target_include_directories(ivory PUBLIC ${NTL_Include_Dirs})
  #message(" ----  NTL -----  ${NTL_Include_Dirs}")
endif()







target_link_libraries(ivory oc::libOTe)



