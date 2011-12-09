#!/bin/sh

if test $# -eq 0; then
    echo "Please consider to also run 'make fuzztest', which will"
    echo "run until it finds an error or you interrupt it."
    exit 77
fi

dir="`mktemp -d fuzztest-XXXXXX`"
trap "rm -r $dir" INT

startsec="`date +%s`"
#seed="$startsec"
seed="`od -vAn -N4 -tu4 < /dev/urandom`"
i=0
while true; do
    validgta="$dir/fuzztest-valid.gta"
    corruptgta="$dir/fuzztest-corrupt.gta"
    newseed="`./fuzztest-create "$seed" "$validgta" "$corruptgta"`"
    if test $? -ne 0; then
        echo "Failed to create fuzz test files."
        break
    fi
    ./fuzztest-check "$validgta" "$corruptgta"
    if test $? -ne 0; then
        echo "Fuzz test generated with seed $seed failed"'!'
        echo "Examine $validgta and $corruptgta."
        break
    fi
    rm "$validgta" "$corruptgta"
    i=$((i+1))
    if test $i -eq 1000; then
        currentsec="`date +%s`";
        echo "1000 fuzz tests succeeded in $(($currentsec-$startsec)) seconds."
        i=0
        startsec=$currentsec
    fi
    seed="$newseed"
done
