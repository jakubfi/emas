#!/bin/bash

set -e

if [ -z "$1" ] ; then
	echo "Missing em400 tests directory"
	echo "Usage: $0 <tests_directory>"
	exit 1
fi

CURD=$(readlink -f .)
EMAS=$(find $CURD -type f -executable -name emas -or -name emas.exe)
if [ -z "$EMAS" ] ; then
	EMAS=$(find $CURD/.. -type f -executable -name emas -or -name emas.exe)
fi
if [ -z "$EMAS" ] ; then
	echo "emas binary not found in current or upper directory"
	exit 2
fi

EMDAS=emdas
BASEDIR=$1
TESTDIRS="addr alu args cycle int mem mod multix ops registers vendor"

for tests in $TESTDIRS ; do
	files=$(ls -1 $BASEDIR/functional/$tests/*.asm 2>/dev/null)
	if [ -z "$files" ] ; then
		echo "Missing tests"
		exit 1
	fi
	for file in $files ; do
		test_name=$(basename $file)
		$EMAS -I ../asminc -Oraw -o /tmp/emas.bin -I $BASEDIR/include $file
		$EMDAS -c mx16 -na -o /tmp/emas.asm /tmp/emas.bin
		$EMAS -Oraw -c mx16 -o /tmp/emas2.bin /tmp/emas.asm
		cmp /tmp/emas.bin /tmp/emas2.bin
	done
done
