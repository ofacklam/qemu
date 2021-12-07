#include "devteroflex.h"
#include "qemu/thread.h"

#define BUFFER_SIZE 1000 // number of instructions (PCs) logged

typedef struct DevteroflexTrace_t {
    bool tracing;
    uint64_t max_calls;
    uint64_t nb_calls;
    uint64_t num_cpus;
    QemuMutex mtx;
    uint64_t buffer_index;
    uint64_t circular_buffer[BUFFER_SIZE];
} DevteroflexTrace_t;

extern DevteroflexTrace_t devteroflexTrace;

void devteroflex_trace_set(bool set, uint64_t nb_insn);
void devteroflex_trace_callback(uint64_t pc_curr);
