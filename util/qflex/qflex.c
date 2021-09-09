#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "qemu/osdep.h"
#include "qemu/thread.h"

#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/error-report.h"
#include "qapi/qmp/qerror.h"
#include "qemu/option_int.h"
#include "qemu/main-loop.h"

#include "qflex/qflex-config.h"
#include "qflex/qflex.h"

QflexState_t qflexState = {
    .inst_done = false,
    .broke_loop = false,
    .singlestep = false,
    .exec_type = QEMU
};

int qflex_singlestep(CPUState *cpu) {
    int ret = 0;
    qflex_update_exec_type(SINGLESTEP);

    while(!qflex_is_inst_done()) {
        ret = qflex_cpu_step(cpu);
    }

    qflex_update_inst_done(false);

    return ret;
}

int qflex_adaptative_execution(CPUState *cpu) {
    while(1) {
        if(qflex_is_singlestep()) {
            qflex_singlestep(cpu);
        }
    }
}