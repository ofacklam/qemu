
#include "qflex/qflex.h"
#include "qflex/devteroflex/tracing.h"
#include "qflex/devteroflex/file-ops.h"
#include "qflex/qflex-log.h"
#include "hw/qdev-core.h"
#include "hw/boards.h"

DevteroflexTrace_t devteroflexTrace = {
    .tracing = false,
    .max_calls = 0,
    .nb_calls = 0,
    .num_cpus = 0,
    .buffer_index = 0,
};

static void dump_buffer_contents(void) {
    FILE* fp;
    file_stream_open(&fp, "execution_trace");

    for(uint64_t i = 0; i < BUFFER_SIZE; i++) {
        uint64_t idx_curr = (i + devteroflexTrace.buffer_index) % BUFFER_SIZE;
        fprintf(fp, "0x%016"PRIx64"\n", devteroflexTrace.circular_buffer[idx_curr]);
    }

    fclose(fp);
}

/** 
 * If the helper is inserted on every function callbacks, it will slowdown
 * considerably the program.
 * We might want only to enable callbacks on specific moments.
 * 
 * TB cache should be cleaned by calling `qflex_tb_flush()` when the flag is set.
 * This will ensure that previously decoded instructions will redecode 
 * and insert the helper.
 */
void devteroflex_trace_set(bool set, uint64_t nb_insn)
{
    MachineState *ms = MACHINE(qdev_get_machine());

    devteroflexTrace.tracing = set;
    devteroflexTrace.max_calls = nb_insn;
    devteroflexTrace.nb_calls = 0;
    devteroflexTrace.num_cpus = ms->smp.cpus;

    if(set) {
        qemu_mutex_init(&devteroflexTrace.mtx);
        devteroflexTrace.buffer_index = 0;
    }

    qflex_tb_flush();

    if(!set) {
        qemu_mutex_destroy(&devteroflexTrace.mtx);
        dump_buffer_contents();
    }
}

/**
 *
 * NOTE: for the `devteroflexTrace.nb_calls` variable, in case you run with Multi-threaded
 *       TCG, it will most likely not be exact because of concurrent additions.
 *       Multiple threads might be updating the `nb_calls` at the same time resulting 
 *       in data races.
 */
void devteroflex_trace_callback(uint64_t pc_curr)
{
    qemu_mutex_lock(&devteroflexTrace.mtx);

    // GET & UPDATE BUFFER INDEX
    /*uint64_t idx_curr, idx_new;
    do {
        idx_curr = devteroflexTrace.buffer_index;
        idx_new = (idx_curr + 1) % BUFFER_SIZE;
    } while (qatomic_cmpxchg(&devteroflexTrace.buffer_index, idx_curr, idx_new) != idx_curr);*/
    uint64_t idx_curr = devteroflexTrace.buffer_index;
    devteroflexTrace.buffer_index = (idx_curr + 1) % BUFFER_SIZE;

    // DUMP THE PROCESSOR STATE
    devteroflexTrace.circular_buffer[idx_curr] = pc_curr;

    qemu_mutex_unlock(&devteroflexTrace.mtx);
}
