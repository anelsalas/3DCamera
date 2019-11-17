
set(BOOST_INCLUDEDIR "C:/Program Files/Boost/include/boost-1_69")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC ON)
set(Boost_USE_MULTITHREAD )

#  set(Boost_USE_STATIC_LIBS OFF)
#  set(Boost_USE_STATIC OFF)
#  set(Boost_USE_MULTITHREAD ON)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_ALL_DYN_LINK -DBOOST_ALL_NO_LIB")


set(Boost_ADDITIONAL_VERSIONS "1.69.0" "1.69")
    
set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_REQUIRED_MODULES filesystem thread date_time iostreams system)
  
find_package(Boost 1.69.0  REQUIRED COMPONENTS ${BOOST_REQUIRED_MODULES})
#find_library(Boost 1.69.0  REQUIRED COMPONENTS ${BOOST_REQUIRED_MODULES})
   
if(Boost_FOUND)
  message(STATUS "boost_FOUND")

  set(BOOST_FOUND TRUE)
  # Obtain diagnostic information about Boost's automatic linking outputted 
  # during compilation time.
  add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
  include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
  message(STATUS "boostincludedirs:" ${Boost_INCLUDE_DIRS} " boost library dirs " ${Boost_LIBRARY_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_FOUND)






