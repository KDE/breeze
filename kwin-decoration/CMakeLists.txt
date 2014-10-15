project(kwin-decoration-breeze)

set(BREEZE_USE_KDE4 ${USE_KDE4})
if(BREEZE_USE_KDE4)

  ############ Language and toolchain features
  ############ copied from ECM
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND NOT WIN32)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()

endif()

################# Qt/KDE #################
if(BREEZE_USE_KDE4)

  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)
  include(MacroLibrary)

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
  include_directories(${KDE4_INCLUDES})

  ### KDE workspace (needed for window decoration)
  find_package(KDE4Workspace 4.9.80)
  set_package_properties(
    KDE4Workspace PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required to oxygen window decoration for kwin"
  )

  ### XCB
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(XCB xcb x11-xcb)
  set(BREEZE_HAVE_X11 ${XCB_FOUND})

else()

  find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)

  find_package(KF5 REQUIRED COMPONENTS
      GuiAddons
      I18n
      ConfigWidgets
      Service
      FrameworkIntegration
      Completion
  )

  find_package(XCB COMPONENTS XCB)
  set_package_properties(XCB PROPERTIES
      DESCRIPTION "X protocol C-language Binding"
      URL "http://xcb.freedesktop.org"
      TYPE OPTIONAL
      PURPOSE "Required to pass style properties to native Windows on X11 Platform"
  )

  find_package(KDecorations CONFIG)
  set_package_properties(
    KDecorations PROPERTIES
    TYPE REQUIRED
 )

  if(NOT APPLE)

    set(BREEZE_HAVE_X11 ${XCB_XCB_FOUND})
    if (XCB_XCB_FOUND)
      find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
    endif()

  endif()


endif()

########### add version number into compilation defines
add_definitions(-DTRANSLATION_DOMAIN=\"breeze_kwin_deco\")

################# configuration #################
configure_file(config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

add_subdirectory(config)

########### next target ###############
set(kwin_breeze_SRCS
     breezeanimation.cpp
     breezebutton.cpp
     breezeclient.cpp
     breezeclientgroupitemdata.cpp
     breezeexceptionlist.cpp
     breezefactory.cpp
     breezehelper.cpp
     breezesizegrip.cpp
     breezetileset.cpp
)

if(BREEZE_USE_KDE4)

  kde4_add_kcfg_files(kwin_breeze_SRCS breezeconfiguration.kcfgc)
  kde4_add_plugin(kwin3_breeze ${kwin_breeze_SRCS})
  target_link_libraries(
      kwin3_breeze
      ${KDE4_KDEUI_LIBS}
      ${KDE4WORKSPACE_KDECORATIONS_LIBS}
  )

  if(BREEZE_HAVE_X11)
    target_link_libraries(kwin3_breeze ${X11_XCB_LIBRARIES})
    target_link_libraries(kwin3_breeze ${XCB_LIBRARIES})
  endif()

  install(TARGETS kwin3_breeze DESTINATION ${PLUGIN_INSTALL_DIR})
  install(FILES breezeclient.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kwin/)

else()

  kconfig_add_kcfg_files(kwin_breeze_SRCS breezeconfiguration.kcfgc)

  add_library(kwin3_breeze MODULE ${kwin_breeze_SRCS})
  target_link_libraries(kwin3_breeze Qt5::Widgets)
  target_link_libraries(kwin3_breeze
     KF5::GuiAddons
     KF5::I18n
     KF5::Style
     KF5::Service
     KF5::ConfigWidgets
  )

  target_link_libraries(kwin3_breeze kdecorations)

  kservice_desktop_to_json(kwin3_breeze breezeclient.desktop)

  if(BREEZE_HAVE_X11)
    target_link_libraries(kwin3_breeze XCB::XCB Qt5::X11Extras)
  endif()

  install(TARGETS kwin3_breeze  DESTINATION ${PLUGIN_INSTALL_DIR}/kwin/kdecorations)

endif()
