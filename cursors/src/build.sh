#!/bin/bash
set -euo pipefail

SVG_THEME_DIR="cursors_scalable"
SIZES=12,18,24,30,36,42,48,54,60,66,72

echo -ne "Checking Requirements...\\r"
if [[ ! -d "$SVG_THEME_DIR" ]]; then
	echo -e "\\nFAIL: ${SVG_THEME_DIR} missing"
	exit 1
fi

if ! command -v xcursorgen > /dev/null ; then
	echo -e "\\nFAIL: xcursorgen must be installed"
	exit 1
fi

if ! command -v kcursorgen > /dev/null ; then
	echo -e "\\nFAIL: kcursorgen must be built and in $PATH"
	exit 1
fi
echo -e "\033[0KChecking Requirements... DONE"

echo "Building XCursors..."
kcursorgen --svg-theme-to-xcursor --theme-dir . --xcursor-sizes $SIZES
echo "Building XCursors... DONE"

echo "COMPLETE!"
