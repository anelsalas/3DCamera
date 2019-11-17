cmake_minimum_required( VERSION 3.13)
 
# Create Project
project( 3D_Thermal_Camera )
if(MSVC)
  set(CMAKE_COMPILER_IS_MSVC 1)
endif()

if(CMAKE_COMPILER_IS_MSVC)
  set(CMAKE_CXX_FLAGS_DEFAULT "/DWIN32 /D_WINDOWS /W3 /GR /EHsc")
  add_definitions("-DBOOST_ALL_NO_LIB -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DPCL_ONLY_CORE_POINT_TYPES /bigobj ${SSE_DEFINITIONS}")
  if("${CMAKE_CXX_FLAGS}" STREQUAL "${CMAKE_CXX_FLAGS_DEFAULT}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /fp:precise /wd4800 /wd4521 /wd4251 /wd4275 /wd4305 /wd4355 ${SSE_FLAGS_STR}")

    # Add extra code generation/link optimizations
    if(CMAKE_MSVC_CODE_LINK_OPTIMIZATION)
      SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
      SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF")
      SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
    endif()
    # /MANIFEST:NO")
 # please, don't disable manifest generation, otherwise crash at start for vs2008

    if(MSVC_VERSION GREATER 1500 AND ${CMAKE_VERSION} VERSION_GREATER "2.8.6")
      include(ProcessorCount)
      ProcessorCount(N)
      if(NOT N EQUAL 0)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP${N}")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")
      endif()
    endif()
  endif()

  if(CMAKE_GENERATOR STREQUAL "Ninja")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
  endif()
 endif()

message (STATUS ${CMAKE_CURRENT_LIST_DIR})

include("findBoost.cmake")



list(APPEND CMAKE_MODULE_PATH "C:/Users/Magdalena/Source/PCL/3dcam/trunk")
# set(EIGEN_ROOT "C:/Program Files/Eigen3")
find_package(Eigen)
 
# Find Packages  COMPONENTS common io
#dec 14, moving to PCL 1.9
#find_package( PCL 1.9 REQUIRED )

# intel TBB stuff
include_directories("C:/Program Files (x86)/IntelSWTools/compilers_and_libraries/windows/tbb/include/tbb")
link_directories("C:/Program Files (x86)/IntelSWTools/compilers_and_libraries/windows/tbb/lib/intel64_win/vc14_uwd")


#Jan 2019, moving out of to PCL 1.9 using VTK directly

# find VTK
#find_package(VTK 8.2 COMPONENTS 
find_package(VTK 8.9 COMPONENTS 
# components are now in vtk 8.9, named without vtk prefix
  ChartsCore
  CommonCore
  CommonDataModel
  IOPLY
  InteractionStyle
  RenderingContext2D
  # had to manually add RenderingContextOpenGL2 to vtk's cmake
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2 
  # had to manually add ViewsContext2D to vtk's cmake
  ViewsContext2D 
  #QUIET
  )
if (NOT VTK_FOUND)
  message("Exiting: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()

#include("all_in_one_installer.cmake")


# C:\Program Files\VTK\include\vtk-8.1\vtkSetGet.h in line 935 
#  "# elif __cplusplus >= 201103L && __has_cpp_attribute(gnu::fallthrough)"
#returns error:  the argument to a feature-test macro must be a simple identifier so I commented it.
# This error should be in vtk-8.2 but  VS2017 points to a file in 8.1??!!
# I replaced it with # elif __cplusplus >= 201103L && __has_cpp_attribute(fallthrough) apparently the gnu:: thingy is not a correct
# macro identifier based on the macro standard.



if(VTK_FOUND)
  message(STATUS "VTK found (version: ${VTK_VERSION})")
endif(VTK_FOUND)

#DLN stuff
#include_directories("C:/Users/Magdalena/Source/PCL/3dcam/trunk/libs/dln/common")
#link_directories("C:/Users/Magdalena/Source/PCL/3dcam/trunk/libs/dln/bin")
#pragma comment(lib, "../dln/bin/dln4s_spi.lib")
include_directories("../dln/bin/common")
link_directories("../dln/bin")




add_executable( 3D_Thermal_Camera WIN32  3D_Thermal_Camera.cpp spi_master.cpp utilities.cpp utilities.h spi_master.h vtk_viewer.cpp vtk_viewer.h myvtk_utilities.h myvtk_utilities.cpp )
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "3D_Thermal_Camera" )
target_link_libraries(3D_Thermal_Camera PRIVATE ${VTK_LIBRARIES} ${Boost_LIBRARIES})

