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
)

### config classes
set(breezedecoration_config_SRCS
    breezeexceptionlist.cpp
    config/breezeconfigwidget.cpp
    config/breezedetectwidget.cpp
    config/breezeexceptiondialog.cpp
    config/breezeexceptionlistwidget.cpp
    config/breezeexceptionmodel.cpp
    config/breezeitemmodel.cpp
)
ki18n_wrap_ui(breezedecoration_config_SRCS
   config/ui/breezeconfigurationui.ui
   config/ui/breezeexceptiondialog.ui
   config/ui/breezeexceptionlistwidget.ui
)
add_library(breezedecoration_STATIC STATIC ${breezedecoration_config_SRCS})
# Needed to link this static lib to shared libs
set_property(TARGET breezedecoration_STATIC PROPERTY POSITION_INDEPENDENT_CODE ON)
kconfig_add_kcfg_files(breezedecoration_STATIC breezesettings.kcfgc)
target_link_libraries(breezedecoration_STATIC
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
        KDecoration2::KDecoration
    PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
)

### build library
add_library(breezedecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

set_target_properties(breezedecoration PROPERTIES
    OUTPUT_NAME org.kde.breeze
)

target_link_libraries(breezedecoration
    PRIVATE
        breezecommon5
        breezedecoration_STATIC
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
)

install(TARGETS breezedecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)

add_subdirectory(config)
