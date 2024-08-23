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

There's no requirement on the canvas size of the SVG file. You must specify a corresponding
`nominal_size` for each SVG in the `metadata.json` file. See the next section for details.

## `metadata.json` format

The `metadata.json` file contains metadata for the cursor. The format is formally
defined as a [JSON Schema](https://json-schema.org/) in
[`svg-cursor-format.schema.json`](svg-cursor-format.schema.json).

For static cursor:

```js
[
{
    "filename": "default.svg", // String. The filename of the SVG file.
    "nominal_size": 24, // Number. The nominal size of the cursor.
    "hotspot_x": 3.5, // Number. The x-coordinate of the hotspot in the nominal size.
    "hotspot_y": 4.5 // Number. The y-coordinate of the hotspot in the nominal size.
}
]
```

For animated cursor:

```js
[
    { // frame 1
        "filename": "wait-01.svg", // String. The filename of the SVG file.
        "nominal_size": 24, // Number. The nominal size of the cursor.
        "hotspot_x": 3.5, // Number. The x-coordinate of the hotspot in the nominal size.
        "hotspot_y": 4.5, // Number. The y-coordinate of the hotspot in the nominal size.
        "delay": 30 // Number. The animation delay to the next frame in milliseconds.
    },
    { // frame 2
        "filename": "wait-02.svg", // String. The filename of the SVG file.
        ...

]
```

### Explanation of the nominal size

We kept the "nominal size" concept from the XCursor format. In XCursor, when the user sets the cursor size to 24 (nominal size), the actual pixmap doesn't have to be 24x24 pixels. The pixmap can be any size, and
not necessarily square. For example, in the Breeze cursor theme, while the default "arrow" pixmap for
the nominal size 24 does fit in a 24x24 rectangle, the "progress" pixmap (arrow with a rotating circle)
doesn't.

Also note that the hotspot coordinates are in the actual pixmap. So they can exceed the nominal size, too.

The same concept applies to the SVG format. Each SVG file should have a corresponding `nominal_size` in the `metadata.json` file. When the user sets a cursor size different than `nominal_size` ,then the SVG canvas
size and the hotspot coordinates should be scaled accordingly.

Example:

If the user sets the cursor size to SIZE, then when rendering the SVG to pixmap, the canvas size
should be:

```python
w = floor(svg.width * SIZE / metadata.nominal_size)
h = floor(svg.height * SIZE / metadata.nominal_size)
```

The hotspot should be:

```python
x = floor(metadata.hotspot_x * SIZE / metadata.nominal_size)
y = floor(metadata.hotspot_y * SIZE / metadata.nominal_size)
```
