#!/bin/bash

echo "---ENCRYPT/DECRYPT TEST---"

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TMPDIR=$(mktemp -d)

INPUT=$SCRIPT_DIR/sample.bin
OUTPUT=$TMPDIR/output_encrypted.bin
INPUT_R=$TMPDIR/sample_decrypted.bin
KEY=$TMPDIR/output_encrypted.key

rm $OUTPUT
rm $INPUT_R
rm $KEY

../src/lcg $INPUT $OUTPUT -k $KEY
../src/lcg -d $OUTPUT $INPUT_R -k $KEY

cmp -lb $INPUT $INPUT_R
RESULT=$?

rmdir $TMPDIR --ignore-fail-on-non-empty

if [ $RESULT -eq 0 ]; then
	echo "Test success."
	exit 0
fi

echo "Test fail."
exit 1


