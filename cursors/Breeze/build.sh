#!/bin/bash
set -euo pipefail

# Open initial output.
# Prefer konsole if its there, otherwise fall back to xterminal.
#tty -s; if [ $? -ne 0 ]; then
#	if command -v konsole &>/dev/null; then
#		konsole -e "$0"; exit;
#		else
#		xterm -e "$0"; exit;
#	fi
#fi

cd "$( dirname "${BASH_SOURCE[0]}" )"
RAWSVG="src/cursors.svg"
INDEX="src/index.theme"
ALIASES="src/cursorList"


echo -ne "Checking Requirements...\\r"
if [[ ! -f "${RAWSVG}" ]]; then
	echo -e "\\nFAIL: '${RAWSVG}' missing in /src"
	exit 1
fi

if [[ ! -f "${INDEX}" ]]; then
	echo -e "\\nFAIL: '${INDEX}' missing in /src"
	exit 1
fi

if ! command -v inkscape > /dev/null ; then
	echo -e "\\nFAIL: inkscape must be installed"
	exit 1
fi

if ! command -v xcursorgen > /dev/null ; then
	echo -e "\\nFAIL: xcursorgen must be installed"
	exit 1
fi
echo -e "Checking Requirements... DONE"

SCALES="50 75 100 125 150 175 200 225 250 275 300"

echo -ne "Making Folders... \\r"
OUTPUT="$(grep --only-matching --perl-regex "(?<=Name\=).*$" $INDEX)"
OUTPUT=${OUTPUT// /_}
for scale in $SCALES; do
	mkdir -p "build/x$scale"
done
mkdir -p "$OUTPUT/cursors"
echo 'Making Folders... DONE';

echo -ne "Generating simple cursor pixmaps...\\r"
genPixmaps="file-open:${RAWSVG};"
for CUR in src/config/*.cursor; do
	BASENAME=${CUR##*/}
	BASENAME=${BASENAME%.*}


	for scale in $SCALES; do
		DIR="build/x${scale}"
		if [[ "${DIR}/${BASENAME}.png" -ot ${RAWSVG} ]]; then
			genPixmaps="${genPixmaps} export-id:${BASENAME}; export-width:$((32*scale/100)); export-height:$((32*scale/100)); export-filename:${DIR}/${BASENAME}.png; export-do;"
		fi
	done
done
inkscape --shell < <(echo "${genPixmaps}") > /dev/null
echo -e "\033[0KGenerating simple cursor pixmaps... DONE"

echo -ne "\033[0KGenerating animated cursor pixmaps... \\r"
genPixmaps="file-open:${RAWSVG};"
for i in {01..23}; do
	for scale in $SCALES; do
		DIR="build/x$scale"

		if [[ "$DIR/progress-$i.png" -ot $RAWSVG ]]; then
			genPixmaps="${genPixmaps} export-id:progress-${i}; export-width:$((32*scale/100)); export-height:$((32*scale/100)); export-filename:${DIR}/progress-${i}.png; export-do;"
		fi

		if [[ "$DIR/wait-$i.png" -ot $RAWSVG ]]; then
			genPixmaps="${genPixmaps} export-id:wait-${i}; export-width:$((32*scale/100)); export-height:$((32*scale/100)); export-filename:${DIR}/wait-${i}.png; export-do;"
		fi
	done
done
inkscape --shell < <(echo "${genPixmaps}") > /dev/null
echo -e "\033[0KGenerating animated cursor pixmaps... DONE"


echo -ne "Generating cursor theme...\\r"
for CUR in src/config/*.cursor; do
	BASENAME=${CUR##*/}
	BASENAME=${BASENAME%.*}

	TMP_CUR="tmp.cursor"
	./scale_cursor "${CUR}" ${SCALES} > $TMP_CUR
	if ! ERR="$( xcursorgen -p build "$TMP_CUR" "$OUTPUT/cursors/$BASENAME" 2>&1 )"; then
		echo "FAIL: $CUR $ERR"
		rm -f "${TMP_CUR}"
	fi
	rm -f "${TMP_CUR}"
done
echo -e "Generating cursor theme... DONE"


echo -ne "Generating shortcuts...\\r"
while read ALIAS ; do
	FROM=${ALIAS% *}
	TO=${ALIAS#* }

	if [[ -e "$OUTPUT/cursors/$FROM" ]]; then
		continue
	fi

	ln -s "$TO" "$OUTPUT/cursors/$FROM"
done < $ALIASES
echo -e "\033[0KGenerating shortcuts... DONE"

echo -ne "Copying Theme Index...\\r"
	if ! [[ -e "$OUTPUT/$INDEX" ]]; then
		cp $INDEX "$OUTPUT/index.theme"
	fi
echo -e "\033[0KCopying Theme Index... DONE"

echo "COMPLETE!"
