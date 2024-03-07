#!/usr/bin/python3

from PySide6.QtWidgets import QApplication, QDialog, QHBoxLayout
from PySide6.QtSvgWidgets import QSvgWidget

class MainWindow(QDialog):

    def __init__(self):
        super(MainWindow, self).__init__()

        layout = QHBoxLayout(self)

        svg1 = QSvgWidget('build/scalable/wait.svg')
        svg1.renderer().setFramesPerSecond(30)
        layout.addWidget(svg1)

        svg2 = QSvgWidget('build/scalable/progress.svg')
        svg2.renderer().setFramesPerSecond(90)
        layout.addWidget(svg2)


app = QApplication([])
window = MainWindow()
window.show()
app.exec()