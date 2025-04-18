# Generate .txt license file for CPack (PackageMaker requires a file extension)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt @ONLY)

# Workaround to remove c++ compiler macros and defines for Eclipse.
# If c++ macros/defines are set __cplusplus is also set which causes
# problems when compiling freerdp/jni. To prevent this problem we set the macros to "".

if(ANDROID AND CMAKE_EXTRA_GENERATOR STREQUAL "Eclipse CDT4")
  set(CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS "")
  message(STATUS "Disabled CXX system defines for eclipse (workaround).")
endif()

set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/;/\\\\.gitignore;/CMakeCache.txt")

if(NOT WIN32)
  if(APPLE AND (NOT IOS))

    if(WITH_SERVER)
      set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} "mfreerdp-server")
    endif()
  endif()

  if(WITH_X11)
    set(CPACK_PACKAGE_EXECUTABLES "xfreerdp")

    if(WITH_SERVER)
      set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} "xfreerdp-server")
    endif()
  endif()
endif()

set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_TOPLEVEL_TAG "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

string(TOLOWER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_lower)
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME_lower}-${FREERDP_VERSION_FULL}-${CPACK_SYSTEM_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME_lower}-${FREERDP_VERSION_FULL}-${CPACK_SYSTEM_NAME}")

set(CPACK_PACKAGE_NAME "FreeRDP")
set(CPACK_PACKAGE_VENDOR "FreeRDP")
set(CPACK_PACKAGE_VERSION ${FREERDP_VERSION_FULL})
set(CPACK_PACKAGE_VERSION_MAJOR ${FREERDP_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${FREERDP_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${FREERDP_VERSION_REVISION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FreeRDP: A Remote Desktop Protocol Implementation")

set(CPACK_PACKAGE_CONTACT "Marc-Andre Moreau")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "marcandre.moreau@gmail.com")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

set(CPACK_PACKAGE_INSTALL_DIRECTORY "FreeRDP")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")

set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources\\\\FreeRDP_Install.bmp")
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/resources\\\\FreeRDP_Icon_96px.ico")
set(CPACK_NSIS_MUI_UNICON "${PROJECT_SOURCE_DIR}/resource\\\\FreeRDP_Icon_96px.ico")

set(CPACK_COMPONENTS_ALL client server libraries headers symbols tools)

if(MSVC)
  string(FIND ${CMAKE_MSVC_RUNTIME_LIBRARY} "DLL" IS_SHARED)

  if(NOT IS_SHARED STREQUAL "-1")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)

    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries)
  endif()
endif()

set(CPACK_COMPONENT_CLIENT_DISPLAY_NAME "Client")
set(CPACK_COMPONENT_CLIENT_GROUP "Applications")

set(CPACK_COMPONENT_SERVER_DISPLAY_NAME "Server")
set(CPACK_COMPONENT_SERVER_GROUP "Applications")

set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_LIBRARIES_GROUP "Runtime")

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")
set(CPACK_COMPONENT_HEADERS_GROUP "Development")

set(CPACK_COMPONENT_SYMBOLS_DISPLAY_NAME "Symbols")
set(CPACK_COMPONENT_SYMBOLS_GROUP "Development")

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_GROUP "Applications")

set(CPACK_COMPONENT_GROUP_RUNTIME_DESCRIPTION "Runtime")
set(CPACK_COMPONENT_GROUP_APPLICATIONS_DESCRIPTION "Applications")
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "Development")

configure_file("${PROJECT_SOURCE_DIR}/CMakeCPackOptions.cmake.in" "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake")

include(CPack)
