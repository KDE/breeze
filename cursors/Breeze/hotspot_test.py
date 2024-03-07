#!/usr/bin/python3

from PySide6.QtSvg import QSvgRenderer
import sys, math

svg = QSvgRenderer(sys.argv[1])
hotspot = svg.transformForElement('hotspot').map(svg.boundsOnElement('hotspot')).boundingRect().topLeft()
print(hotspot.x(), hotspot.y())
print(math.floor(hotspot.x()), math.floor(hotspot.y()))