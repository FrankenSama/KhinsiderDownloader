cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#organize me!
if(WIN32)
#$ENV{VCPKG_ROOT}
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Path to vcpkg toolchain file")
        message(STATUS "Setting vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

# Keep the CMake project version in sync with the version embedded in the app.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/APP_VERSION" APP_VERSION_FILE)
string(STRIP "${APP_VERSION_FILE}" APP_VERSION_FILE)
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" APP_VERSION_NUMBER "${APP_VERSION_FILE}")
if(NOT APP_VERSION_NUMBER)
    set(APP_VERSION_NUMBER "0.0.0.0")
endif()

project(KhinsiderQT VERSION ${APP_VERSION_NUMBER} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    add_compile_options("/Zc:__cplusplus" "/permissive-")
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Xml Network)
find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

set(QML_IMPORT_PATH "${Qt6_DIR}/../../../qml" CACHE STRING "QML import paths")
set(QML_IMPORT_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/search"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/download/"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/settings"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/shared"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
        "${CMAKE_CURRENT_SOURCE_DIR}/icons"
        CACHE STRING "QML import paths" FORCE
)

if(WIN32)
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc")
endif()

qt_standard_project_setup(REQUIRES 6.8)

set(CMAKE_AUTORCC ON)

set(QRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} resources.qrc)
qt_add_resources(RESOURCE_SRCS ${QRC_FILE})

qt_add_executable(appKhinsiderQT
        src/main.cpp
        src/pages/search/SearchController.cpp
        src/pages/search/SearchController.h
        ${PUGIXML_SOURCES}
        src/parser/HTMLParserHelper.cpp
        src/parser/KhinsiderParser.cpp
        src/pages/search/SearchResultModel.h
        src/pages/search/SearchAlbumInfoViewModel.h
        src/ImagePrecache.cpp
        src/ImagePrecache.h
        src/App.cpp
        ${RESOURCE_SRCS}
        src/pages/settings/SettingsController.cpp
        src/pages/settings/SettingsController.h
        src/pages/settings/SettingsViewModel.h
        src/pages/settings/Settings.cpp
        src/pages/settings/Settings.h
        src/downloader/DownloadManager.cpp
        src/downloader/DownloadManager.h
        src/downloader/DownloadWorker.cpp
        src/downloader/DownloadWorker.h
        src/pages/download/DownloaderController.cpp
        src/pages/download/DownloaderController.h
        src/pages/download/DownloaderModel.h
        src/downloader/WDownloadReply.h
        src/pages/download/DownloaderModel.cpp
        src/parser/Song.h
        src/parser/Album.cpp
        src/parser/Album.h
		${app_icon_resource_windows}
		src/pages/about/aboutcontroller.h 
		src/pages/about/aboutcontroller.cpp
        src/LogController.cpp
        src/LogController.h
)

target_include_directories(appKhinsiderQT PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/parser"
        ${CURL_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIRS}
        ${Qt6Network_INCLUDE_DIRS}
)

qt_add_qml_module(appKhinsiderQT
        URI KhinsiderQT
        VERSION 1.0
        QML_FILES
        src/ui/Main.qml
        
        # Shared components
        src/ui/shared/SideButton.qml
        src/ui/shared/WButton.qml
        src/ui/shared/WEnumButton.qml
        src/ui/shared/WNumberBox.qml
        src/ui/shared/WNumberBoxButton.qml
        src/ui/shared/WScrollView.qml

        # Search components
        src/ui/search/AlbumImageCaret.qml
        src/ui/search/AlbumImageCaretButton.qml
        src/ui/search/AlbumInfoSide.qml
        src/ui/search/SearchPanel.qml
        src/ui/search/SearchResultsList.qml
        
        # Download components
        src/ui/download/AlbumItem.qml
        src/ui/download/DownloadPanel.qml
        src/ui/download/DownloadSide.qml

        # About
        src/ui/about/AboutPanel.qml
        src/ui/about/UpdateCheckerDialog.qml

        # Settings components
        src/ui/settings/SettingsPanel.qml

        RESOURCES resources.qrc

)

set_target_properties(appKhinsiderQT PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(appKhinsiderQT
        PRIVATE Qt6::Quick Qt6::Xml Qt6::Network CURL::libcurl LibXml2::LibXml2
)

include(GNUInstallDirs)
install(TARGETS appKhinsiderQT
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(KHINSIDER_BUILD_TESTS "Build parser regression tests" OFF)
if(KHINSIDER_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(khinsider_parser_tests
            tests/parser_tests.cpp
            src/parser/KhinsiderParser.cpp
            src/parser/HTMLParserHelper.cpp
            src/parser/Album.cpp
            src/parser/Album.h
            src/parser/Song.h
            src/downloader/WDownloadReply.h
    )

    target_include_directories(khinsider_parser_tests PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/parser"
            ${LIBXML2_INCLUDE_DIRS}
    )

    target_link_libraries(khinsider_parser_tests PRIVATE
            Qt6::Test
            Qt6::Network
            Qt6::Xml
            LibXml2::LibXml2
    )

    add_test(NAME parser_tests COMMAND khinsider_parser_tests)
endif()
