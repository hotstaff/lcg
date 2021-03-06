#!/bin/bash
echo "---ENCODE/DECODE TEST---"

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TMPDIR=$(mktemp -d)

INPUT=$SCRIPT_DIR/sample.bin
OUTPUT=$TMPDIR/output.bin
INPUT_R=$TMPDIR/sample_r.bin

../src/lcg $INPUT $OUTPUT
../src/lcg -d $OUTPUT $INPUT_R

cmp -lb $INPUT $INPUT_R
RESULT=$?

$(rmdir --ignore-fail-on-non-empty $TMPDIR)

if [ $RESULT -eq 0 ]; then
	echo "Test success."
	exit 0
fi

echo "Test fail."
exit 1


