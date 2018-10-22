#!/bin/sh

# build with command:
# CXX=afl-g++ VERIFY=1 make

TESTCASES_DIR=".grip/testcases"
OUTPUT_DIR="findings"
DIFF_DIR="$OUTPUT_DIR/diffs"

GRIP='../grip/grip -E -f'
GREP='grep -rnIE --include="*.cpp" -f'

find . -type f -name '*.cpp' | ../gripgen/gripgen

mkdir -p "$TESTCASES_DIR"
echo 'class|int' > "$TESTCASES_DIR/pattern1"

eval "afl-fuzz -m 1000 -t 10 -i $TESTCASES_DIR -o $OUTPUT_DIR -- ../grip/grip-verify --abort --grip-cmd $GRIP @@ --ext-cmd $GREP @@"

mkdir -p "$DIFF_DIR"

shopt -s nullglob
for f in $OUTPUT_DIR/crashes/*; do
	name=`basename "$f"`

	if [ "$name" != "README.txt" ]; then
		echo " - $name"
		eval "$GRIP $f" > "$DIFF_DIR/$name.grip"
		eval "$GREP $f" > "$DIFF_DIR/$name.grep"
		diff -u "$DIFF_DIR/$name.grep" "$DIFF_DIR/$name.grip" > "$DIFF_DIR/$name.diff"
	fi
done

