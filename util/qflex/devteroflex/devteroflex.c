#include <stdbool.h>

#include "qemu/osdep.h"
#include "qemu/thread.h"
#include "hw/core/cpu.h"

#include "qflex/qflex.h"
#include "qflex/qflex-arch.h"

#include "qflex/devteroflex/devteroflex.h"
#include "qflex/devteroflex/page-demander.h"

#ifdef AWS_FPGA
#include "qflex/devteroflex/aws/fpga.h"
#include "qflex/devteroflex/aws/fpga_interface.h"
#else
#include "qflex/devteroflex/simulation/fpga.h"
#include "qflex/devteroflex/simulation/fpga_interface.h"
#endif


DevteroflexConfig devteroflexConfig = { false, false };
static FPGAContext c;
static DevteroflexArchState state;

static uint64_t insert_entry_get_ppn(uint64_t hvp, uint64_t ipt_bits) {
    uint64_t paddr;
    int ret = devteroflex_ipt_add_entry(hvp, ipt_bits, &paddr);
    if(ret == SYNONYM) {
        return paddr;
    } else if (ret == PAGE) {
        return get_free_paddr();
    } else {
        perror("IPT Table error in adding entry");
        return -1;
    }
}

void devteroflex_push_page_fault(uint64_t hvp, uint64_t ipt_bits) {
     QEMUMissReply reply = {
        .type = sMissReply,
        .vpn_hi = VPN_GET_HI(IPT_GET_VA(ipt_bits)),
        .vpn_lo = VPN_GET_LO(IPT_GET_VA(ipt_bits)),
        .pid = IPT_GET_PID(ipt_bits),
        .permission = IPT_GET_PER(ipt_bits)
    };
    
    uint64_t paddr = -1;
    int ret = devteroflex_ipt_add_entry(hvp, ipt_bits, &paddr);
    if (ret == SYNONYM) {
        reply.ppn = GET_PPN_FROM_PADDR(paddr);
    } else if (ret == PAGE) {
        reply.ppn = 0; //TODO devteroflex_pop_paddr();
        pushPageToFPGA(&c, reply.ppn, (void*) hvp);
    }
    
    sendMessageToFPGA(&c, (void *) &reply, sizeof(reply));
}

// TODO:
/* Evictions are done lazely, and translation from GVA to HVA require the CPUState, which 
 * might already have changed. We must store the previously translated gva2hva somewhere.
 */
static void handle_evict_notify(PageEvictNotification *message) {
    /*
    uint64_t ipt_bits = IPT_COMPRESS(IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo), message->pid, message->permission);

    CPUState *cpu = qemu_get_cpu(message->tid);
    uint64_t hvp = gva_to_hva(cpu, IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo) & PAGE_MASK, message->permission);
    assert(hvp != -1);

    if(message->modified) {
        int mask = 0;
        for(int i = 0; i < 32; i++) {
            mask = 1 << i;
            if(!(evict_pending_v & mask)) {
                evict_pending_v |= mask;
                //evict_pending[i][0] = hvp;
                evict_pending[i][1] = ipt_bits;
                break;
            }
        }
    } else {
        devteroflex_ipt_evict(hvp, ipt_bits);
    }
    */
}

// TODO: Same as Evict Notify
static void handle_evict_writeback(PageEvictNotification* message) {
    /*
    uint64_t ipt_bits = IPT_COMPRESS(IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo), message->pid, message->permission);
    CPUState *cpu = qemu_get_cpu(message->tid);
    uint64_t hvp = gva_to_hva(cpu, IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo), message->permission);
    assert(hvp != -1);

    fetchPageFromFPGA(&c, message->ppn, (void *) hvp);
    devteroflex_ipt_evict(hvp, ipt_bits);
    run_pending_raw(hvp);

    int mask = 0;
    for(int i = 0; i < 32; i++) {
        mask = 1 << i;
        if(evict_pending_v & mask) {
            if(ipt_bits == evict_pending[i][1]) {
                evict_pending_v &= ~mask;
                assert(hvp == evict_pending[i][0]);
                break;
            }
        }
    }
    */
}


