#pragma once
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <bitset>
#include <list>
#include <memory>
#include <cassert>
#include "Kernel.h"
#include "ExecContext.h"
#include "Warp.h"
#include "Compute.h"

// #include "inc/ExecContext.h"
using namespace std;
using namespace model_gpu;


class Instruction;

class ThreadItem
{
public:
    enum ThreadItemStatus {
        ThreadItemStatusActive = 0,
        ThreadItemStatusSuspend
    };




public:

    /// Create a thread item. grid object to create thread item
    ThreadItem(ThreadBlock* block,
             uint32_t threadIdX,
             uint32_t threadIdY,
             uint32_t threadIdZ,
             uint32_t threadId,
		     uint32_t warpId,
		     uint32_t laneId);

    virtual ~ThreadItem() {
    };

    uint32_t GetThreadIdX() const {return m_threadIdX;}
    uint32_t GetThreadIdY() const {return m_threadIdY;}
    uint32_t GetThreadIdZ() const {return m_threadIdZ;}


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

	/// Get global id
	unsigned GetGlobalId() const { return m_globalId; }
	unsigned GetLocalId() const { return m_threadId; }

	void SetGlobalId(uint32_t id) { this->m_globalId = id; }
	void SetLocalId(uint32_t id) { this->m_threadId = id; }

	unsigned GetGlobalId3D(uint32_t dim) const { return m_globalId_3d[dim]; }
	unsigned GetLocalId3D(uint32_t dim) const { return m_threadId_3d[dim]; }

	/// Return the wavefront that this work-items belongs to
	shared_ptr<Warp> GetWarp() const { return m_warp; }
	void SetWarp(shared_ptr<Warp> warp) { m_warp = warp; }

	unsigned ReadSReg(int sreg);
	void WriteSReg(int sreg, unsigned value);
	unsigned ReadVReg(int vreg);
	void WriteVReg(int vreg, unsigned value);
	unsigned ReadReg(int reg);
	void WriteBitmaskSReg(int sreg, unsigned value);
	int ReadBitmaskSReg(int sreg);
	void ReadBufferResource(int sreg, BufferDescriptor &buffer_desc);
	void ReadMemPtr(int sreg, MemoryPointer &memory_pointer);
	void ReadVRegMemPtr(int vreg, MemoryPointer &memory_pointer);

	void Execute(shared_ptr<Instruction> inst) ;

    bool is_done() { return m_thread_done; }

    address_type GetPC() { return m_PC;}
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

    std::list<stack_entry> m_callstack;
    address_type get_return_PC() { return m_callstack.back().m_PC; }

    bool is_leading_thread() { return m_leading_thread;}

    // unsigned m_icount;
    unsigned m_PC;
    unsigned m_NPC;
    unsigned m_RPC;
    bool m_RPC_updated;
    bool m_last_was_call;


private:
    ThreadBlock* m_block;
    shared_ptr<Warp> m_warp;	// Global memory
	// mem::Memory *global_mem = nullptr;

	// Local memory
	// mem::Memory *lds = nullptr;

	// Vector registers
	Register vreg[256];

    // The status of the thread item;
    ThreadItemStatus status = ThreadItemStatusActive;

public:
    int32_t m_threadIdX;
    int32_t m_threadIdY;
    int32_t m_threadIdZ;
    int32_t m_threadId;        // id in block
    int32_t m_warpId;
    int32_t m_laneId;
    int32_t m_globalId;

	int32_t m_threadId_3d[3];
	int32_t m_globalId_3d[3];

    bool m_thread_done;
    bool m_leading_thread = false;

    // address_type m_kernel_addr;
    // address_type m_kernel_args;

	/// Last global memory address
	unsigned global_memory_access_address = 0;

	/// Last global memory access size
	unsigned global_memory_access_size = 0;

    // shared_ptr<exec_context>   m_exec_context;
	int lds_access_count = 0;

	/// Maximum number of LDS accesses per instruction
	static const int MaxLdsAccessesPerInst = 2;

	/// Information for each lds access
	MemoryAccess lds_access[MaxLdsAccessesPerInst];
    void WriteLDS(uint32_t, uint32_t, char* value);
    void ReadLDS(uint32_t, uint32_t, char* value);

    void ReadMemory(uint32_t, uint32_t, char* value);
    void WriteMemory(uint32_t, uint32_t, char* value);

friend class Instruction;
};

