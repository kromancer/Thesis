#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/bug81/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/bug81/../../libear/libear.so"

RUN_MAKE="make -C /home/kisp/Bear_src/test/bug81 -f build.mk"

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb one.json $RUN_MAKE
test `grep "command" one.json | wc -l` -eq 1
$RUN_MAKE clean
