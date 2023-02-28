find_package(KF${QT_MAJOR_VERSION}ConfigWidgets ${KF_MIN_VERSION} REQUIRED)

add_executable(breezetobreezelight main.cpp)

target_link_libraries(breezetobreezelight KF${QT_MAJOR_VERSION}::ConfigWidgets)

install(TARGETS breezetobreezelight DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezelight.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
