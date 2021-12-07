#include "devteroflex.h"
#include "qemu/thread.h"

typedef struct DevteroflexTrace_t {
    bool tracing;
    bool multi_buffer;
    uint64_t num_cpus;
    QemuMutex mtx;
    uint64_t buffer_size;
    uint64_t *buffer_index;
    uint64_t *circular_buffer;
} DevteroflexTrace_t;

extern DevteroflexTrace_t devteroflexTrace;

void devteroflex_trace_set(bool set, bool multi_buffer, uint64_t buffer_size);
void devteroflex_trace_callback_single(uint64_t pc_curr);
void devteroflex_trace_callback_multi(uint64_t cpu_idx, uint64_t pc_curr);
