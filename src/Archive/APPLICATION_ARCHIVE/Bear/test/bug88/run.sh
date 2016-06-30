#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/bug88/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/bug88/../../libear/libear.so"

RUN_TEST='cc "-DKEY=\"value with spaces\"" /home/kisp/Bear_src/test/bug88/spaces.c -o space'

python ${BEAR_EXE} -l ${BEAR_LIB} --cdb out.json sh -c "$RUN_TEST"

[ "$(./space)" = "value with spaces" ] && echo "compiled correctly"
[ "$(cat out.json | grep command | wc -l)" -eq 1 ] && echo "command captured"
[ "$(cat out.json | grep command | sed 's/\s/\n/g' | uniq | grep -c -e \\\\\\\")" -eq 2 ] && echo "3 backslash is there"
