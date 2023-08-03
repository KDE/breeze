add_executable(breezetobreezelight main.cpp)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(breezetobreezelight KF5::ConfigWidgets)
else()
    target_link_libraries(breezetobreezelight KF6::ColorScheme)
endif()

install(TARGETS breezetobreezelight DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
install(FILES breezetobreezelight.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
