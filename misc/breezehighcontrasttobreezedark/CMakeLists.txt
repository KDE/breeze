add_executable(breezehighcontrasttobreezedark main.cpp)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(breezehighcontrasttobreezedark KF5::ConfigWidgets)
else()
    target_link_libraries(breezehighcontrasttobreezedark KF6::ColorScheme)
endif()

install(TARGETS breezehighcontrasttobreezedark DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezehighcontrasttobreezedark.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
