#ifndef QFLEX_H
#define QFLEX_H

#include <stdbool.h>

#include "qemu/osdep.h"
#include "qemu/thread.h"

#include "qflex/qflex-log.h"
#include "qflex/qflex-arch.h"

#define QFLEX(name)       glue(qflex_, name)

/** NOTE for cpu_exec (accel/tcg/cpu_exec.c)
  * Depending on the desired type of execution,
  * cpu_exec should break from the double while loop
  * in the correct manner.
  */
typedef enum {
    SINGLESTEP, // Breaks when a single TB (instruction) is executed
    QEMU        // Normal qemu execution
} QFlexExecType_t;

typedef struct QflexState_t {
    QFlexExecType_t exec_type;
    bool inst_done;
    bool broke_loop;
    bool singlestep;
} QflexState_t;

extern QflexState_t qflexState;

/** qflex_singlestep
 * This functions executes a single instruction (as TB) before returning.
 * It sets the broke_loop flag if it broke the main execution loop (in cpu-exec.c)
 */
int qflex_singlestep(CPUState *cpu);

/** qflex_adaptative_execution
 * This executes normal qemu till a different mode flag is set.
 * For the moment only profiling is supported.
 */
int qflex_adaptative_execution(CPUState *cpu);

/** qflex_cpu_step (cpus.c)
 */
int qflex_cpu_step(void *arg);

/* Get and Setters for state flags and vars
 */
static inline QflexState_t qflex_get_state(void)   { return qflexState; }
static inline QFlexExecType_t qflex_is_type(void)   { return qflexState.exec_type; }
static inline bool qflex_is_inst_done(void)         { return qflexState.inst_done; }
static inline bool qflex_is_broke_loop(void)        { return qflexState.broke_loop; }

static inline void qflex_update_inst_done(bool done) {
    qflexState.inst_done = done; }
static inline void qflex_update_broke_loop(bool broke) {
    qflexState.broke_loop = broke; }
static inline void qflex_update_exec_type(QFlexExecType_t type) {
    qflexState.exec_type = type; }

static inline bool qflex_is_singlestep(void) { return qflexState.exec_type == SINGLESTEP; }
static inline void qflex_singlestep_start(void) {  qflexState.exec_type = SINGLESTEP; }
static inline void qflex_singlestep_stop(void) {  qflexState.exec_type = QEMU; }


/* Located in cpu-exec.c
 * Because QFLEX relies sometimes on added TB instrumentation
 * Flush already decoded TBs such that the extra TCG insts are generated
 */
void qflex_tb_flush(void);
#endif /* QFLEX_H */
