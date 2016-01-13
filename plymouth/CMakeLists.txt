find_package(Plymouth)

if(PLYMOUTH_THEME_DIR)
  install(DIRECTORY breeze DESTINATION ${PLYMOUTH_THEME_DIR})
  install(CODE "MESSAGE(\"Remember to set default.plymouth to breeze and run update-initramfs -u.\")")
endif()
