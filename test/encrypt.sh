#!/bin/bash
echo "---ENCRYPT/DECRYPT TEST---"

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TMPDIR=$(mktemp -d)

INPUT=$SCRIPT_DIR/sample.bin
OUTPUT=$TMPDIR/output_encrypted.bin
INPUT_R=$TMPDIR/sample_decrypted.bin
KEY=$TMPDIR/output_encrypted.key

../src/lcg -k $KEY $INPUT $OUTPUT
../src/lcg -d -k $KEY $OUTPUT $INPUT_R

cmp -lb $INPUT $INPUT_R
RESULT=$?

rmdir --ignore-fail-on-non-empty $TMPDIR 

if [ $RESULT -eq 0 ]; then
	echo "Test success."
	exit 0
fi

echo "Test fail."
exit 1


