# Breeze Cursors

## Editing the SVGs

SVG cursors are individual ".svg" files in the "{Breeze,Breeze_Light}/src/svg" directory.

The canvas size should correspond to a nominal size (reported to apps, and shown in the
cursor KCM) of 24.

Each SVG must contain an invisible rectangle with the id `hotspot`. The top-left
corner of the rectangle defines the hotspot of the cursor. It doesn't have to be
at integer coordinates. When exporting the cursor to pixmap format, the hotspot
will be rounded to the floor.

## Building the Cursors

Because building cursors requires `Inkscape`, it's not integrated into the normal `cmake`
build. After making changes to the SVGs, you should manually build the cursor theme:

1. Ensure you have `Inkscape` and `xcursorgen` installed.
2. "cd" into the "Breeze" and "Breeze_Light" directory and run "../src/build.sh" script.
   The script will first use `Inkscape` to render SVGs into PNGs in the "build" directory,
   then use `xcursorgen` to generate the X11 cursor theme in the "Breeze/Breeze" or
   "Breeze_Light/Breeze_Light" directory.
3. Commit changes in the theme directories to git.

## SVG cursor format

In addition to the standard XCursor format in "cursors" directory, the original SVGs are also
deployed in the "cursors_scalable" directory. See "cursors_scalable.md" for the spec.
