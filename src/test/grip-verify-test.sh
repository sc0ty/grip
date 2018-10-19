#!/bin/sh

# build with command:
# CXX=afl-g++ VERIFY=1 make

TESTCASES_DIR=".grip/testcases"

GRIP="../grip/grip -E -f"
GREP="grep -rnIE --include=*.cpp -f"

find . -type f -name '*.cpp' | ../gripgen/gripgen
mkdir -p "$TESTCASES_DIR"
echo 'class' > "$TESTCASES_DIR/pattern1"

afl-fuzz -m 1000 -i "$TESTCASES_DIR" -o findings -- ../grip/grip-verify --abort --grip-cmd $GRIP @@ --ext-cmd $GREP @@

for f in findings/crashes/id*; do
	$GRIP $f > "$f.grip"
	$GREP $f > "$f.grep"
	diff -u "$f.grep" "$f.grip" > "$f.diff"
done

