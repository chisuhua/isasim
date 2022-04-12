#pragma once
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <bitset>
#include <list>
#include <memory>
#include <cassert>
//#include "Kernel.h"
#include "ExecContext.h"
#include "Warp.h"
//#include "../../libcuda/abstract_hardware_model.h"
//#include "Compute.h"

// #include "inc/ExecContext.h"
using namespace std;
//using namespace libcuda;

class Instruction;
class ThreadBlock;
class cta_info_t;
namespace libcuda {
class gpgpu_t;
class memory_space;
class memory_space_t;
class dram_callback_t;
}

#if !defined(__VECTOR_TYPES_H__)
#include "vector_types.h"
/*
struct dim3 {
   unsigned int x, y, z;
};
*/
#endif


class ThreadItem
{
public:
	ThreadItem() {
        m_barrier_num = -1;
        m_at_barrier = false;
        m_valid = false;
        m_thread_done = false;
        m_PC = 0;
        m_branch_taken = 0;
        m_shared_mem = NULL;
        m_sstarr_mem = NULL;
        m_warp = NULL;
        m_local_mem = NULL;
        m_RPC = -1;
        m_RPC_updated = false;
    };

    enum ThreadItemStatus {
        ThreadItemStatusActive = 0,
        ThreadItemStatusSuspend
    };

    /// Create a thread item. grid object to create thread item
    void init(libcuda::gpgpu_t *gpu, ThreadBlock* block,
             uint32_t sid,
             uint32_t cta_id,
             uint32_t wid,
             uint32_t tid) {
        m_gpu = gpu;
        m_block = block;
        m_hw_sid = sid;
        m_hw_ctaid = cta_id;
        m_hw_wid = wid;
        m_hw_tid = tid;
		status = ThreadItemStatusActive;
    }

    virtual ~ThreadItem() { };

    uint32_t GetThreadIdX() const {return m_tid.x;}
    uint32_t GetThreadIdY() const {return m_tid.y;}
    uint32_t GetThreadIdZ() const {return m_tid.z;}


    /// Return the status of the thread item
    ThreadItemStatus getStatus() const { return status; }

    /// Set the status of the thread item
    void setStatus(ThreadItemStatus status) {
        this->status = status;
    }

    /// Return the thread group that this threaditem is in
    ThreadBlock* GetBlock() const {
        return m_block;
    }

    // Register GetThreadVreg() const {
    //     // return m_exec_context;
    // }

	unsigned get_uid() const {
		return GetGlobalId();
	}

	/// Get global id
	unsigned GetGlobalId() const {
		return m_tid.x + m_tid.y * m_ntid.x + \
			   m_tid.z * m_ntid.x * m_ntid.y;
	}

	unsigned GetLocalId() const { return m_threadId; }

	void SetGlobalId(uint32_t id) { this->m_globalId = id; }
	void SetLocalId(uint32_t id) { this->m_threadId = id; }

	unsigned GetGlobalId3D(uint32_t dim) const { return m_globalId_3d[dim]; }
	unsigned GetLocalId3D(uint32_t dim) const { return m_threadId_3d[dim]; }

	/// Return the wavefront that this work-items belongs to
	Warp* GetWarp() const { return m_warp; }
	void SetWarp(Warp* warp) { m_warp = warp; }

    std::shared_ptr<Instruction> getInstruction(addr_t );

	// unsigned ReadSReg(int sreg);
	// void WriteSReg(int sreg, unsigned value);
	// unsigned ReadVReg(int vreg);
	// void WriteVReg(int vreg, unsigned value);
	// unsigned ReadReg(int reg);
	// void WriteBitmaskSReg(int sreg, unsigned value);
	// int ReadBitmaskSReg(int sreg);
	// void ReadBufferResource(int sreg, BufferDescriptor &buffer_desc);
	// void ReadMemPtr(int sreg, MemoryPointer &memory_pointer);
	// void ReadVRegMemPtr(int vreg, MemoryPointer &memory_pointer);

    void Execute(std::shared_ptr<Instruction> inst, WarpState *warp_state);
	void print_insn(addr_t pc, FILE *fp);

    bool is_done() { return m_thread_done; }

    addr_t get_pc() { return m_PC;}
    void set_npc(unsigned npc) { m_NPC = npc; }
    void update_pc() { m_PC = m_NPC; }
    bool rpc_updated() const { return m_RPC_updated; }
    bool last_was_call() const { return m_last_was_call; }

    unsigned next_instr() {
        // m_icount++;
        // m_branch_taken = false;
        return m_PC;
    }

    void clearRPC() {
        m_RPC = -1;
        m_RPC_updated = false;
        m_last_was_call = false;
    }

