include(ECMMarkNonGuiExecutable)

set(CMAKE_AUTOMOC ON)

add_executable(kcursorgen)

ecm_mark_nongui_executable(kcursorgen)

target_sources(kcursorgen PRIVATE
    main.cpp
    kcursorgen.cpp
)
target_link_libraries(kcursorgen Qt6::Core Qt6::Svg)

install(TARGETS kcursorgen)
