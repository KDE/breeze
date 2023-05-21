set(breezedecoration_config_SRCS
    ../breezeexceptionlist.cpp
    breezeconfigwidget.cpp
    breezedetectwidget.cpp
    breezeexceptiondialog.cpp
    breezeexceptionlistwidget.cpp
    breezeexceptionmodel.cpp
    breezeitemmodel.cpp
)
ki18n_wrap_ui(breezedecoration_config_SRCS
   ui/breezeconfigurationui.ui
   ui/breezeexceptiondialog.ui
   ui/breezeexceptionlistwidget.ui
)

kcoreaddons_add_plugin(kcm_breezedecoration SOURCES kcm_breezedecoration.cpp ${breezedecoration_config_SRCS} INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
kconfig_add_kcfg_files(kcm_breezedecoration ../breezesettings.kcfgc)
target_include_directories(kcm_breezedecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_breezedecoration
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
    PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::KCMUtils
        breezecommon
)
kcmutils_generate_desktop_file(kcm_breezedecoration)
