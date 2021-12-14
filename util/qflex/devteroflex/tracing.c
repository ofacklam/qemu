
#include "qflex/qflex.h"
#include "qflex/devteroflex/tracing.h"
#include "qflex/devteroflex/file-ops.h"
#include "qflex/qflex-log.h"
#include "hw/qdev-core.h"
#include "hw/boards.h"

#define MULTI_THREAD 0

DevteroflexTrace_t devteroflexTrace = {
    .tracing = false,
    .multi_buffer = false,
    .num_cpus = 0,
    .buffer_size = 0,
    .buffer_index = NULL,
    .circular_buffer = NULL,
};

static void dump_buffer_contents(void) {
    FILE* fp;
    file_stream_open(&fp, "execution_trace");

    uint64_t outer_iterations = devteroflexTrace.multi_buffer ? devteroflexTrace.num_cpus : 1;

    for(uint64_t cpu_idx = 0; cpu_idx < outer_iterations; cpu_idx++) {
        for(uint64_t i = 0; i < devteroflexTrace.buffer_size; i++) {
            uint64_t idx_curr = cpu_idx * devteroflexTrace.buffer_size
                    + (devteroflexTrace.buffer_index[cpu_idx] + i) % devteroflexTrace.buffer_size;
            fprintf(fp, "0x%016"PRIx64"\n", devteroflexTrace.circular_buffer[idx_curr]);
        }
        fprintf(fp, "==========================\n");
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
void devteroflex_trace_set(bool set, bool multi, uint64_t bufsize)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    devteroflexTrace.tracing = set;

    if(set) {
        devteroflexTrace.multi_buffer = multi;
        devteroflexTrace.num_cpus = ms->smp.cpus;
        qemu_mutex_init(&devteroflexTrace.mtx);
        devteroflexTrace.buffer_size = bufsize;

        uint64_t num_buffers = devteroflexTrace.multi_buffer ? devteroflexTrace.num_cpus : 1;
        devteroflexTrace.buffer_index = (uint64_t*) calloc(num_buffers, sizeof(uint64_t));
        devteroflexTrace.circular_buffer = (uint64_t*) calloc(num_buffers * devteroflexTrace.buffer_size, sizeof(uint64_t));
    }

    qflex_tb_flush();

    if(!set) {
        dump_buffer_contents();

        devteroflexTrace.multi_buffer = false;
        devteroflexTrace.num_cpus = 0;
        qemu_mutex_destroy(&devteroflexTrace.mtx);
        devteroflexTrace.buffer_size = 0;

        free(devteroflexTrace.buffer_index);
        devteroflexTrace.buffer_index = NULL;
        free(devteroflexTrace.circular_buffer);
        devteroflexTrace.circular_buffer = NULL;
    }

}

/**
 *
 * NOTE: for the `devteroflexTrace.nb_calls` variable, in case you run with Multi-threaded
 *       TCG, it will most likely not be exact because of concurrent additions.
 *       Multiple threads might be updating the `nb_calls` at the same time resulting 
 *       in data races.
 */
void devteroflex_trace_callback_single(uint64_t pc_curr)
{
#if MULTI_THREAD
    qemu_mutex_lock(&devteroflexTrace.mtx);
#endif

    // GET & UPDATE BUFFER INDEX
    uint64_t idx_curr = devteroflexTrace.buffer_index[0];
    devteroflexTrace.buffer_index[0] = (idx_curr + 1) % devteroflexTrace.buffer_size;

    // DUMP THE PROCESSOR STATE
    devteroflexTrace.circular_buffer[idx_curr] = pc_curr;

#if MULTI_THREAD
    qemu_mutex_unlock(&devteroflexTrace.mtx);
#endif
}

void devteroflex_trace_callback_multi(uint64_t cpu_idx, uint64_t pc_curr)
{
    // GET BUFFER NUMBER
    uint64_t buf_num = cpu_idx;

    // GET & UPDATE BUFFER INDEX
    uint64_t idx_curr = devteroflexTrace.buffer_index[buf_num];
    devteroflexTrace.buffer_index[buf_num] = (idx_curr + 1) % devteroflexTrace.buffer_size;

    // DUMP THE PROCESSOR STATE
    devteroflexTrace.circular_buffer[buf_num * devteroflexTrace.buffer_size + idx_curr] = pc_curr;
}
