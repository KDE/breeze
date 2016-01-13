find_package(Plymouth)

if(PLYMOUTH_THEME_DIR)
  file(READ breeze/breeze.plymouth.in CONFIGFILE)
  string(REPLACE "PLYMOUTH_THEME_DIR" ${PLYMOUTH_THEME_DIR} CONFIGFILE ${CONFIGFILE})
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/breeze.plymouth ${CONFIGFILE})

  install(DIRECTORY breeze DESTINATION ${PLYMOUTH_THEME_DIR})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/breeze.plymouth DESTINATION ${PLYMOUTH_THEME_DIR}/breeze/)
  install(CODE "MESSAGE(\"Remember to set default.plymouth to breeze and run update-initramfs -u.\")")
endif()
