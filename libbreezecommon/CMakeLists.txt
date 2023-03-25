################# dependencies #################
### Qt/KDE
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets)

################# breezestyle target #################
set(breezecommon_LIB_SRCS
    breezeboxshadowrenderer.cpp
)

add_library(breezecommon ${breezecommon_LIB_SRCS})

generate_export_header(breezecommon
    BASE_NAME breezecommon
    EXPORT_FILE_NAME breezecommon_export.h)

target_link_libraries(breezecommon
    PUBLIC
        Qt::Core
        Qt::Gui)

set_target_properties(breezecommon PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME breezecommon${QT_MAJOR_VERSION})

install(TARGETS breezecommon ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
