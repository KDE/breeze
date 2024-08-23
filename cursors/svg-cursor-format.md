# SVG cursor format specification

## Directory structure

SVG cursors and their metadata are stored in the `cursors_scalable` directory
in the cursor theme directory, side-by-side with the `cursors` directory for
XCursor format. The directory structure is as follows:

```plain
breeze_cursors/
├── cursors (XCursor format)
├── cursors_scalable (SVG format)
│   ├── default
│   │   ├── default.svg
│   │   └── metadata.json
│   ├── wait
│   │   ├── wait-01.svg
│   │   ├── wait-02.svg
│   │   ├── wait-03.svg
│   │   └── metadata.json
│   └── watch (symlink to `wait`)
└── index.theme
```

Each cursor is in its own sub-directory, named after the cursor name. The sub-directory
contains one `metadata.json` file and one SVG file (or multiple SVG files for an animated cursor).

Aliases are symlinks to the original cursor directory.

## SVG format

Each SVG file should be conforming to [SVG Tiny 1.2](https://www.w3.org/TR/SVGTiny12/).
The "height" and "weight" attributes of the root `<svg>` element must be defined to the
canvas size when the nominal cursor size is 24. (The canvas size can exceed 24x24, though.)

## `metadata.json` format

The `metadata.json` file contains metadata for the cursor. The format is as follows:

For static cursor:

```js
[
{
    "filename": "default.svg", // String. The filename of the SVG file.
    "hotspot_x": 3.5, // Number. The x-coordinate of the hotspot when nominal size is 24.
    "hotspot_y": 4.5 // Number. The y-coordinate of the hotspot when nominal size is 24.
}
]
```

For animated cursor:

```js
[
    { // frame 1
        "filename": "wait-01.svg", // String. The filename of the SVG file.
        "hotspot_x": 3.5, // Number. The x-coordinate of the hotspot when nominal size is 24.
        "hotspot_y": 4.5, // Number. The y-coordinate of the hotspot when nominal size is 24.
        "frametime": 30 // Number. The duration of the frame in milliseconds.
    },
    { // frame 2
        "filename": "wait-02.svg", // String. The filename of the SVG file.
        ...

]
```

## Rendering the SVG

When rendering the SVG to pixmap for a nominal cursor size in SIZE, the canvas size should be:

```python
w = floor(svg.width * SIZE / 24)
h = floor(svg.height * SIZE / 24)
```

The hotspot should be:

```python
x = floor(hotspot_x * SIZE / 24)
y = floor(hotspot_y * SIZE / 24)
```

However, to prevent rounding errors, it's recommended to offset the hotspot a little bit to the bottom-right:

```python
x = floor(hotspot_x * SIZE / 24 + 0.01)
y = floor(hotspot_y * SIZE / 24 + 0.01)
```
