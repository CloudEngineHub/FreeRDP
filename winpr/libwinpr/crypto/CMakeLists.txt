# WinPR: Windows Portable Runtime
# libwinpr-crypto cmake build script
#
# Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

set(SRCS hash.c rand.c cipher.c cert.c crypto.c crypto.h)
if(WITH_INTERNAL_RC4)
  list(APPEND SRCS rc4.c rc4.h)
endif()

if(WITH_INTERNAL_MD4)
  list(APPEND SRCS md4.c md4.h)
endif()

if(WITH_INTERNAL_MD5)
  list(APPEND SRCS md5.c md5.h)
  list(APPEND SRCS hmac_md5.c hmac_md5.h)
endif()

winpr_module_add(${SRCS})

if(OPENSSL_FOUND)
  winpr_system_include_directory_add(${OPENSSL_INCLUDE_DIR})
  winpr_library_add_private(${OPENSSL_LIBRARIES})
endif()

if(MBEDTLS_FOUND)
  winpr_system_include_directory_add(${MBEDTLS_INCLUDE_DIR})
  winpr_library_add_private(${MBEDTLS_LIBRARIES})
endif()

if(WIN32)
  winpr_library_add_public(crypt32)
endif()

if(BUILD_TESTING_INTERNAL OR BUILD_TESTING)
  add_subdirectory(test)
endif()
