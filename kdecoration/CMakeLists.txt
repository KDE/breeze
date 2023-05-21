add_definitions(-DTRANSLATION_DOMAIN="breeze_kwin_deco")

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets I18n IconThemes)
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS DBus)

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# newt target #################
### plugin classes
set(breezedecoration_SRCS
    breezebutton.cpp
    breezedecoration.cpp
    breezesettingsprovider.cpp
    breezeexceptionlist.cpp
)

### build library
kconfig_add_kcfg_files(breezedecoration_SRCS breezesettings.kcfgc)
add_library(breezedecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

set_target_properties(breezedecoration PROPERTIES
    OUTPUT_NAME org.kde.breeze
)

target_link_libraries(breezedecoration
    PRIVATE
        breezecommon
        Qt::DBus
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KDecoration2::KDecoration
)

install(TARGETS breezedecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

if (TARGET "KF${QT_MAJOR_VERSION}::KCMUtils")
    add_subdirectory(config)
endif()
