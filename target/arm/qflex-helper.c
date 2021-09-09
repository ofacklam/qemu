#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h"
#include "exec/log.h"
#include "exec/exec-all.h"

#include "qflex/qflex.h"
#include "qflex-helper.h"
#include "qflex/qflex-helper-a64.h"

/* TCG helper functions. (See exec/helper-proto.h  and target/arch/helper-a64.h)
 * This one expands prototypes for the helper functions.
 * They get executed in the TB
 * To use them: in translate.c or translate-a64.c
 * ex: HELPER(qflex_func)(arg1, arg2, ..., argn)
 * gen_helper_qflex_func(arg1, arg2, ..., argn)
 */

/**
 * @brief HELPER(qflex_magic_inst)
 * In ARM, hint instruction (which is like a NOP) comes with an int with range 0-127
 * Big part of this range is defined as a normal NOP.
 * Too see which indexes are already used ref (curently 39-127 is free) :
 * https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/hint-hint-instruction
 *
 * This function is called when a HINT n (90 < n < 127) TB is executed
 * nop_op: in HINT n, it's the selected n.
 *
 * To do more complex calls from QEMU guest to host
 * You can chain two nops
 *
 */
#include "qflex/qflex-qemu-calls.h"
static uint64_t prev_nop_op = 0;

static inline void qflex_cmds(uint64_t nop_op) {
    switch(nop_op) {
        case QFLEX_SINGLESTEP_START: qflex_singlestep_start(); break;
        case QFLEX_SINGLESTEP_STOP: qflex_singlestep_stop(); break;
        default: break;
    }
}

void HELPER(qflex_magic_inst)(CPUARMState *env, uint64_t nop_op) {
    assert(nop_op >= 90);
    assert(nop_op <= 127);
    qflex_log_mask(QFLEX_LOG_MAGIC_INST, "MAGIC_INST:%"PRIu64"\n", nop_op);

    // CPUState *cs = CPU(env_archcpu(env));

    // Make chained nop_op event
    switch(prev_nop_op) {

        case QFLEX_OP:
            qflex_cmds(nop_op);
            prev_nop_op = 0;
            return; // HELPER EXIT

        default: break;
    }

    // Get chained nop_op op type
    switch(nop_op) {
        case QFLEX_OP: prev_nop_op = QFLEX_OP; break;
        default: prev_nop_op = 0; break;
    }
}
