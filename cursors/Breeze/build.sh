#!/bin/bash
# Open initial output.
# Prefer konsole if its there, otherwise fall back to xterminal.
#tty -s; if [ $? -ne 0 ]; then
#	if command -v konsole &>/dev/null; then
#		konsole -e "$0"; exit;
#		else
#		xterm -e "$0"; exit;
#	fi
#fi

set -e

cd "$( dirname "${BASH_SOURCE[0]}" )"
RAWSVG="src/cursors.svg"
INDEX="src/index.theme"
ALIASES="src/cursorList"


echo -ne "Checking Requirements...\\r"
if [ ! -f $RAWSVG ] ; then
	echo -e "\\nFAIL: '$RAWSVG' missing in /src"
	exit 1
fi

if [ ! -f $INDEX ] ; then
	echo -e "\\nFAIL: '$INDEX' missing in /src"
	exit 1
fi

if  ! type "inkscape" > /dev/null ; then
	echo -e "\\nFAIL: inkscape must be installed"
	exit 1
fi

if  ! type "xcursorgen" > /dev/null ; then
	echo -e "\\nFAIL: xcursorgen must be installed"
	exit 1
fi
echo -e "Checking Requirements... DONE"

SCALES="50 75 100 125 150 175 200 225 250 275 300"

echo -ne "Making Folders... $BASENAME\\r"
OUTPUT="$(grep --only-matching --perl-regex "(?<=Name\=).*$" $INDEX)"
OUTPUT=${OUTPUT// /_}
for scale in $SCALES; do
	mkdir -p "build/x$scale"
done
mkdir -p "build/scalable"
mkdir -p "$OUTPUT/cursors"
mkdir -p "$OUTPUT/cursors_scalable"
echo 'Making Folders... DONE';



for CUR in src/config/*.cursor; do
	BASENAME=$CUR
	BASENAME=${BASENAME##*/}
	BASENAME=${BASENAME%.*}
	
	echo -ne "\033[0KGenerating SVG... $BASENAME\\r"	
	
	DIR="build/scalable"
	if [ "$DIR/$BASENAME.svg" -ot $RAWSVG ] ; then
		# Set viewbox around the cursor
		inkscape $RAWSVG -i $BASENAME -o /tmp/"$BASENAME".svg > /dev/null
		# Remove everything except the cursor
		./clean_svg /tmp/"$BASENAME".svg /tmp/"$BASENAME"-cleaned.svg "$BASENAME"
		# Remove groups in the middle, and remove Inkscape-specific attributes
		inkscape /tmp/"$BASENAME"-cleaned.svg -o $DIR/"$BASENAME".svg --actions 'select-all:groups;selection-ungroup;select-all:groups;selection-ungroup' -l > /dev/null
		# TODO: Further clean up the SVG, remove unused defs.

		rm /tmp/"$BASENAME".svg /tmp/"$BASENAME"-cleaned.svg
	fi
done
echo -e "\033[0KGenerating SVG... DONE"



for CUR in src/config/*.cursor; do
	BASENAME=$CUR
	BASENAME=${BASENAME##*/}
	BASENAME=${BASENAME%.*}

	echo -ne "\033[0KGenerating simple cursor pixmaps... $BASENAME\\r"

	for scale in $SCALES; do
		DIR="build/x$scale"
		if [ "$DIR/$BASENAME.png" -ot $RAWSVG ] ; then
			inkscape $RAWSVG -i $BASENAME -w $((32*$scale/100)) -h $((32*$scale/100)) -o "$DIR/$BASENAME.png" > /dev/null
		fi
	done
done
echo -e "\033[0KGenerating simple cursor pixmaps... DONE"



for i in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23
do
	echo -ne "\033[0KGenerating animated cursor pixmaps... $i / 23 \\r"

	for CUR in progress wait
	do
		BASENAME="$CUR-$i"
		for scale in $SCALES; do
			DIR="build/x$scale"
			
			if [ "$DIR/$BASENAME.png" -ot $RAWSVG ] ; then
				inkscape $RAWSVG -i $BASENAME -w $((32*$scale/100)) -h $((32*$scale/100)) -o "$DIR/$BASENAME.png" > /dev/null

			fi
		done
	done
done
echo -e "\033[0KGenerating animated cursor pixmaps... DONE"



echo -ne "Generating cursor theme...\\r"
for CUR in src/config/*.cursor; do
	BASENAME=$CUR
	BASENAME=${BASENAME##*/}
	BASENAME=${BASENAME%.*}

	TMP_CUR="tmp.cursor"
	./scale_cursor $CUR $SCALES > $TMP_CUR
	ERR="$( xcursorgen -p build "$TMP_CUR" "$OUTPUT/cursors/$BASENAME" 2>&1 )"
	rm $TMP_CUR

	if [[ "$?" -ne "0" ]]; then
		echo "FAIL: $CUR $ERR"
	fi

	cp "build/scalable/$BASENAME.svg" "$OUTPUT/cursors_scalable/"
done
# Animated SVG cursors, currently hand-edited
for CUR in progress wait
do
	cp src/"$CUR".svg $OUTPUT/cursors_scalable/
done
echo -e "Generating cursor theme... DONE"



echo -ne "Generating shortcuts...\\r"
while read ALIAS ; do
	FROM=${ALIAS% *}
	TO=${ALIAS#* }

	if [ -e "$OUTPUT/cursors/$FROM" ] ; then
		continue
	fi

	ln -s "$TO" "$OUTPUT/cursors/$FROM"

done < $ALIASES
while read ALIAS ; do
	FROM=${ALIAS% *}
	TO=${ALIAS#* }

	if [ -e "$OUTPUT/cursors_scalable/$FROM.svg" ] ; then
		continue
	fi

	ln -s "$TO.svg" "$OUTPUT/cursors_scalable/$FROM.svg"

done < $ALIASES
echo -e "\033[0KGenerating shortcuts... DONE"



echo -ne "Copying Theme Index...\\r"
	if ! [ -e "$OUTPUT/$INDEX" ] ; then
		cp $INDEX "$OUTPUT/index.theme"
	fi
echo -e "\033[0KCopying Theme Index... DONE"



echo "COMPLETE!"
