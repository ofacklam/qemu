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

#include "qflex/qflex.h"
#include "qflex/qflex-traces.h"


QflexState_t qflexState = {
    .inst_done = false,
    .broke_loop = false,
    .singlestep = false,
    .exit_main_loop = false,
    .skip_interrupts = false
};

int qflex_singlestep(CPUState *cpu) {
    int ret;
    ret = qflex_cpu_step(cpu);

    if(ret) {
        qemu_log("QFLEX: Singlestep went wrong\n");
    }

    return ret;
}

int qflex_adaptative_execution(CPUState *cpu) {
    while(1) {
        if(!qflex_is_exit_main_loop()) {
            break;
        } 
        else if(qflexState.singlestep) {
            qflex_singlestep(cpu);
        }
    }
    }
    return 0;
}