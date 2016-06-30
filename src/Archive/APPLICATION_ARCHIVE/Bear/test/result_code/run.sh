#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/result_code/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/result_code/../../libear/libear.so"

set +o errexit
python ${BEAR_EXE} -l ${BEAR_LIB} false
rc=$?
set -o errexit

if [ 0 = $rc ]; then
  echo "exit code was not captured" && false
fi

python ${BEAR_EXE} -l ${BEAR_LIB} true