    std::list<SimtStack_entry> m_callstack;
    addr_t get_return_PC() { return m_callstack.back().m_PC; }

    bool is_leading_thread() { return m_leading_thread;}

    // unsigned m_icount;
    unsigned m_PC;
    unsigned m_NPC;
    unsigned m_RPC;
    bool m_RPC_updated;
    bool m_last_was_call;

  int m_barrier_num;
  bool m_at_barrier;

  unsigned get_local_mem_stack_pointer() const {
    return m_local_mem_stack_pointer;
  }

  libcuda::memory_space *get_global_memory();
  libcuda::memory_space *get_tex_memory();
  libcuda::memory_space *get_surf_memory();
  //FIXME memory_space *get_param_memory() { return m_kernel.get_param_memory(); }

  void exitCore() {
    // m_core is not used in case of functional simulation mode
    // if (!m_functionalSimulationMode) m_core->warp_exit(m_hw_wid);
  }

  void set_done() {
    assert(!m_at_barrier);
    m_thread_done = true;
  }

  void registerExit();
  unsigned get_reduction_value(unsigned ctaid, unsigned barid) ;
  void and_reduction(unsigned ctaid, unsigned barid, bool value);
  void or_reduction(unsigned ctaid, unsigned barid, bool value);
  void popc_reduction(unsigned ctaid, unsigned barid, bool value);

  libcuda::gpgpu_t *m_gpu;
  ThreadBlock* m_block;
  Warp* m_warp;	// Global memory
  // mem::Memory *global_mem = nullptr;
  cta_info_t *m_cta_info;
  unsigned m_local_mem_stack_pointer;

private:
  // Local memory
  // mem::Memory *lds = nullptr;

  // Vector registers
  // Register vreg[256];

  // The status of the thread item;
  ThreadItemStatus status = ThreadItemStatusActive;

public:
  addr_t m_last_effective_address;
  bool m_branch_taken;
  libcuda::memory_space_t *m_last_memory_space;
  libcuda::dram_callback_t *m_last_dram_callback;
  libcuda::memory_space *m_shared_mem;
  libcuda::memory_space *m_sstarr_mem;
  libcuda::memory_space *m_local_mem;

  dim3 get_ctaid() const { return m_ctaid; }
  dim3 get_tid() const { return m_tid; }
  dim3 get_ntid() const { return m_ntid; }
  void set_tid(dim3 tid) { m_tid = tid; }
  void cpy_tid_to_reg(dim3 tid);
  void set_ctaid(dim3 ctaid) { m_ctaid = ctaid; }
  void set_ntid(dim3 tid) { m_ntid = tid; }
  void set_nctaid(dim3 cta_size) { m_nctaid = cta_size; }
  void set_laneid(uint32_t laneid) { m_laneId = laneid; }

  unsigned get_hw_tid() const { return m_hw_tid; }
  unsigned get_hw_ctaid() const { return m_hw_ctaid; }
  unsigned get_hw_wid() const { return m_hw_wid; }
  unsigned get_hw_sid() const { return m_hw_sid; }

  void set_valid() { m_valid = true; }

public:
  dim3 m_ntid;
  dim3 m_tid;
  dim3 m_nctaid;
  dim3 m_ctaid;

  unsigned m_gridid;
  bool m_thread_done;
  unsigned m_hw_sid;
  unsigned m_hw_tid;
  unsigned m_hw_wid;
  unsigned m_hw_ctaid;

  bool m_valid;


    int32_t m_threadIdX;
    int32_t m_threadIdY;
    int32_t m_threadIdZ;
    int32_t m_threadId;        // id in block
    int32_t m_warpId;
    int32_t m_laneId;
    int32_t m_globalId;

	int32_t m_threadId_3d[3];
	int32_t m_globalId_3d[3];

    bool m_leading_thread = false;

    // addr_t m_kernel_addr;
    // addr_t m_kernel_args;

	/// Last global memory address
	unsigned global_memory_access_address = 0;

	/// Last global memory access size
	unsigned global_memory_access_size = 0;

    // shared_ptr<exec_context>   m_exec_context;
	int lds_access_count = 0;


	/// Maximum number of LDS accesses per instruction
	static const int MaxLdsAccessesPerInst = 2;

	/// Information for each lds access
	// MemoryAccess lds_access[MaxLdsAccessesPerInst];
    // void WriteLDS(uint32_t, uint32_t, char* value);
    // void ReadLDS(uint32_t, uint32_t, char* value);

    // void ReadMemory(uint32_t, uint32_t, char* value);
    // void WriteMemory(uint32_t, uint32_t, char* value);

friend class Instruction;
};

