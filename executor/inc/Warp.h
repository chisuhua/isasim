#pragma once
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <bitset>
#include <list>
#include <memory>
#include <cassert>
#include "inc/Kernel.h"
#include "inc/ExecContext.h"
#include "inc/Compute.h"

// #include "inc/ExecContext.h"
using namespace std;
using namespace model_gpu;


class Instruction;


typedef uint32_t symbol;
typedef std::map<uint32_t, uint64_t> symbol_table;

struct stack_entry {
  stack_entry() {
    m_symbol_table = NULL;
    m_PC = 0;
    m_RPC = -1;
    m_return_var_src = NULL;
    m_return_var_dst = NULL;
    m_call_uid = 0;
    m_valid = false;
  }
  stack_entry(symbol_table *s, uint32_t pc, uint32_t rpc,
              const symbol *return_var_src, const symbol *return_var_dst,
              unsigned call_uid) {
    m_symbol_table = s;
    m_PC = pc;
    m_RPC = rpc;
    m_return_var_src = return_var_src;
    m_return_var_dst = return_var_dst;
    m_call_uid = call_uid;
    m_valid = true;
  }

  bool m_valid;
  symbol_table *m_symbol_table;
  unsigned m_PC;
  unsigned m_RPC;
  const symbol *m_return_var_src;
  const symbol *m_return_var_dst;
  unsigned m_call_uid;
};



class Warp
{
  enum stack_entry_type { STACK_ENTRY_TYPE_NORMAL = 0, STACK_ENTRY_TYPE_CALL };
    struct simt_stack_entry {
        address_type m_pc;
        uint32_t m_calldepth;
        simt_mask_t m_active_mask;
        address_type m_recvg_pc;
        uint64_t m_branch_div_cycle;
        stack_entry_type m_type;
        simt_stack_entry()
            : m_pc(-1),
            m_calldepth(0),
            m_active_mask(),
            m_recvg_pc(-1),
            m_branch_div_cycle(0),
            m_type(STACK_ENTRY_TYPE_NORMAL){};
    };

public:
    Warp(uint32_t warp_id, uint32_t warp_size, ThreadBlock *block)
        : m_warpId(warp_id)
        , m_thd_blk(block)
        , m_warp_size(warp_size) {
            reset();
        }

    ~Warp() {};

    void reset();

    void launch(address_type start_pc, const simt_mask_t &active_mask) {
        reset();
        simt_stack_entry new_stack_entry;
        new_stack_entry.m_pc = start_pc;
        new_stack_entry.m_calldepth = 1;
        new_stack_entry.m_active_mask = active_mask;
        new_stack_entry.m_type = STACK_ENTRY_TYPE_NORMAL;
        m_stack.push_back(new_stack_entry);
    }

    void update(simt_mask_t &thread_done, addr_vector_t &next_pc,
              address_type recvg_pc, OpType next_inst_op,
              unsigned next_inst_size, address_type next_inst_pc);

    const simt_mask_t &get_simt_active_mask() const {
        assert(m_stack.size() > 0);
        return m_stack.back().m_active_mask;
    }

    void get_pdom_stack_top_info(address_type *pc, address_type *rpc) {
        assert(m_stack.size() > 0);
        *pc = m_stack.back().m_pc;
        *rpc = m_stack.back().m_recvg_pc;
    }

    unsigned get_rp() const {
        assert(m_stack.size() > 0);
        return m_stack.back().m_recvg_pc;
    }
/*
    void ActivateAllThreadItems();
    */

    void AddThreadItem(std::shared_ptr<ThreadItem> item);
    uint32_t GetWarpId() { return m_warpId; }
private:
    uint32_t m_warpId;

    // The thread group it belongs to
    ThreadBlock *m_thd_blk;
    std::vector<std::shared_ptr<ThreadItem>> m_items;

    std::deque<simt_stack_entry> m_stack;

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
            m_items.resize(MAX_WARP_SIZE);
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

    address_type GetWarpPC() { return m_PC;}
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


