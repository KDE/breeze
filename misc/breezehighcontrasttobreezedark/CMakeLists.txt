find_package(KF${QT_MAJOR_VERSION}ConfigWidgets ${KF_MIN_VERSION} REQUIRED)

add_executable(breezehighcontrasttobreezedark main.cpp)

target_link_libraries(breezehighcontrasttobreezedark KF${QT_MAJOR_VERSION}::ConfigWidgets)

install(TARGETS breezehighcontrasttobreezedark DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezehighcontrasttobreezedark.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
