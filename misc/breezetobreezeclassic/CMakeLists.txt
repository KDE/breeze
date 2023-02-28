find_package(KF${QT_MAJOR_VERSION}ConfigWidgets ${KF_MIN_VERSION} REQUIRED)

add_executable(breezetobreezeclassic main.cpp)

target_link_libraries(breezetobreezeclassic KF${QT_MAJOR_VERSION}::ConfigWidgets)

install(TARGETS breezetobreezeclassic DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezeclassic.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
