#!/bin/bash

EMAS=build/emas
EMDAS=emdas
BASEDIR=../em400/tests
TESTDIRS="alu args cycle int mem mod multix ops registers vendor"

for tests in $TESTDIRS ; do
	files=$(ls -1 $BASEDIR/functional/$tests/*.asm 2>/dev/null)
	if [ -z "$files" ] ; then
		echo "Missing tests"
		exit 1
	fi
	for file in $files ; do
		echo $file
		$EMAS -Iasminc -Oraw -o /tmp/emas.bin -I $BASEDIR/include $file
		$EMDAS -c mx16 -na -o /tmp/emas.asm /tmp/emas.bin
		$EMAS -Oraw -c mx16 -o /tmp/emas2.bin /tmp/emas.asm
		cmp /tmp/emas.bin /tmp/emas2.bin || exit 1
	done
done
