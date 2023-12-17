#!/bin/bash

set -e

if [ -z "$1" ] ; then
	echo "Missing em400 tests directory"
	echo "Usage: $0 <tests_directory>"
	exit 1
fi

CURD=$(readlink -f .)
EMAS=$(find $CURD -type f -executable -name emas -or -name emas.exe)
STABLE_EMAS=$(which emas)
if [ -z "$EMAS" ] ; then
	EMAS=$(find $CURD/.. -type f -executable -name emas -or -name emas.exe)
fi
if [ -z "$EMAS" ] ; then
	echo "emas binary not found in current or upper directory"
	exit 2
fi

EMDAS=$(which emdas)
BASEDIR=$1
TESTDIRS="addr alu args barnb cycle int mem mod multix ops registers vendor"

echo "Testing assembly with: $EMAS"
if [ -n "$EMDAS" ] ; then
	echo "Testing against deassembly using: $EMDAS"
fi
if [ -n "$STABLE_EMAS" ] ; then
	echo "Testing against stable version using: $STABLE_EMAS"
fi

for tests in $TESTDIRS ; do
	echo
	echo "--- Test directory: $tests -----------------------------"
	files=$(ls -1 $BASEDIR/functional/$tests/*.asm 2>/dev/null)
	if [ -z "$files" ] ; then
		echo "Missing tests"
		exit 1
	fi
	for file in $files ; do
		test_name=$(basename $file)
		echo -n "$test_name "
		$EMAS -I ../asminc -Oraw -o /tmp/emas.bin -I $BASEDIR/include $file
		if [ -n "$EMDAS" ] ; then
			$EMDAS -c mx16 -na -o /tmp/emas.asm /tmp/emas.bin
			$EMAS -Oraw -c mx16 -o /tmp/emas2.bin /tmp/emas.asm
			cmp /tmp/emas.bin /tmp/emas2.bin
		fi
		if [ -n "$STABLE_EMAS" ] ; then
			$STABLE_EMAS -I ../asminc -Oraw -o /tmp/emas_stable.bin -I $BASEDIR/include $file
			cmp /tmp/emas.bin /tmp/emas_stable.bin
		fi
	done
done
echo
