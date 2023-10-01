#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

# Get Qt version from CI
KDE_VERSION: Final = 5 if "qt515" in os.environ.get("CI_JOB_IMAGE", "") else 6


class KCMCrossTest(unittest.TestCase):
    """
    Tests the Breeze {cursor,decoration,kstyle} theme is installed to the correct place.
    """

    driver: webdriver.Remote | None = None
    options: AppiumOptions

    @classmethod
    def setUpClass(cls) -> None:
        cls.options = AppiumOptions()
        cls.options.set_capability("timeouts", {'implicit': 10000})
        # From XCURSORPATH
        icon_data_dirs: str = ":".join([f"{dir}/icons" for dir in os.environ['XDG_DATA_DIRS'].split(":")])
        cls.options.set_capability("environ", {
            "XCURSOR_PATH": f"{icon_data_dirs}:~/.icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons",
        })

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_#{self.id()}.png")
        self.driver.quit()
        self.driver = None

    @classmethod
    def tearDownClass(cls) -> None:
        pass

    def launch_kcm(self, module_id: str) -> None:
        """
        Helper function to start the specified KCM
        """
        if self.driver is not None:
            self.driver.quit()

        options = self.options
        options.set_capability("app", f"kcmshell{KDE_VERSION} {module_id}")

        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def test_1_kcm_cursortheme(self) -> None:
        """
        Cursor
        """
        self.launch_kcm("kcm_cursortheme")
        self.driver.find_element(AppiumBy.NAME, "Breeze")
        self.driver.find_element(AppiumBy.NAME, "Breeze Light")

    def test_2_kcm_kwindecoration(self) -> None:
        """
        Decoration
        """
        self.launch_kcm("kcm_kwindecoration")
        self.driver.find_element(AppiumBy.NAME, "Breeze")

    def test_3_kcm_style(self) -> None:
        """
        KStyle
        """
        self.launch_kcm("kcm_style")
        self.driver.find_element(AppiumBy.NAME, "Fusion")
        self.driver.find_element(AppiumBy.NAME, "Breeze")


if __name__ == '__main__':
    unittest.main()
