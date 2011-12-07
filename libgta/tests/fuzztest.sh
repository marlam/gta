#!/bin/sh

if test $# -eq 0; then
    echo "Please consider to also run 'make fuzztest', which will"
    echo "run until it finds an error or you interrupt it."
    exit 77
fi

startsec="`date +%s`"
seed="$startsec"
i=0
while true; do
    seed="`./fuzztest-create $seed fuzztest-valid.gta fuzztest-corrupt.gta`"
    if test $? -ne 0; then
        echo 'Failed to create fuzz test files'
        break
    fi
    ./fuzztest-check fuzztest-valid.gta fuzztest-corrupt.gta
    if test $? -ne 0; then
        echo 'Fuzz test failed!'
        echo 'Examine fuzztest-valid.gta and fuzztest-corrupt.gta'
        break
    fi
    i=$((i+1))
    if test $i -eq 1000; then
        currentsec="`date +%s`";
        echo "1000 fuzz tests succeeded in $(($currentsec-$startsec)) seconds"
        i=0
        startsec=$currentsec
    fi
done
