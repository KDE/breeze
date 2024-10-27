include(ECMMarkNonGuiExecutable)
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core Svg)

set(CMAKE_AUTOMOC ON)

add_executable(kcursorgen)

ecm_mark_nongui_executable(kcursorgen)

target_sources(kcursorgen PRIVATE
    main.cpp
    kcursorgen.cpp
)
target_link_libraries(kcursorgen Qt6::Core Qt6::Svg)

install(TARGETS kcursorgen)
