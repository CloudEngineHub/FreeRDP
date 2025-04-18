# FreeRDP: A Remote Desktop Protocol Implementation
# FreeRDP cmake build script
#
# Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
# Copyright 2024 Armin Novak <anovak@thincast.com>
# Copyright 2024 Thincast Technologies GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(MODULE_NAME "freerdp-channels-client")
set(MODULE_PREFIX "FREERDP_CHANNELS_CLIENT")

set(${MODULE_PREFIX}_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/tables.c ${CMAKE_CURRENT_SOURCE_DIR}/tables.h ${CMAKE_CURRENT_SOURCE_DIR}/addin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/addin.h ${CMAKE_CURRENT_SOURCE_DIR}/generic_dynvc.c
)

if(CHANNEL_STATIC_CLIENT_ENTRIES)
  list(REMOVE_DUPLICATES CHANNEL_STATIC_CLIENT_ENTRIES)
endif()

foreach(STATIC_ENTRY ${CHANNEL_STATIC_CLIENT_ENTRIES})
  foreach(STATIC_MODULE ${CHANNEL_STATIC_CLIENT_MODULES})
    foreach(ENTRY ${${STATIC_MODULE}_CLIENT_ENTRY})
      if(${ENTRY} STREQUAL ${STATIC_ENTRY})
        set(STATIC_MODULE_NAME ${${STATIC_MODULE}_CLIENT_NAME})
        set(STATIC_MODULE_CHANNEL ${${STATIC_MODULE}_CLIENT_CHANNEL})
        list(APPEND ${MODULE_PREFIX}_LIBS ${STATIC_MODULE_NAME})

        set(ENTRY_POINT_NAME "${STATIC_MODULE_CHANNEL}_${ENTRY}")
        if(${ENTRY} STREQUAL "VirtualChannelEntry")
          set(ENTRY_POINT_IMPORT "extern BOOL VCAPITYPE ${ENTRY_POINT_NAME}(PCHANNEL_ENTRY_POINTS);")
        elseif(${ENTRY} STREQUAL "VirtualChannelEntryEx")
          set(ENTRY_POINT_IMPORT "extern BOOL VCAPITYPE ${ENTRY_POINT_NAME}(PCHANNEL_ENTRY_POINTS,PVOID);")
        elseif(${ENTRY} MATCHES "DVCPluginEntry$")
          set(ENTRY_POINT_IMPORT "extern UINT VCAPITYPE ${ENTRY_POINT_NAME}(IDRDYNVC_ENTRY_POINTS* pEntryPoints);")
        elseif(${ENTRY} MATCHES "DeviceServiceEntry$")
          set(ENTRY_POINT_IMPORT
              "extern UINT VCAPITYPE ${ENTRY_POINT_NAME}(PDEVICE_SERVICE_ENTRY_POINTS pEntryPoints);"
          )
        else()
          set(ENTRY_POINT_IMPORT "extern UINT VCAPITYPE ${ENTRY_POINT_NAME}(void);")
        endif()

        string(APPEND ${STATIC_ENTRY}_IMPORTS "\n${ENTRY_POINT_IMPORT}")
        string(APPEND ${STATIC_ENTRY}_TABLE "\n\t{ \"${STATIC_MODULE_CHANNEL}\", ${ENTRY_POINT_NAME} },")
      endif()
    endforeach()
  endforeach()
endforeach()

string(APPEND CLIENT_STATIC_ENTRY_TABLES_LIST "\nextern const STATIC_ENTRY_TABLE CLIENT_STATIC_ENTRY_TABLES[];\n")
string(APPEND CLIENT_STATIC_ENTRY_TABLES_LIST "const STATIC_ENTRY_TABLE CLIENT_STATIC_ENTRY_TABLES[] =\n{")

