add_executable(breezehighcontrasttobreezedark main.cpp)

target_link_libraries(breezehighcontrasttobreezedark KF6::ColorScheme)

install(TARGETS breezehighcontrasttobreezedark DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezehighcontrasttobreezedark.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
