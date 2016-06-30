#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/bug80/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/bug80/../../libear/libear.so"

RUN_MAKE="make -C /home/kisp/Bear_src/test/bug80 -f build.mk"

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb one.json $RUN_MAKE test-in-one
$RUN_MAKE clean
test `grep "command" one.json | wc -l` -eq 1

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb two.json $RUN_MAKE test-in-two
$RUN_MAKE clean
test `grep "command" two.json | wc -l` -eq 1
