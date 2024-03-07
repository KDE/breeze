#!/usr/bin/env python3

import sys

from PySide6.QtSvg import QSvgRenderer
from PySide6.QtGui import QGuiApplication, QPainter, QPixmap, QColor

app = QGuiApplication()

svg = QSvgRenderer(sys.argv[1])
size = svg.defaultSize()
image = QPixmap(size)
image.fill(QColor(0, 0, 0, 0))
painter = QPainter(image)
svg.render(painter)
painter.end()
image.save(sys.argv[2])
