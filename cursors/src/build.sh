#!/bin/bash
set -euo pipefail

SRC_DIR="src"
RAWSVG_DIR="$SRC_DIR/svg"
INDEX="$SRC_DIR/index.theme"
ALIASES="$SRC_DIR/alias.list"

FRAME_TIME=30
SIZES=12,18,24,30,36,42,48,54,60,66,72

echo -ne "Checking Requirements...\\r"
if [[ ! -d "${RAWSVG_DIR}" ]]; then
	echo -e "\\nFAIL: ${RAWSVG_DIR} missing"
	exit 1
fi

if [[ ! -f "${INDEX}" ]]; then
	echo -e "\\nFAIL: ${INDEX} missing"
	exit 1
fi

if [[ ! -f "${ALIASES}" ]]; then
	echo -e "\\nFAIL: ${ALIASES} missing"
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

echo -ne "Making Folders...\\r"
OUTPUT="$(grep --only-matching --perl-regex "(?<=Name\=).*$" $INDEX)"
OUTPUT=${OUTPUT// /_}
rm -rf "$OUTPUT"
mkdir -p "$OUTPUT"
echo -e "\033[0KMaking Folders... DONE"

echo -ne "Copying Theme Index...\\r"
	cp $INDEX "$OUTPUT/index.theme"
echo -e "\033[0KCopying Theme Index... DONE"

echo "Building SVG cursors..."
kcursorgen --build-svg-theme --theme-dir "$OUTPUT" --svg-src-dir "$RAWSVG_DIR" --alias-file "$ALIASES" --frametime $FRAME_TIME --sizes $SIZES
echo "Building SVG cursors... DONE"

echo "Building XCursors..."
kcursorgen --svg-theme-to-xcursor --theme-dir "$OUTPUT"
echo "Building XCursors... DONE"

echo "COMPLETE!"
