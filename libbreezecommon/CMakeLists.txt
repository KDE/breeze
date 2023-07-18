add_library(breezecommon OBJECT breezeboxshadowrenderer.cpp)

set_target_properties(breezecommon PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(breezecommon
    PUBLIC
        Qt::Core
        Qt::Gui)
