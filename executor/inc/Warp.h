#pragma once
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <bitset>
#include <list>
#include <memory>
#include <cassert>
// #include "inc/Kernel.h"
#include "inc/ExecContext.h"
// #include "inc/Compute.h"
#include "inc/IsaSim.h"

// #include "inc/ExecContext.h"
using namespace std;

class Instruction;
class ThreadBlock;
class ThreadItem;

  enum stack_entry_type { STACK_ENTRY_TYPE_NORMAL = 0, STACK_ENTRY_TYPE_CALL };
    struct SimtStack_entry {
        addr_t m_PC;
        uint32_t m_calldepth;
        simt_mask_t m_active_mask;
        addr_t m_recvg_pc;
        uint64_t m_branch_div_cycle;
        stack_entry_type m_type;
        SimtStack_entry()
            : m_PC(-1),
            m_calldepth(0),
            m_active_mask(),
            m_recvg_pc(-1),
            m_branch_div_cycle(0),
            m_type(STACK_ENTRY_TYPE_NORMAL){};
    };


class Warp
{

public:
    Warp(uint32_t warp_id, uint32_t warp_size, ThreadBlock *block)
        : m_warpId(warp_id)
        , m_thd_blk(block)
        , m_warp_size(warp_size) {
            reset();
        }

    ~Warp() {};

    void reset();

    void launch(addr_t start_pc, const simt_mask_t &active_mask) {
        reset();
        SimtStack_entry new_stack_entry;
        new_stack_entry.m_PC = start_pc;
        new_stack_entry.m_calldepth = 1;
        new_stack_entry.m_active_mask = active_mask;
        new_stack_entry.m_type = STACK_ENTRY_TYPE_NORMAL;
        m_stack.push_back(new_stack_entry);
    }

    void update(simt_mask_t &thread_done, std::vector<addr_t> &next_pc,
              addr_t recvg_pc, op_type_t next_inst_op,
              unsigned next_inst_size, addr_t next_inst_pc);

    const simt_mask_t &get_simt_active_mask() const {
        assert(m_stack.size() > 0);
        return m_stack.back().m_active_mask;
    }

    void get_pdom_stack_top_info(addr_t *pc, addr_t *rpc) {
        assert(m_stack.size() > 0);
        *pc = m_stack.back().m_PC;
        *rpc = m_stack.back().m_recvg_pc;
    }

    unsigned get_rp() const {
        assert(m_stack.size() > 0);
        return m_stack.back().m_recvg_pc;
    }
/*
    void ActivateAllThreadItems();
    */

    void AddThreadItem(ThreadItem* item);
    uint32_t GetWarpId() { return m_warpId; }
private:
    uint32_t m_warpId;

    // The thread group it belongs to
    ThreadBlock *m_thd_blk;
    std::vector<ThreadItem*> m_items;

    std::deque<SimtStack_entry> m_stack;

  public:

    void do_atomic(bool forceDo = false);
    void do_atomic(const active_mask_t &access_mask, bool forceDo = false);
    void clear() { m_empty = true; }

    void issue(const active_mask_t &mask, unsigned warp_id, unsigned long long cycle, int dynamic_warp_id, int sch_id);



    void completed(unsigned long long cycle) const;  // stat collection: called when the instruction is completed
    void add_callback(unsigned lane_id,
                    void (*function)(const class Instruction *,
                                     class ptx_thread_info *),
                    const Instruction *inst, class ptx_thread_info *thread,
                    bool atomic) {
        if (!m_per_scalar_thread_valid) {
            m_items.resize(MAX_WARPSIZE);
            m_per_scalar_thread_valid = true;
            if (atomic) m_isatomic = true;
        }
        // m_items[lane_id].callback.function = function;
        // m_items[lane_id].callback.instruction = inst;
        // m_items[lane_id].callback.thread = thread;
    }

    void set_warp_active_mask(const active_mask_t &active, bool isatomic);
    void update_warp_active_mask(bool isatomic_op) {
        set_warp_active_mask(get_simt_active_mask(), isatomic_op);
    };

    void clear_active(const active_mask_t &inactive);
    void set_not_active(unsigned lane_id);

    // bool active(uint32_t laneId) const { return m_warp_active_mask.test(laneId); }
    unsigned active_count() const { return m_warp_active_mask.count(); }

    unsigned issued_count() const {
        assert(m_empty == false);
        return m_warp_issued_mask.count();
    }  // for instruction counting

    bool isatomic() const { return m_isatomic; }
    active_mask_t &get_warp_active_mask() { return m_warp_active_mask; }
    unsigned get_schd_id() const { return m_scheduler_id; }
  protected:

  struct per_item_info {
    per_item_info() {
      // for (unsigned i = 0; i < MAX_ACCESSES_PER_INSN_PER_THREAD; i++)
        // memreqaddr[i] = 0;
    }
    /*
    dram_callback_t callback;
    new_addr_type memreqaddr[MAX_ACCESSES_PER_INSN_PER_THREAD];  // effective address,
                                                       // upto 8 different
                                                       // requests (to support
                                                       // 32B access in 8 chunks
                                                       // of 4B each)
                                                       */
  };

    bool m_empty = true;
    bool m_isatomic = false;
    bool m_per_scalar_thread_valid = false;
    bool m_mem_accesses_created = false;
    bool should_do_atomic = true;
    uint32_t m_scheduler_id;
    uint32_t m_warp_size;
    uint32_t m_branch_div_cycle = 100;

    active_mask_t m_warp_active_mask;  // dynamic active mask for timing model
                                     // (after predication)
    active_mask_t m_warp_issued_mask;  // active mask at issue (prior to predication test)
                           // -- for instruction counting
                           //
  public:
	// void setSReg(int sreg, unsigned value);
	unsigned getSregUint(int sreg_id) const;
	void setSregUint(int id, unsigned int value);

    addr_t GetWarpPC() { return m_PC;}
	void SetWarpPC(unsigned pc) { m_PC = pc; }
	void IncWarpPC(int increment) { m_PC += increment; }

	void SetBarrierInstruction(bool barrier_instruction) { this->barrier_instruction = barrier_instruction; }
	bool isBarrierInstruction() const { return barrier_instruction; }

	void SetMemoryWait(bool mem_wait) { this->memory_wait = mem_wait; }
	void SetAtBarrier(bool at_barrier) { this->at_barrier = at_barrier; }
	void SetFinished(bool finished) { this->finished = finished; }

	void SetScalarMemoryRead(bool scalar_memory_read)
	{
		this->scalar_memory_read = scalar_memory_read;
	}

	/// Flag set during instruction emulation.
	void SetVectorMemoryGlobalCoherency(bool vector_mem_global_coherency)
	{
		this->vector_memory_global_coherency = vector_mem_global_coherency;
	}


    unsigned m_PC;

	Register sreg[256];

	bool vector_memory_read = false;   // vecor memory write
	bool vector_memory_write = false;   // vecor memory write
	bool vector_memory_atomic = false;  // vector meory read
	bool scalar_memory_read = false;    // scalar memory read
	bool ds_read = false;       // wait instruction
	bool ds_wait = false;       // wait instruction
	bool memory_wait = false;   // wai instruction
	bool at_barrier = false;    // barrier instruction
	bool finished = false;      // warp is done
	bool vector_memory_global_coherency = false;
	bool barrier_instruction = false;

};


