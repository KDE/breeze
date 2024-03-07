# How SVG cursors are built in Breeze

The source file of Breeze cursors is a single `src/cursors.svg` file, created in Inkscape.
So we need to extract individual cursors from this file and save them as separate `.svg` files.

The process is as follows:

## Preparing the source file

`src/cursors.svg` is modified in Inkscape:

1. An invisible rectangle labeled "hotspot" is added
   to each cursor, whose top-left corner defines the cursor's hotspot.

1. Ensure that all visible parts of the cursor is in a group labeled "<cursor-name>" in the
   "Cursors" layer. And they are inside the box with "id=<cursor-name>" in the "Web Slicer" layer.

These are required by the building process. When you add a new cursor, you should do the same.

## Building

Building is done by the `build.sh` script. Whenever `src/cursors.svg` is modified, it must be
re-run.

The script is not part of the KDE build system, because it requires Inkscape and we don't want
to make it a hard dependency for all developers. Instead, its build result is committed to the
repository.

The script does the following for each SVG cursor:

1. Run `inkscape src/cursors.svg -i <cursor-name> -o tmp1.svg` to export the cursor from the
source file. This is similar to what we export PNG for Xcursor. But for SVG export, Inkscape only
modifies the view box, not removing anything outside of it. So the exported file is as large as
the source file, and we need following steps to make it smaller.

1. Run `clean_svg tmp1.svg tmp2.svg <cursor-name>`. This script does the following:
    1. Remove all `<g>` (group) elements except the one labeled "<cursor-name>". This should purge
       all other cursors from the file.
    1. Inside the group labeled `<cursor-name>`, find the element labeled "hotspot" and change its
       `id` to `hotspot`. We can't do that in the source file, because Inkscape doesn't
       allow duplicate `id="hotspot"` elements for different cursors. Build process will fail if
       the hotspot element is not found.
1. Run `inkscape` again to remove intermediate "Cursors" and "Row" groups, moving the cursor to
   the root, and export as a "plain SVG" file without Inkscape-specific metadata.

The resulting file still contains some unused defs. But it seems non-trivial to remove them.

### Animated cursor

`progress.svg` and `wait.svg` contain "<AnimatedTrasform>" elements. These are currently manually
added into these files. So the build script just copies them from the "src/" dir, instead of
extracting from `src/cursors.svg`. So, if you modify these cursors in `src/cursors.svg`, the build
result won't reflect your changes.

If you modify them or add a new animated cursor, you have to run `build.sh` once, then copy the
extracted svg file from `build/scalable` to `src/`, then manually add the "<AnimatedTrasform>"
element. Then run `build.sh` again to update the build result.
