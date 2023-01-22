################# Qt/KDE #################
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets DBus)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Config
    GuiAddons
    IconThemes
    ConfigWidgets
    WindowSystem)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Quick)
set(BREEZE_HAVE_QTQUICK ${Qt${QT_MAJOR_VERSION}Quick_FOUND})
if( BREEZE_HAVE_QTQUICK )
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2)
endif()

if (TARGET Qt5::Core)
    find_package(Qt5 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS X11Extras)
    set(BREEZE_HAVE_QTX11EXTRAS ${Qt5X11Extras_FOUND})
elseif(NOT WIN32)
    set(BREEZE_HAVE_QTX11EXTRAS TRUE) # part of Qt6::Gui
endif()

find_package( KF5FrameworkIntegration ${KF5_MIN_VERSION} CONFIG )
set_package_properties(KF5FrameworkIntegration PROPERTIES
    DESCRIPTION "KF5 Framework Integration"
    URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
    TYPE OPTIONAL
    PURPOSE "Required to use KStyle convenience functionalities in style")

set(BREEZE_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})


################# includes #################
include_directories(
  animations
  debug
)

include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# configuration #################
configure_file(config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

########### next target ###############
set(breeze_PART_SRCS
    animations/breezeanimation.cpp animations/breezeanimation.h
    animations/breezeanimations.cpp animations/breezeanimations.h
    animations/breezeanimationdata.cpp animations/breezeanimationdata.h
    animations/breezebaseengine.cpp animations/breezebaseengine.h
    animations/breezebusyindicatordata.cpp animations/breezebusyindicatordata.h
    animations/breezebusyindicatorengine.cpp animations/breezebusyindicatorengine.h
    animations/breezedialdata.cpp animations/breezedialdata.h
    animations/breezedialengine.cpp animations/breezedialengine.h
    animations/breezeenabledata.cpp animations/breezeenabledata.h
    animations/breezegenericdata.cpp animations/breezegenericdata.h
    animations/breezeheaderviewdata.cpp animations/breezeheaderviewdata.h
    animations/breezeheaderviewengine.cpp animations/breezeheaderviewengine.h
    animations/breezescrollbardata.cpp animations/breezescrollbardata.h
    animations/breezescrollbarengine.cpp animations/breezescrollbarengine.h
    animations/breezespinboxengine.cpp animations/breezespinboxengine.h
    animations/breezespinboxdata.cpp animations/breezespinboxdata.h
    animations/breezestackedwidgetdata.cpp animations/breezestackedwidgetdata.h
    animations/breezestackedwidgetengine.cpp animations/breezestackedwidgetengine.h
    animations/breezetabbarengine.cpp animations/breezetabbarengine.h
    animations/breezetabbardata.cpp animations/breezetabbardata.h
    animations/breezetoolboxengine.cpp animations/breezetoolboxengine.h
    animations/breezetransitiondata.cpp animations/breezetransitiondata.h
    animations/breezetransitionwidget.cpp animations/breezetransitionwidget.h
    animations/breezewidgetstateengine.cpp animations/breezewidgetstateengine.h
    animations/breezewidgetstatedata.cpp animations/breezewidgetstatedata.h
    debug/breezewidgetexplorer.cpp debug/breezewidgetexplorer.h
    breezeaddeventfilter.cpp breezeaddeventfilter.h
    breezeblurhelper.cpp breezeblurhelper.h
    breezeframeshadow.cpp breezeframeshadow.h
    breezehelper.cpp breezehelper.h
    breezemdiwindowshadow.cpp breezemdiwindowshadow.h
    breezemnemonics.cpp breezemnemonics.h
    breezepropertynames.cpp breezepropertynames.h
    breezeshadowhelper.cpp breezeshadowhelper.h
    breezesplitterproxy.cpp breezesplitterproxy.h
    breezestyle.cpp breezestyle.h
    breezestyleplugin.cpp breezestyleplugin.h
    breezetileset.cpp breezetileset.h
    breezewindowmanager.cpp breezewindowmanager.h
    breezetoolsareamanager.cpp breezetoolsareamanager.h
)

kconfig_add_kcfg_files(breeze_PART_SRCS ../kdecoration/breezesettings.kcfgc)
kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)
add_library(breeze MODULE ${breeze_PART_SRCS})
target_link_libraries(breeze Qt::Core Qt::Gui Qt::Widgets Qt::DBus)
if( BREEZE_HAVE_QTQUICK )
    target_link_libraries(breeze Qt::Quick KF5::CoreAddons KF5::Kirigami2)
endif()
if (TARGET Qt5::X11Extras)
    target_link_libraries(breeze Qt5::X11Extras)
elseif (TARGET Qt6::Gui)
    target_link_libraries(breeze Qt6::GuiPrivate)
endif()
target_link_libraries(breeze KF5::CoreAddons KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::IconThemes KF5::WindowSystem)
target_link_libraries(breeze breezecommon5)

if(KF5FrameworkIntegration_FOUND)
    target_link_libraries(breeze KF5::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(breeze PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()


########### install files ###############
install(TARGETS breeze DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles/)
install(FILES breeze.themerc  DESTINATION  ${KDE_INSTALL_DATADIR}/kstyle/themes)

########### subdirectories ###############
add_subdirectory(config)
