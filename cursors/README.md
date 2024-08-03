# Breeze Cursors

## Editing the SVGs

SVG cursors are individual ".svg" files in the "{Breeze,Breeze_Light}/src/svg" directory.

The size of the cursor should correspond to a nominal size (reported to apps, and shown
in the cursor KCM) of 24. The canvas size doesn't need to be 24x24. For a cursor with
canvas size WxH, when the user selects a nominal size of S, the cursor will be scaled
to size W*S/24 x H*S/24.

Each SVG must contain an invisible rectangle with the id `hotspot`. The top-left
corner of the rectangle defines the hotspot of the cursor. It doesn't have to be
at integer coordinates. When exporting the cursor to pixmap format, the hotspot
coordinates will be rounded to the floor.

## Building the Cursors

Running `cmake` doesn't build cursors. After making changes to the SVGs, you should manually
build the cursor theme and commit the result to git:

1. Ensure you have `xcursorgen` and `kcursorgen` installed.
2. "cd" into the "Breeze" and "Breeze_Light" directory and run "../src/build.sh" script.
   The script will generate SVG and XCursor themes in the "Breeze/Breeze" or
   "Breeze_Light/Breeze_Light" directory.
3. Commit changes in the theme directories to git.

## SVG cursor format

In addition to the standard XCursor format in "cursors" directory, the original SVGs are also
deployed in the "cursors_scalable" directory. See "cursors_scalable.md" for the spec.
