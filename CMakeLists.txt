cmake_minimum_required(VERSION 3.16)
project(breeze)
set(PROJECT_VERSION "5.27.80")
set(PROJECT_VERSION_MAJOR 5)

set(KF_MIN_VERSION "5.102.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT_MIN_VERSION "5.15.2")

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include(FeatureSummary)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH}  ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(GtkUpdateIconCache)

option(WITH_DECORATIONS "Build Breeze window decorations for KWin" ON)

if(WIN32 OR APPLE OR QT_MAJOR_VERSION STREQUAL "5")
    set(WITH_DECORATIONS OFF)
endif()

find_package(KF${QT_MAJOR_VERSION}KCMUtils ${KF_MIN_VERSION} REQUIRED)

if(WITH_DECORATIONS)
    find_package(KDecoration2 REQUIRED)
    add_subdirectory(kdecoration)
endif()

add_subdirectory(colors)
add_subdirectory(cursors)
add_subdirectory(libbreezecommon)
add_subdirectory(kstyle)
add_subdirectory(misc)

option(WITH_WALLPAPERS "Install Breeze default wallpapers" ON)
if (WITH_WALLPAPERS)
    add_subdirectory(wallpapers)
endif()

find_package(KF${QT_MAJOR_VERSION}I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
ki18n_install(po)

include(ECMSetupVersion)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX BREEZE
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/BreezeConfigVersion.cmake"
                  )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/Breeze")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/BreezeConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/BreezeConfig.cmake"
                                  PATH_VARS KDE_INSTALL_FULL_DATADIR
                                  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                  )

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/BreezeConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/BreezeConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
        )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
