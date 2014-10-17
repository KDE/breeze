add_definitions(-DTRANSLATION_DOMAIN=\"breeze_decoration_config\")

set(kwin_breeze_config_PART_SRCS
    breezeconfig.cpp
    breezeconfigwidget.cpp
    breezedetectwidget.cpp
    breezeexceptiondialog.cpp
    breezeexceptionlistwidget.cpp
    breezeexceptionmodel.cpp
    breezeitemmodel.cpp
    ../breezeexceptionlist.cpp
 )

set(kwin_breeze_config_PART_FORMS
   ui/breezeconfigurationui.ui
   ui/breezedetectwidget.ui
   ui/breezeexceptiondialog.ui
   ui/breezeexceptionlistwidget.ui
)

if(BREEZE_USE_KDE4)
  kde4_add_kcfg_files(kwin_breeze_config_PART_SRCS ../breezeconfiguration.kcfgc)
  kde4_add_ui_files(kwin_breeze_config_PART_FORMS_HEADERS ${kwin_breeze_config_PART_FORMS})
  kde4_add_plugin(kwin_breeze_config
      ${kwin_breeze_config_PART_SRCS}
      ${kwin_breeze_config_PART_FORMS_HEADERS})

  target_link_libraries(
      kwin_breeze_config
      ${KDE4_KDEUI_LIBS}
      ${KDE4WORKSPACE_KDECORATIONS_LIBS}
 )

  if(BREEZE_HAVE_X11)
    target_link_libraries(kwin_breeze_config ${X11_XCB_LIBRARIES})
    target_link_libraries(kwin_breeze_config ${XCB_LIBRARIES})
  endif()

  install(TARGETS kwin_breeze_config DESTINATION ${PLUGIN_INSTALL_DIR})

else()
  kconfig_add_kcfg_files(kwin_breeze_config_PART_SRCS ../breezeconfiguration.kcfgc)

  ki18n_wrap_ui(kwin_breeze_config_PART_FORMS_HEADERS ${kwin_breeze_config_PART_FORMS})
  add_library(kwin_breeze_config MODULE
      ${kwin_breeze_config_PART_SRCS}
      ${kwin_breeze_config_PART_FORMS_HEADERS})

  target_link_libraries(kwin_breeze_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
  target_link_libraries(kwin_breeze_config KF5::I18n KF5::WindowSystem KF5::ConfigCore KF5::ConfigWidgets)

  if(BREEZE_HAVE_X11)
    target_link_libraries(kwin_breeze_config XCB::XCB)
    target_link_libraries(kwin_breeze_config Qt5::X11Extras)
  endif()

  install(TARGETS kwin_breeze_config DESTINATION ${PLUGIN_INSTALL_DIR}/kwin/kdecorations/config)
endif()
