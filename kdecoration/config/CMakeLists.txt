kcoreaddons_add_plugin(kcm_breezedecoration SOURCES kcm_breezedecoration.cpp INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
target_include_directories(kcm_breezedecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_breezedecoration breezecommon5 breezedecoration_STATIC KF5::I18n KF5::CoreAddons KF5::ConfigWidgets Qt::DBus)
kcmutils_generate_desktop_file(kcm_breezedecoration)
