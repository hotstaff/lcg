#!/bin/bash

echo "---ENCRYPT/DECRYPT TEST---"

SCRIPT_DIR=$(cd $(dirname $0); pwd)

INPUT=$SCRIPT_DIR/sample.bin
OUTPUT=$SCRIPT_DIR/output_encrypted.bin
INPUT_R=$SCRIPT_DIR/sample_decrypted.bin
KEY=$SCRIPT_DIR/output_encrypted.key

rm $OUTPUT
rm $INPUT_R
rm $KEY

../src/lcg $INPUT $OUTPUT -k $KEY
../src/lcg -d $OUTPUT $INPUT_R -k $KEY

cmp -lb $INPUT $INPUT_R
if [ "$?" -eq 0 ]; then
	echo "Test success."
	exit 0
fi

echo "Test fail."
exit 1


