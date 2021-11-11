
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
    .log_files = NULL
};

static void open_files(uint64_t num_cpus) {
    devteroflexTrace.num_cpus = num_cpus;
    devteroflexTrace.log_files = (FILE**) calloc(num_cpus, sizeof(FILE*));

    char filename[sizeof "execution_trace_00"];
    for(uint64_t i = 0; i < num_cpus; i++) {
        sprintf(filename, "execution_trace_%02ld", i);
        file_stream_open(&devteroflexTrace.log_files[i], filename);
    }
}

static void close_files(void) {
    if(devteroflexTrace.log_files) {
        for(uint64_t i = 0; i < devteroflexTrace.num_cpus; i++) {
            fclose(devteroflexTrace.log_files[i]);
        }

        free(devteroflexTrace.log_files);
        devteroflexTrace.log_files = NULL;
        devteroflexTrace.num_cpus = 0;
    }
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

    if(set) {
        close_files();
        open_files(ms->smp.cpus);
    }

    qflex_tb_flush();
}

/**
 *
 * NOTE: for the `devteroflexTrace.nb_calls` variable, in case you run with Multi-threaded
 *       TCG, it will most likely not be exact because of concurrent additions.
 *       Multiple threads might be updating the `nb_calls` at the same time resulting 
 *       in data races.
 */
void devteroflex_trace_callback(uint64_t cpu_index, DevteroflexArchState *cpu)
{
    FILE *cpu_file = devteroflexTrace.log_files[cpu_index];

    // DUMP THE PROCESSOR STATE
    fprintf(cpu_file, "CPU state for PC[0x%016"PRIx64"]:\n", cpu->pc);
    for(int i = 0; i < 32; i++) {
        fprintf(cpu_file, "\t r%02"PRId32" = 0x%016"PRIx64"\n", i, cpu->xregs[i]);
    }
    fprintf(cpu_file, "\t SP = 0x%016"PRIx64"\n", cpu->sp);
    fprintf(cpu_file, "\t NZCV = 0x%016"PRIx64"\n", cpu->nzcv);

    // FLUSH THE FILE STREAM
    fflush(cpu_file);

    devteroflexTrace.nb_calls++;
    if(devteroflexTrace.max_calls <= devteroflexTrace.nb_calls) {
        // Stop executing helpers
        devteroflex_trace_set(false, 0);
    }
}
