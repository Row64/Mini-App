cmake_minimum_required (VERSION 3.0)
project( MiniApp )


#################################################################################
# CMAKE flags and platform specific definitions 								#
#################################################################################

# CMAKE 3.20 and later (3.20 is included in Visual Studio 2019) introduced a new
# policy of requiring source files to be specified with explicit extensions.
# To minimize changes to third-party external libraries, set this variable to
# revert to the pre-3.20 policy, which silences configuration-time warnings.
# See: https://cmake.org/cmake/help/latest/variable/CMAKE_POLICY_DEFAULT_CMPNNNN.html
set(CMAKE_POLICY_DEFAULT_CMP0115 OLD)

if ( WIN32 )
	set(CMAKE_CXX_STANDARD 17)
elseif ( APPLE )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++1z -Wno-c++11-narrowing" )
elseif ( UNIX )
	set(CMAKE_CXX_STANDARD 17)
endif()

#################################################################################
# Project options						 										#
#################################################################################

add_compile_definitions("R64_MINI")

#################################################################################
# Project specific includes 													#
#################################################################################

include_directories( "Include" )
include_directories( "Include/Core" )
include_directories( "Include/IO" )
include_directories( "Include/Layouts" )
include_directories( "Include/PipelineSetup" )
include_directories( "Include/Networking" )
include_directories( "Include/TUI_Panes" )
include_directories( "Include/User" )

include_directories( "UILib" )
include_Directories( "Libraries/MathLib")
include_Directories( "Libraries/StringLib")
include_Directories( "Libraries/FileLib")

include_directories( "Blocks" )
include_directories( "Blocks/Chart" )
include_directories( "Blocks/Fonts" )
include_directories( "Blocks/Modal" )
include_directories( "Blocks/PPG" )
include_directories( "Blocks/Widget" )

include_directories( "External" )
include_directories( "External/fileformats" )
include_directories( "External/imgui" )
include_directories( "External/glm" )
include_directories( "External/poly2tri" )
include_directories( "External/ColorSpace/src" )
include_directories( "External/miniz" )

include_directories( "External/cpp-httplib" )

include_directories( "External/websocketpp" )
include_directories( "External/websocketpp/websocketpp" )
include_directories( "External/websocketpp/websocketpp/config" )
include_directories( "External/websocketpp/websocketpp/transport" )
include_directories( "External/websocketpp/websocketpp/transport/asio" )
include_directories( "External/websocketpp/websocketpp/base64/base64" )

file( GLOB_RECURSE SHARED_SOURCE_FILES
	"Include/*.cpp" 
	"UILib/*.cpp"
	"Libraries/*.cpp"
	"Blocks/*.cpp"
)

file( GLOB EXTERNAL_SOURCE_FILES
	"External/imgui/*.cpp"
	"External/*.cpp"
	"External/poly2tri/*.cc"
	"External/poly2tri/common/*.cc"
	"External/poly2tri/sweep/*.cc"
	"External/ColorSpace/src/*.cpp"
    "External/fileformats/*.cpp"
    "External/miniz/*.c"
)

set( ALL_SHARED_FILES ${SHARED_SOURCE_FILES} ${EXTERNAL_SOURCE_FILES} )

# Add .RC file for executable icon
if ( WIN32 )
    list(APPEND ALL_SHARED_FILES ${PROJECT_SOURCE_DIR}/Resources/RESOURCES.RC )
    include_directories( "${PROJECT_SOURCE_DIR}/Resources" )
endif()

#################################################################################
# Add Executable 																#
#################################################################################

# Executable
add_executable( ${PROJECT_NAME} ${ALL_SHARED_FILES}
	main.cpp
)

#################################################################################
# External Libraries 															#
#################################################################################

# OpenSSL -----------------------------------------------------------------------

