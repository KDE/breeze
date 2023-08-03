add_executable(breezetobreezeclassic main.cpp)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(breezetobreezeclassic KF5::ConfigWidgets)
else()
    target_link_libraries(breezetobreezeclassic KF6::ColorScheme)
endif()

install(TARGETS breezetobreezeclassic DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezeclassic.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
