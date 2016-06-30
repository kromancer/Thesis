#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/bug94/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/bug94/../../libear/libear.so"

RUN_TEST="cc -c /home/kisp/Bear_src/test/bug94/foo.c /home/kisp/Bear_src/test/bug94/bar.c"

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb out.json sh -c "$RUN_TEST"
test `grep "command" out.json | wc -l` -eq 2
test `cat out.json | grep "command" | grep "foo.c" | wc -l` -eq 1
test `cat out.json | grep "command" | grep "bar.c" | wc -l` -eq 1
