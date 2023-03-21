# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(shared-sources INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/wasm/wasmcryptosettings.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/android/androidcommons.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/shared/platforms/android/androidcommons.h
)

if(ADJUST_TOKEN)
    message(Adjust SDK enabled)
    # SDK Token present, let's enable that.
    add_compile_definitions("MZ_ADJUST")
    target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustfiltering.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusthandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxyconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjustproxypackagehandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/adjust/adjusttasksubmission.h
    )
else()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(${CMAKE_BUILD_TYPE})
        message( FATAL_ERROR "Adjust token cannot be empty for release builds")
    endif()
endif()