foreach(STATIC_ENTRY ${CHANNEL_STATIC_CLIENT_ENTRIES})
  set(CLIENT_STATIC_ENTRY_IMPORTS "${CLIENT_STATIC_ENTRY_IMPORTS}\n${${STATIC_ENTRY}_IMPORTS}")
  if(${STATIC_ENTRY} STREQUAL "VirtualChannelEntry")
    set(CLIENT_STATIC_ENTRY_TYPE "STATIC_ENTRY_VC")
    set(CLIENT_STATIC_ENTRY_INITIALIZER ".csevc")
  elseif(${STATIC_ENTRY} STREQUAL "VirtualChannelEntryEx")
    set(CLIENT_STATIC_ENTRY_TYPE "STATIC_ENTRY_VCEX")
    set(CLIENT_STATIC_ENTRY_INITIALIZER ".csevcex")
  elseif(${STATIC_ENTRY} MATCHES "DVCPluginEntry$")
    set(CLIENT_STATIC_ENTRY_TYPE "STATIC_ENTRY_DVC")
    set(CLIENT_STATIC_ENTRY_INITIALIZER ".csedvc")
  elseif(${STATIC_ENTRY} MATCHES "DeviceServiceEntry$")
    set(CLIENT_STATIC_ENTRY_TYPE "STATIC_ENTRY_DSE")
    set(CLIENT_STATIC_ENTRY_INITIALIZER ".csedse")
  else()
    set(CLIENT_STATIC_ENTRY_TYPE "STATIC_ENTRY")
    set(CLIENT_STATIC_ENTRY_INITIALIZER ".cse")
  endif()

  string(APPEND CLIENT_STATIC_ENTRY_TABLES
         "\nextern const ${CLIENT_STATIC_ENTRY_TYPE} CLIENT_${STATIC_ENTRY}_TABLE[];\n"
  )
  string(APPEND CLIENT_STATIC_ENTRY_TABLES "const ${CLIENT_STATIC_ENTRY_TYPE} CLIENT_${STATIC_ENTRY}_TABLE[] =\n{")
  string(APPEND CLIENT_STATIC_ENTRY_TABLES "\n${${STATIC_ENTRY}_TABLE}")
  string(APPEND CLIENT_STATIC_ENTRY_TABLES "\n\t{ NULL, NULL }\n};")
  string(APPEND CLIENT_STATIC_ENTRY_TABLES_LIST
         "\n\t{ \"${STATIC_ENTRY}\", { ${CLIENT_STATIC_ENTRY_INITIALIZER} = CLIENT_${STATIC_ENTRY}_TABLE } },"
  )
endforeach()

string(APPEND CLIENT_STATIC_ENTRY_TABLES_LIST "\n\t{ NULL, { .cse = NULL } }\n};")

set(CLIENT_STATIC_ADDIN_TABLE "extern const STATIC_ADDIN_TABLE CLIENT_STATIC_ADDIN_TABLE[];\n")
string(APPEND CLIENT_STATIC_ADDIN_TABLE "const STATIC_ADDIN_TABLE CLIENT_STATIC_ADDIN_TABLE[] =\n{")

