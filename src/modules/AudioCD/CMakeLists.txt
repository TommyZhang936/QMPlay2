cmake_minimum_required(VERSION 2.8.6)
if(COMMAND cmake_policy)
    if(POLICY CMP0003)
        cmake_policy(SET CMP0003 NEW)
    endif()
    if(POLICY CMP0020)
        cmake_policy(SET CMP0020 NEW)
    endif()
    if(POLICY CMP0042)
        cmake_policy(SET CMP0042 NEW)
    endif()
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif()
endif()
project(AudioCD)

set(AudioCD_HDR
    AudioCD.hpp
    AudioCDDemux.hpp
)

set(AudioCD_SRC
    AudioCD.cpp
    AudioCDDemux.cpp
)

set(AudioCD_RESOURCES
    icons.qrc
)

pkg_check_modules(LIBCD libcdio libcddb REQUIRED)
link_directories(${LIBCD_LIBRARY_DIRS})

if(USE_QT5)
    qt5_add_resources(AudioCD_RESOURCES_RCC ${AudioCD_RESOURCES})
else()
    qt4_add_resources(AudioCD_RESOURCES_RCC ${AudioCD_RESOURCES})
endif()

include_directories(../../qmplay2/headers ${LIBCD_INCLUDE_DIRS})

add_library(${PROJECT_NAME} ${QMPLAY2_MODULE}
    ${AudioCD_HDR}
    ${AudioCD_SRC}
    ${AudioCD_RESOURCES_RCC}
)

if(USE_QT5)
    qt5_use_modules(${PROJECT_NAME} Gui Widgets)
else()
    target_link_libraries(${PROJECT_NAME} Qt4::QtCore Qt4::QtGui)
endif()

add_dependencies(${PROJECT_NAME} libqmplay2)
target_link_libraries(${PROJECT_NAME}
    ${qmplay2lib}
)

if(WIN32)
    set(CUSTOM_AUDIOCD_LIBRARIES "" CACHE STRING "Custom libraries for AudioCD")
    mark_as_advanced(CUSTOM_AUDIOCD_LIBRARIES)
endif()

if(WIN32 AND CUSTOM_AUDIOCD_LIBRARIES)
    separate_arguments(CUSTOM_AUDIOCD_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${CUSTOM_AUDIOCD_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} ${LIBCD_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${MODULES_INSTALL_PATH})
