
#project(ivoryTests)




#############################################
#                 Link libOTe               #
#############################################

 
find_library(
  libOTe_LIB 
  NAMES libOTe 
  HINTS "${libOTe_Dirs}/lib/")

if(NOT EXISTS "${libOTe_LIB}")
  message(FATAL_ERROR "Failed to find libOTe at ${libOTe_LIB}" )
endif()

 

#############################################
#               Build ivoryTests            #
#############################################

file(GLOB_RECURSE SRCS *.cpp)

include_directories(${CMAKE_SOURCE_DIR}/ivoryTests)


add_library(ivoryTests ${SRCS})

target_link_libraries(ivoryTests ivory)