static void handle_page_fault(PageFaultNotification *message) {
    CPUState *cpu;
    CPU_FOREACH(cpu) {
        if(message->tid == cpu->cpu_index) 
            break;
    }
    assert(message->tid == cpu->cpu_index);

    uint64_t ipt_bits = IPT_COMPRESS(IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo), message->pid, message->permission);
    uint64_t hvp = gva_to_hva(cpu, IPT_ASSEMBLE_64(message->vpn_hi, message->vpn_lo) & PAGE_MASK, message->permission);
    if(hvp == -1) {
        // Permission Violation
        // devteroflex_push_resp(cpu, ipt_bits, FAULT); // TODO
        perror("TODO: Not supported yet transplant back");
        return;
    }
    // TODO: Insert hvp into a table to save gva2hva translation for eviction
    uint64_t ppn = insert_entry_get_ppn(hvp, ipt_bits);
    pushPageToFPGA(&c, GET_PPN_FROM_PADDR(ppn), (void*) hvp);
    devteroflex_push_page_fault(hvp, ipt_bits);
}

static void run_requests(void) {
    uint8_t message[64];
    MessageType type;
    if(hasMessagePending(&c)) {
        getMessagePending(&c, message);
        type = ((uint32_t*) message)[0];
        switch(type) {
            case sPageFaultNotify:
            handle_page_fault((PageFaultNotification *) message);
            break;
            case sEvictNotify:
            handle_evict_notify((PageEvictNotification *) message);
            break;
            case sEvictDone:
            handle_evict_writeback((PageEvictNotification *) message);
            break;
            default:
            exit(1);
            break;
        }
    }
}


static void run_transplant(CPUState *cpu, uint32_t thread) {
    transplant_getState(&c, thread, (uint64_t *) &state, DEVTEROFLEX_TOT_REGS);
    devteroflex_unpack_archstate(cpu, &state);
    qflex_singlestep(cpu);
    devteroflex_pack_archstate(&state, cpu);
    if(devteroflex_is_running()) {
        transplant_pushState(&c, thread, (uint64_t *) &state, DEVTEROFLEX_TOT_REGS);
        transplant_start(&c, thread);
    }
}

static void run_transplants(void) {
    uint32_t pending = 0;
    uint32_t thread = 0;
    uint32_t mask = 1;
    CPUState *cpu = first_cpu;
    transplant_pending(&c, &pending);
    if(pending) {
        while(cpu) {
            if(pending & mask) { 
                pending &= ~mask;
                run_transplant(cpu, thread);
            }
            mask<<=1;
            thread++;
            cpu = CPU_NEXT(cpu);
        }
    }
}

static void push_cpus(void) {
    CPUState *cpu;
    CPU_FOREACH(cpu) {
        registerThreadWithProcess(&c, cpu->cpu_index, QFLEX_GET_ARCH(asid)(cpu));
        devteroflex_pack_archstate(&state, cpu);
        transplant_pushState(&c, cpu->cpu_index, (uint64_t *) &state, DEVTEROFLEX_TOT_REGS);
        transplant_start(&c, cpu->cpu_index);
    }
}

static void pull_cpus(void) {
    CPUState *cpu;
    CPU_FOREACH(cpu) {
        // transplant_stop(c) // TODO: Enable mechanism to stop execution in the FPGA and request state back
        transplant_getState(&c, cpu->cpu_index, (uint64_t *) &state, DEVTEROFLEX_TOT_REGS);
        devteroflex_unpack_archstate(cpu, &state);
    }
}

int devteroflex_execution_flow(void) {
    initFPGAContext(&c);
    push_cpus();
    while(1) {
       // Check for pending transplants
       run_transplants();
       // Check for pending requests
       run_requests();
       if(!devteroflex_is_running()) {
           pull_cpus();
           break;
       }
    }

    releaseFPGAContext(&c);
    return 0;
}

int devteroflex_singlestepping_flow(void) {
    CPUState *cpu;
    qflex_update_exit_main_loop(false);
    qemu_log("DEVTEROFLEX: START\n");
    qflex_singlestep_start();
    qflex_update_skip_interrupts(true);
    CPU_FOREACH(cpu) {
        cpu_single_step(cpu, SSTEP_ENABLE | SSTEP_NOIRQ | SSTEP_NOTIMER);
        qatomic_mb_set(&cpu->exit_request, 0);
    }
    devteroflex_execution_flow();

    //while(devteroflex_is_running()) {
    //    CPU_FOREACH(cpu) {
    //        qflex_singlestep(cpu);
    //    }
    //}

    qflex_singlestep_stop();
    qflex_update_skip_interrupts(false);
    CPU_FOREACH(cpu) {
        cpu_single_step(cpu, 0);
    }
    qemu_log("DEVTEROFLEX: EXIT\n");
    return 0;
}