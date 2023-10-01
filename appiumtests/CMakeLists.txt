# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

add_test(
    NAME kcmtest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcmtest.py
)
set_tests_properties(kcmtest PROPERTIES TIMEOUT 120)
