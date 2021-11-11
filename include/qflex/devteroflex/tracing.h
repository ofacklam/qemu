#include "devteroflex.h"

typedef struct DevteroflexTrace_t {
    bool tracing;
    uint64_t max_calls;
    uint64_t nb_calls;
    uint64_t num_cpus;
    FILE **log_files;
} DevteroflexTrace_t;

extern DevteroflexTrace_t devteroflexTrace;

void devteroflex_trace_set(bool set, uint64_t nb_insn);
void devteroflex_trace_callback(uint64_t cpu_index, DevteroflexArchState *cpu);
