#include "sysemu/tcg.h"

#include "qemu/osdep.h"
#include <dirent.h>
#include "monitor/monitor-internal.h"
#include "qapi/error.h"
#include "qapi/qmp/qdict.h"
#include "qapi/qmp/qnum.h"

#include "qflex/qflex-hmp.h"
#include "qflex/qflex.h"
#include "qflex/qflex-traces.h"

void hmp_qflex_singlestep_start(Monitor *mon, const QDict *qdict) {
    int error = 0;
    if(!tcg_enabled()) {
       monitor_printf(mon, "`singlestep` available only with TCG.");
       error |= 1;
    }
    if(!qemu_loglevel_mask(CPU_LOG_TB_NOCHAIN)) {
        monitor_printf(mon, "`singlestep` available only with `-d nochain`.");
        error |= 1;
    }
    if(error) return;

    qflex_singlestep_start();
}

void hmp_qflex_singlestep_stop(Monitor *mon, const QDict *qdict) {
    if(!qflexState.singlestep) {
       monitor_printf(mon, "singlestepping was not running");
    }
    qflex_singlestep_stop();
}

void hmp_qflex_mem_trace_start(Monitor *mon, const QDict *qdict) {
    size_t nb_insn = qdict_get_int(qdict, "nb_insn");
    int trace_type = qdict_get_int(qdict, "trace_type");
    qflex_mem_trace_start(nb_insn, trace_type);
}

void hmp_qflex_mem_trace_stop(Monitor *mon, const QDict *qdict) {
    qflex_mem_trace_stop();
}

void hmp_qflex_mem_trace_end(Monitor *mon, const QDict *qdict) {
    qflex_mem_trace_end();
}

void hmp_qflex_mem_trace_log_stats(Monitor *mon, const QDict *qdict) {
    char log[256];
    qflex_mem_trace_log_stats(log, 256);
    monitor_printf(mon, "%s", log);
}

#ifdef CONFIG_DEVTEROFLEX
#include "qflex/devteroflex/devteroflex.h"
#include "qflex/devteroflex/verification.h"
#include "qflex/devteroflex/custom-instrumentation.h"
void hmp_devteroflex_start(Monitor *mon, const QDict *qdict) {
    devteroflex_init(true, true);
}

void hmp_devteroflex_gen_verification_start(Monitor *mon, const QDict *qdict) {
    size_t nb_insn = qdict_get_int(qdict, "nb_insn");
    gen_verification_start(nb_insn);
}

void hmp_devteroflex_gen_example(Monitor *mon, const QDict *qdict) {
    size_t nb_insn = qdict_get_int(qdict, "nb_insn");
    const char *op = qdict_get_str(qdict, "op");
    if(strcmp(op, "start") == 0) {
        monitor_printf(mon, "DevteroFlex: start gen example helper for %li instructions\n", nb_insn);
        devteroflex_gen_example_set(true, nb_insn);
    } else if (strcmp(op, "stop") == 0) {
        monitor_printf(mon, "DevteroFlex: stop gen example helper\n");
        devteroflex_gen_example_set(false, 0);
    } else {
        monitor_printf(mon, "Devteroflex: Unexpected 'op' parameter [start | stop]\n");
    }
}
#endif