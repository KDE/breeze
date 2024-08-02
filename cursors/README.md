# Breeze Cursors

## Editing the SVGs

SVG cursors are individual ".svg" files in the "Breeze/Breeze/cursors_scalable"
directory (or corresponding "Breeze_Light" directory).

The size of the cursor should correspond to a nominal size (reported to apps, and shown
in the cursor KCM) of 24. The canvas size doesn't need to be 24x24. For a cursor with
canvas size WxH, when the user selects a nominal size of S, the cursor will be scaled
to size W*S/24 x H*S/24.

## SVG cursor format

The "cursors_scalable" directory contains both SVG files and their metadata ("metadata.json" files).
Edit "metadata.json" to modify hotspots or animation frame lists.

See "svg-cursor-format.md" for the spec.

## Updating XCursors

After making changes to the SVGs, you should manually update XCursors:

1. Ensure you have `xcursorgen` and `kcursorgen` installed.
2. "cd" into the "Breeze/Breeze" (or "Breeze_Light/Breeze_Light") directory
   and run "../../src/build.sh" script. The script will update XCursor files
   in the "cursors" directory.
3. Commit changed files to git.
