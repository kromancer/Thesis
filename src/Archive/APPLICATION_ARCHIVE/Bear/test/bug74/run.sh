#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/bug74/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/bug74/../../libear/libear.so"

RUN_TEST="cc -c -MMD /home/kisp/Bear_src/test/bug74/foo.c"

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb out.json sh -c "$RUN_TEST"
test `grep "command" out.json | wc -l` -eq 1
test `cat out.json | grep "command" | grep "MMD" | wc -l` -eq 0
