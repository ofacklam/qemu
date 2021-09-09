#include "sysemu/tcg.h"

#include "qemu/osdep.h"
#include <dirent.h>
#include "monitor/monitor-internal.h"
#include "qapi/error.h"
#include "qapi/qmp/qdict.h"
#include "qapi/qmp/qnum.h"

#include "qflex/qflex-hmp.h"
#include "qflex/qflex.h"

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
    if(!qflex_is_singlestep()) {
       monitor_printf(mon, "singlestepping was not running");
    }
    qflex_singlestep_stop();
}