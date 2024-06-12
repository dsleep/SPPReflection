cmake_minimum_required(VERSION 3.5)
project(SPPGLOBAL VERSION 1.0.0 LANGUAGES CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

message( STATUS "BUILD SYSTEM: ${CMAKE_SYSTEM}" )
message( STATUS "BUILD SYSTEM NAME: ${CMAKE_SYSTEM_NAME}" ) 
message( STATUS "BUILD SYSTEM PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )

#General C++ Compiling
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "d" )

if(MSVC)

	set(USE_VS_MULTIPROCESS ON CACHE BOOL "Use MultiProcessor compiling on VS")
		
	if(USE_VS_MULTIPROCESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	endif()
	
endif()

##########
add_library(SPPReflection SHARED "")
set_target_properties(SPPReflection PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_compile_definitions(SPPReflection
	PRIVATE
		SPP_REFLECTION_EXPORT=1)
				
target_sources(SPPReflection
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/inc/SPPReflection.h"
		"${CMAKE_CURRENT_LIST_DIR}/inc/SPPRDataManipulators.h"
		"${CMAKE_CURRENT_LIST_DIR}/inc/SPPRTypeTraits.h"

		"${CMAKE_CURRENT_LIST_DIR}/src/SPPReflection.cpp"
		)

target_include_directories(SPPReflection
	PUBLIC  	
		"${CMAKE_CURRENT_LIST_DIR}/inc" )
			

##########
add_executable( testGame "")
add_dependencies( testGame SPPReflection )		
target_sources( testGame 
	PRIVATE 
		"${CMAKE_CURRENT_LIST_DIR}/testGame.cpp" )		
target_link_libraries( testGame SPPReflection )
set_target_properties( testGame PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})