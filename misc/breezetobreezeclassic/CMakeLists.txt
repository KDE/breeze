add_executable(breezetobreezeclassic main.cpp)


target_link_libraries(breezetobreezeclassic KF6::ColorScheme)

install(TARGETS breezetobreezeclassic DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezeclassic.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
