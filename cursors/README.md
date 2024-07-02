# Breeze Cursors

## Editing the SVGs

SVG cursors are individual ".svg" files in the "src/svg" directory.

The canvas size should be 32x32. 

Each SVG must contain an invisible rectangle with the id `hotspot`. The top-left
corner of the rectangle defines the hotspot of the cursor. It doesn't have to be
at integer coordinates. When exporting the cursor to pixmap format, the hotspot
will be rounded to the floor.

## Building the Cursors

1. Ensure you have inkscape and xcursorgen installed.
2. "cd" into the "Breeze" directory and run "../src/build.sh" script.

The script will first generate PNGs in the "build" directory, and then use xcursorgen
to generate the X11 cursor theme in the "Breeze" directory.
