#!/bin/sh

set -o nounset
set -o errexit

BEAR_EXE="/home/kisp/Bear/test/exec_anatomy/../../bear/bear"
BEAR_LIB="/home/kisp/Bear/test/exec_anatomy/../../libear/libear.so"

touch execve.c execv.c execvpe.c execvp.c execl.c execlp.c execle.c execv_p.c posix_spawn.c posix_spawnp.c
python ${BEAR_EXE} -l ${BEAR_LIB} ./exec_anatomy expected.json
python "/home/kisp/Bear_src/test/exec_anatomy/json_diff.py" expected.json compile_commands.json
