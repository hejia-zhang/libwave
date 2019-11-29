
message(STATUS "we should look according to the CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message(STATUS "Poco_FIND_COMPONENTS : ${Poco_FIND_COMPONENTS}")

#This module reads hints about search locations from variables::
#
#  POCO_ROOT             - Preferred installation prefix


# The FPHSA helper provides standard way of reporting final search results to
# the user including the version and component checks.
include(FindPackageHandleStandardArgs)


# build up the search location where we will look for our poco header
set(Poco_SEARCH_LOCATIONS "")
if( POCO_ROOT )
    list(APPEND Poco_SEARCH_LOCATIONS ${POCO_ROOT})
elseif( _ENV_POCO_ROOT )
    list(APPEND Poco_SEARCH_LOCATIONS ${_ENV_POCO_ROOT})
endif()
#    /usr/local ??? #could it be found when this is not specified and it would be installed there ??

if(NOT Poco_ROOT_DIR)
    # look for an installed version of POCO
    message(STATUS "Looking for Poco install directory structure.")
    find_path(Poco_ROOT_DIR
            NAMES include/Poco/Version.h
            HINTS ${Poco_SEARCH_LOCATIONS}
            )
    if(NOT Poco_ROOT_DIR)
        # poco was still not found -> Fail
        if(Poco_FIND_REQUIRED)
            message(FATAL_ERROR "Poco: Could not find Poco install directory")
        endif()
        if(NOT Poco_FIND_QUIETLY)
            message(STATUS "Poco: Could not find Poco install directory")
        endif()
        return()
    else()
        # poco was found with the make install directory structure
        message(STATUS "Assuming Poco install directory structure at ${Poco_ROOT_DIR}.")
    endif()
endif()

# ------------------------------------------------------------------------
#  Extract version information from Version.h
# ------------------------------------------------------------------------

if(Poco_ROOT_DIR)
    set(Poco_VERSION_MACRO 0)
    file(STRINGS "${Poco_ROOT_DIR}/include/Poco/Version.h" _poco_VERSION_H_CONTENTS REGEX "#define POCO_VERSION ")
    if("${_poco_VERSION_H_CONTENTS}" MATCHES "#define POCO_VERSION 0x([0-9]+)")
        set(Poco_VERSION_MACRO "${CMAKE_MATCH_1}")
    endif()
    unset(_poco_VERSION_HPP_CONTENTS)

    message(STATUS "Poco_VERSION_MACRO : ${Poco_VERSION_MACRO}")

    # Calculate version components from eg 01090300 (still interpreting as decimal and not hex)
    math(EXPR Poco_VERSION_MAJOR "${Poco_VERSION_MACRO} / 1000000")
    math(EXPR Poco_VERSION_MINOR "${Poco_VERSION_MACRO} / 10000 % 100")
    math(EXPR Poco_VERSION_PATCH "${Poco_VERSION_MACRO} / 100 % 100")

    message(STATUS "Poco_VERSION_MAJOR : ${Poco_VERSION_MAJOR}")
    message(STATUS "Poco_VERSION_MINOR : ${Poco_VERSION_MINOR}")
    message(STATUS "Poco_VERSION_PATCH : ${Poco_VERSION_PATCH}")

    # Define Poco version in x.y.z format
    set(Poco_VERSION_STRING "${Poco_VERSION_MAJOR}.${Poco_VERSION_MINOR}.${Poco_VERSION_PATCH}")

    message(STATUS "Poco_VERSION_STRING : ${Poco_VERSION_STRING}")
endif()


if(Poco_ROOT_DIR)
    set(Poco_INCLUDE_DIRS ${Poco_ROOT_DIR}/include/ CACHE PATH "The global include path for Poco")
endif()


function(_Poco_COMPONENT_HEADERS component _hdrs)
    # Note: new poco components will require adding here if they don't follow the normal scheme.
    if(component STREQUAL "NetSSL")
        set(_Poco_${component}_HEADERS "Net/NetSSL.h")
    elseif(component STREQUAL "Foundation")
        set(_Poco_${component}_HEADERS "Foundation.h")
    else()
        set(_Poco_${component}_HEADERS ${component}/${component}.h)
    endif()

    set(${_hdrs} ${_Poco_${component}_HEADERS} PARENT_SCOPE)
    message(STATUS "Headers for Poco::${component}: ${_Poco_${component}_HEADERS}")
endfunction()

# normally for each component the include directory is the same as the main poco include dir (so no need to search for it again)
# however are the sources of that component present, aka has that component been installed ?
foreach( component ${Poco_FIND_COMPONENTS} )
    #
    # Check if the component header exists, if not ==> component is NOT found
    #
    _Poco_COMPONENT_HEADERS("${component}" Poco_${component}_HEADER_NAME)

    if(EXISTS "${Poco_INCLUDE_DIRS}/Poco/${Poco_${component}_HEADER_NAME}")
        set(Poco_${component}_HEADER ON)
    else()
        set(Poco_${component}_HEADER OFF)
    endif()
    message(STATUS "Poco_${component}_HEADER: ${Poco_${component}_HEADER}")

    #when we have a header for the component, let's hunt for the library
    if(NOT Poco_${component}_HEADER)
        if(NOT Poco_FIND_QUIETLY)
            message(FATAL_ERROR "Could not find Poco component ${component}!")
        endif()
    else()
        # release library
        if(NOT Poco_${component}_LIBRARY)
            find_library(
                    Poco_${component}_LIBRARY
                    NAMES Poco${component}
                    HINTS ${Poco_ROOT_DIR}
                    PATH_SUFFIXES
                    lib
                    bin
            )
        endif()
        if(Poco_${component}_LIBRARY)
            message(STATUS "Found Poco ${component}: ${Poco_${component}_LIBRARY}")
            list(APPEND Poco_LIBRARIES "optimized" ${Poco_${component}_LIBRARY} )
            mark_as_advanced(Poco_${component}_LIBRARY)
        endif()

        # debug library
        if(NOT Poco_${component}_LIBRARY_DEBUG)
            find_library(
                    Poco_${component}_LIBRARY_DEBUG
                    Names Poco${component}d
                    HINTS ${Poco_ROOT_DIR}
                    PATH_SUFFIXES
                    lib
                    bin
            )
        endif()
        if(Poco_${component}_LIBRARY_DEBUG)
            message(STATUS "Found Poco ${component} (debug): ${Poco_${component}_LIBRARY_DEBUG}")
            list(APPEND Poco_LIBRARIES "debug" ${Poco_${component}_LIBRARY_DEBUG})
            mark_as_advanced(Poco_${component}_LIBRARY_DEBUG)
        endif()

        # mark component as found or handle not finding it
        if(Poco_${component}_LIBRARY_DEBUG OR Poco_${component}_LIBRARY)
            set(Poco_${component}_FOUND TRUE)
        elseif(NOT Poco_FIND_QUIETLY)
            message(FATAL_ERROR "Could not find Poco component ${component}!")
        endif()
    endif()
endforeach()


# ------------------------------------------------------------------------
#  Call FPHSA helper, see https://cmake.org/cmake/help/latest/module/FindPackageHandleStandardArgs.html
# ------------------------------------------------------------------------
# Define aliases as needed by the component handler in the FPHSA helper below
#foreach(_comp IN LISTS Boost_FIND_COMPONENTS)
#  string(TOUPPER ${_comp} _uppercomp)
#  if(DEFINED Boost_${_uppercomp}_FOUND)
#    set(Boost_${_comp}_FOUND ${Boost_${_uppercomp}_FOUND})
#  endif()
#endforeach()

message(STATUS "Poco_INCLUDE_DIRS ${Poco_INCLUDE_DIRS}!")

find_package_handle_standard_args(Poco
        REQUIRED_VARS Poco_INCLUDE_DIRS
        VERSION_VAR Poco_VERSION_STRING
        HANDLE_COMPONENTS)


message(STATUS "Poco_FOUND ${Poco_FOUND}")

function(_Poco_COMPONENT_DEPENDENCIES component _ret)
    if(Poco_VERSION_STRING AND Poco_VERSION_STRING VERSION_LESS 1.8.0)
        message(WARNING "Imported targets and dependency information not available for Poco version ${Poco_VERSION_STRING} (all versions older than 1.8)")
    else()
        if(Poco_VERSION_STRING VERSION_LESS 1.9.4)
            set(_Poco_Encodings_DEPENDENCIES Foundation)
            set(_Poco_JSON_DEPENDENCIES Foundation)
            set(_Poco_MongoDB_DEPENDENCIES Foundation Net)
            set(_Poco_Net_DEPENDENCIES Foundation)
            set(_Poco_Redis_DEPENDENCIES Foundation Net)
            set(_Poco_Util_DEPENDENCIES Foundation XML JSON)
            set(_Poco_XML_DEPENDENCIES Foundation)
            set(_Poco_Zip_DEPENDENCIES Foundation Util XML)
        else()
            message(WARNING "New Poco version may have incorrect or missing dependencies for its imported targets")
        endif()
    endif()
    set(${_ret} ${_Poco_${component}_DEPENDENCIES} PARENT_SCOPE)
    message(STATUS "Dependencies for Poco::${component}: ${_Poco_${component}_DEPENDENCIES}")
endfunction()

foreach( component ${Poco_FIND_COMPONENTS} )
    if(NOT TARGET Poco::${component})
        add_library(Poco::${component} STATIC IMPORTED)
        set_target_properties(Poco::${component} PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIRS}"
                IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                IMPORTED_LOCATION "${Poco_${component}_LIBRARY}")
        _Poco_COMPONENT_DEPENDENCIES("${component}" Poco_${component}_DEPENDENCIES)
        if(Poco_${component}_DEPENDENCIES)
            foreach(dep ${Poco_${component}_DEPENDENCIES})
                list(APPEND _Poco_${component}_TARGET_DEPENDENCIES Poco::${dep})
            endforeach()
            message(STATUS "Dependencies for Poco::${component}: ${_Poco_${component}_TARGET_DEPENDENCIES}")
            set_target_properties(Poco::${component} PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${_Poco_${component}_TARGET_DEPENDENCIES}")
        endif()
    endif()
endforeach()

# ------------------------------------------------------------------------
#  Finalize
# ------------------------------------------------------------------------

# Report Poco_LIBRARIES
set(Poco_LIBRARIES2 "")
foreach(component IN LISTS Poco_FIND_COMPONENTS)
    if(Poco_${component}_FOUND)
        list(APPEND Poco_LIBRARIES2 ${Poco_${component}_LIBRARY})
    endif()
endforeach()


message(STATUS "Found Poco: ${Poco_LIBRARIES}")
message(STATUS "Found2 Poco: ${Poco_LIBRARIES2}")