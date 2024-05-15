########### next target ###############
set(breeze_test_SOURCES
    breezebenchmarkwidget.cpp
    breezebuttondemowidget.cpp
    breezedemodialog.cpp
    breezedemowidget.cpp
    breezeframedemowidget.cpp
    breezeinputdemowidget.cpp
    breezelistdemowidget.cpp
    breezemdidemowidget.cpp
    breezesimulator.cpp
    breezesliderdemowidget.cpp
    breezetabdemowidget.cpp
    breezetabwidget.cpp
    main.cpp)

set(breeze_test_FORMS
    ui/breezebenchmarkwidget.ui
    ui/breezebuttondemowidget.ui
    ui/breezeframedemowidget.ui
    ui/breezeinputdemowidget.ui
    ui/breezelistdemowidget.ui
    ui/breezemdidemowidget.ui
    ui/breezesliderdemowidget.ui
    ui/breezetabdemowidget.ui
)

find_package(KF${QT_MAJOR_VERSION}ConfigWidgets CONFIG)
if(KF${QT_MAJOR_VERSION}ConfigWidgets_FOUND)
    set(breeze_test_SOURCES ${breeze_test_SOURCES} breezeschemechooser.cpp breezestylechooser.cpp)
    add_definitions(-DHAVE_SCHEME_CHOOSER)
endif()

ki18n_wrap_ui(breeze_test_FORMS_HEADERS ${breeze_test_FORMS})

add_executable(breeze-test${QT_MAJOR_VERSION}
    ${breeze_test_SOURCES}
    ${breeze_test_FORMS_HEADERS}
)

target_include_directories(breeze-test${QT_MAJOR_VERSION} PRIVATE ${CMAKE_BINARY_DIR}/libbreeze${QT_MAJOR_VERSION})

target_link_libraries(breeze-test${QT_MAJOR_VERSION} Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Widgets)
target_link_libraries(breeze-test${QT_MAJOR_VERSION} KF${QT_MAJOR_VERSION}::Completion KF${QT_MAJOR_VERSION}::ConfigGui KF${QT_MAJOR_VERSION}::WidgetsAddons KF${QT_MAJOR_VERSION}::I18n)

if(KF${QT_MAJOR_VERSION}ConfigWidgets_FOUND)
    target_link_libraries(breeze-test${QT_MAJOR_VERSION} KF${QT_MAJOR_VERSION}::ConfigWidgets)
endif()

########### install files ###############
install(TARGETS breeze-test${QT_MAJOR_VERSION} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
