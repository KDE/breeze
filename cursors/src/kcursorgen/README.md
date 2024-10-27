# kcursorgen: Convert SVG theme to XCursor theme

`kcursorgen` is a tool to convert SVG theme to XCursor theme. Traditionally, Breeze (and plenty of other themes) SVG files are rendered with Inkscape and packaged as XCursor files at build time, then the resulting XCursor files are shipped to the end user. `kcursorgen` only depends on `QtSvg` and `xcursorgen`, making it possible to only ship SVG files and this program to the end user, and generate XCursor files at install time (in the distro's postinstall script) or runtime (during Plasma startup and in the cursor theme KCM).

The benefits of this approach are:

1. Smaller package size: Breeze SVG files are less than 1MB, while the XCursor files are around 15MB.
2. Allow the user to set any cursor size and scale factor, instead of being limited to the sizes provided by the theme.

## Usage

```sh
kcursorgen --svg-theme-to-xcursor --svg-dir <path> --xcursor-dir <path> --sizes <size1,size2,...> --scales <scale1,scale2,...>
```

- `--svg-dir` is the path to the SVG theme directory, e.g. `/usr/share/icons/breeze_cursors/cursors_scalable`.
- `--xcursor-dir` is the path to the XCursor theme directory, e.g. `/usr/share/icons/breeze_cursors/cursors`. You need to clear this directory before running `kcursorgen`.
- `--sizes` is a comma-separated list of sizes to generate, e.g. `16,24,32,48,64`.
- `--scales` is a comma-separated list of scales to generate, e.g. `1,1.25,1.5,1.75,2`. The scale factor is multiplied by the size to get the final list of cursor sizes. `kcursorgen` will take care of alignment requirements at integer scales. (E.g. GTK3 requires the cursor image size to be a multiple of 3 at scale 3.)

Note that while a postinstall script can write to locations like `/usr/share/icons/breeze_cursors/`, a normal user cannot. So in the Plasma startup code, the cursor theme KCM, or manually running `kcursorgen`, you should copy the cursor theme to `~/.local/share/icons/` and run the tool there.

For example, if the user sets a cursor size of 50 and a scale factor of 2.5, we can copy `/usr/share/icons/breeze_cursors/` to `~/.local/share/icons/` and run:

```sh
kcursorgen --svg-theme-to-xcursor --svg-dir ~/.local/share/icons/breeze_cursors/cursors_scalable --xcursor-dir ~/.local/share/icons/breeze_cursors/cursors --sizes 50 --scales 1,2.5,3
```

In the `--scales` argument, "1" provides an unscaled cursor for Wayland apps without HiDPI support and X11 apps running in "scaled by system" mode, "2.5" provides a cursor for Wayland apps with fractional scaling support and X11 apps in "scale themselves" mode, and "3" provides a cursor for Wayland apps with only integer scaling support.