if(WIN32)

	set ( OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/External/openssl-1.1/x64 )
	include_directories( ${OPENSSL_ROOT_DIR}/include)

    find_library(OPENSSL_LIBRARY
        NAMES libssl
        PATHS
        ${OPENSSL_ROOT_DIR}/lib
        NO_DEFAULT_PATH
	)
	
	find_library(CRYPTO_LIBRARY
        NAMES libcrypto
        PATHS
        ${OPENSSL_ROOT_DIR}/lib
        NO_DEFAULT_PATH
    )

    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})

    file( GLOB DLLS "${OPENSSL_ROOT_DIR}/bin/*.dll" )
	file( COPY ${DLLS} DESTINATION "${CMAKE_BINARY_DIR}" )

elseif( UNIX )

	find_package(OpenSSL REQUIRED)
	
	if( OPENSSL_FOUND)
		message(STATUS "Found OpenSSL: " ${OPENSSL_INCLUDE_DIR})
		include_directories(${OPENSSL_INCLUDE_DIR})
		target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto )
	else()
		message(STATUS "Could not find OpenSSL")
	endif()


endif()



# GLFW library ------------------------------------------------------------------

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(External/glfw)
target_link_libraries (${PROJECT_NAME} glfw)

# Direct X / dxguid -------------------------------------------------------------
# https://microsoft.public.win32.programmer.directx.video.narkive.com/90n37Xku/lnk-error-in-release-only

if(WIN32)

    find_library(DXGUID_LIBRARY
        NAMES dxguid
	)

	target_link_libraries(${PROJECT_NAME} ${DXGUID_LIBRARY})

	# Direct X
	target_link_libraries( ${PROJECT_NAME} d3d11.lib )
	target_link_libraries( ${PROJECT_NAME} dxgi.lib )
	target_link_libraries( ${PROJECT_NAME} d3d9.lib )

	# Find editor associated with .txt files
	target_link_libraries( ${PROJECT_NAME} shlwapi.lib )

endif()

# Boost -------------------------------------------------------------------------

if ( WIN32 )
	set(boost_min_ver 1.70.0)
	set(BOOST_ROOT "C:/local/boost_1_70_0")
	set(BOOST_INCLUDEDIR "${BOOST_ROOT}/boost")
	set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-14.2")
	set(Boost_USE_STATIC_LIBS	ON)
	set(Boost_ARCHITECTURE 		"-x64")
elseif ( APPLE )
	set(boost_min_ver 1.69.0)
endif()

set(boost_libs system filesystem date_time regex random)
find_package(Boost ${boost_min_ver})

if(Boost_FOUND)
	find_package(Boost ${boost_min_ver} COMPONENTS ${boost_libs})
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

if ( UNIX )
	target_link_libraries( ${PROJECT_NAME} stdc++fs)
endif()

#################################################################################
# Data Copies       															#
#################################################################################

# Macro to copy a directory to the /build folder
macro( COPY_FOLDER _foldername )
    file( GLOB_RECURSE ${_foldername}_FILES "${PROJECT_SOURCE_DIR}/${_foldername}/*" )
    add_custom_target(
        ${PROJECT_NAME}_${_foldername}
        COMMENT "Copying folder: ${_foldername}"
        DEPENDS ${_foldername}_FILES
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/${_foldername}/ ${CMAKE_BINARY_DIR}/${_foldername}/
    )
    add_dependencies( ${PROJECT_NAME} ${PROJECT_NAME}_${_foldername} )
endmacro()

COPY_FOLDER( Data )


#################################################################################
# Windows App Manifest  														#
#################################################################################
# Need application manifest file so we can query the proper Windows Version.
# https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa
# https://docs.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
# https://docs.microsoft.com/en-us/cpp/build/how-to-embed-a-manifest-inside-a-c-cpp-application?view=msvc-170
IF ( WIN32 )
	SET ( CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO )
	ADD_CUSTOM_COMMAND(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/Row64.manifest\" -outputresource:\"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe\"\;\#1
		COMMENT "Embedding manifest file..." 
	)
ENDIF(WIN32)