foreach(STATIC_MODULE ${CHANNEL_STATIC_CLIENT_MODULES})
  set(STATIC_MODULE_NAME ${${STATIC_MODULE}_CLIENT_NAME})
  set(STATIC_MODULE_CHANNEL ${${STATIC_MODULE}_CLIENT_CHANNEL})
  string(TOUPPER "CLIENT_${STATIC_MODULE_CHANNEL}_SUBSYSTEM_TABLE" SUBSYSTEM_TABLE_NAME)
  set(SUBSYSTEM_TABLE
      "extern const STATIC_SUBSYSTEM_ENTRY ${SUBSYSTEM_TABLE_NAME}[];\nconst STATIC_SUBSYSTEM_ENTRY ${SUBSYSTEM_TABLE_NAME}[] =\n{"
  )
  get_target_property(CHANNEL_SUBSYSTEMS ${STATIC_MODULE_NAME} SUBSYSTEMS)
  if(CHANNEL_SUBSYSTEMS MATCHES "NOTFOUND")
    set(CHANNEL_SUBSYSTEMS "")
  endif()
  foreach(STATIC_SUBSYSTEM ${CHANNEL_SUBSYSTEMS})
    if(${STATIC_SUBSYSTEM} MATCHES "^([^-]*)-(.*)")
      string(REGEX REPLACE "^([^-]*)-(.*)" "\\1" STATIC_SUBSYSTEM_NAME ${STATIC_SUBSYSTEM})
      string(REGEX REPLACE "^([^-]*)-(.*)" "\\2" STATIC_SUBSYSTEM_TYPE ${STATIC_SUBSYSTEM})
    else()
      set(STATIC_SUBSYSTEM_NAME "${STATIC_SUBSYSTEM}")
      set(STATIC_SUBSYSTEM_TYPE "")
    endif()
    string(LENGTH "${STATIC_SUBSYSTEM_TYPE}" _type_length)
    set(SUBSYSTEM_MODULE_NAME "${STATIC_MODULE_NAME}-${STATIC_SUBSYSTEM}")
    list(APPEND ${MODULE_PREFIX}_LIBS ${SUBSYSTEM_MODULE_NAME})
    if(_type_length GREATER 0)
      set(STATIC_SUBSYSTEM_ENTRY
          "${STATIC_SUBSYSTEM_NAME}_freerdp_${STATIC_MODULE_CHANNEL}_client_${STATIC_SUBSYSTEM_TYPE}_subsystem_entry"
      )
    else()
      set(STATIC_SUBSYSTEM_ENTRY "${STATIC_SUBSYSTEM_NAME}_freerdp_${STATIC_MODULE_CHANNEL}_client_subsystem_entry")
    endif()
    string(APPEND SUBSYSTEM_TABLE
           "\n\t{ \"${STATIC_SUBSYSTEM_NAME}\", \"${STATIC_SUBSYSTEM_TYPE}\", ${STATIC_SUBSYSTEM_ENTRY} },"
    )
    set(SUBSYSTEM_IMPORT "extern UINT VCAPITYPE ${STATIC_SUBSYSTEM_ENTRY}(void*);")
    string(APPEND CLIENT_STATIC_SUBSYSTEM_IMPORTS "\n${SUBSYSTEM_IMPORT}")
  endforeach()
  string(APPEND SUBSYSTEM_TABLE "\n\t{ NULL, NULL, NULL }\n};")
  string(APPEND CLIENT_STATIC_SUBSYSTEM_TABLES "\n${SUBSYSTEM_TABLE}")

  foreach(ENTRY ${${STATIC_MODULE}_CLIENT_ENTRY})
    set(ENTRY_POINT_NAME ${STATIC_MODULE_CHANNEL}_${ENTRY})
    if(${ENTRY} STREQUAL "VirtualChannelEntry")
      set(ENTRY_INITIALIZER ".csevc")
    elseif(${ENTRY} STREQUAL "VirtualChannelEntryEx")
      set(ENTRY_INITIALIZER ".csevcex")
    elseif(${ENTRY} MATCHES "DVCPluginEntry$")
      set(ENTRY_INITIALIZER ".csedvc")
    elseif(${ENTRY} MATCHES "DeviceServiceEntry$")
      set(ENTRY_INITIALIZER ".csedse")
    else()
      set(ENTRY_INITIALIZER ".cse")
    endif()
    string(
      APPEND
      CLIENT_STATIC_ADDIN_TABLE
      "\n\t{ \"${STATIC_MODULE_CHANNEL}\", \"${ENTRY}\", { ${ENTRY_INITIALIZER} = ${ENTRY_POINT_NAME} }, ${SUBSYSTEM_TABLE_NAME} },"
    )
  endforeach()
endforeach()
string(APPEND CLIENT_STATIC_ADDIN_TABLE "\n\t{ NULL, NULL, { .cse = NULL }, NULL }\n};")

cleaning_configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tables.c.in ${CMAKE_CURRENT_BINARY_DIR}/tables.c)

set(${MODULE_PREFIX}_LIBS ${${MODULE_PREFIX}_LIBS} freerdp winpr)

set(${MODULE_PREFIX}_SRCS ${${MODULE_PREFIX}_SRCS} PARENT_SCOPE)
set(${MODULE_PREFIX}_LIBS ${${MODULE_PREFIX}_LIBS} PARENT_SCOPE)
