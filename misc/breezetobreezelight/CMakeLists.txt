add_executable(breezetobreezelight main.cpp)

target_link_libraries(breezetobreezelight KF6::ColorScheme)

install(TARGETS breezetobreezelight DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezelight.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
