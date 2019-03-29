#!/bin/bash

echo "---ENCODE/DECODE TEST---"

SCRIPT_DIR=$(cd $(dirname $0); pwd)

INPUT=$SCRIPT_DIR/sample.bin
OUTPUT=$SCRIPT_DIR/output.bin
INPUT_R=$SCRIPT_DIR/sample_r.bin

rm $OUTPUT
rm $INPUT_R
../src/lcg $INPUT $OUTPUT
../src/lcg -d $OUTPUT $INPUT_R

cmp -lb $INPUT $INPUT_R
if [ "$?" -eq 0 ]; then
	echo "Test success."
	exit 0
fi

echo "Test fail."
exit 1


