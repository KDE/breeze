# SVG cursor specification

## Directory layout

SVG cursors are individual ".svg" files, stored in a "cursors_scalable" directory side-by-side
 with the Xcursor "cursors" directory.

An example directory layout:

- Breeze
  - cursors
    - default (Xcursor file)
    - left_ptr -> default (symlink)
    - ...
  - cursors_scalable
    - default.svg (SVG cursor file)
    - left_ptr.svg -> default.svg (symlink)
    - ...
  - index.theme

## SVG cursor file format

An SVG cursor file is a [SVG Tiny 1.2](https://www.w3.org/TR/SVGTiny12/) file with the following additional requirements:

### Nominal size

SVG cursors are assumed to have a "nominal size" of 24. So if the user sets a cursor size of 48,
the SVG cursor should be rendered at 2x scale.

### Hotspot

The cursor hotspot is embedded in the SVG, as an invisible `<rect>` element with `id="hotspot"`
and `style="display:none"`. The top-left corner of the `<rect>` defines the hotspot.

Example - querying hotspot in Qt:

```cpp
svg = QSvgRenderer("default.svg")
hotspot = svg.transformForElement('hotspot').map(svg.boundsOnElement('hotspot')).boundingRect().topLeft()
```

Note: In many cases, the hotspot is put in the center of a line in the cursor. So its coordinates
 can be in the center of a pixel, not necessarily integers.

### Animation

Animation is not done through frames as in Xcursor, but through [SVG Animation](https://www.w3.org/TR/SVGTiny12/animate.html).

Usually, the animation should have `repeatCount="indefinite"` so it loops forever.
