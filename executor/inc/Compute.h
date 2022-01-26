#pragma once
#include "Kernel.h"
#include "ExecContext.h"
#include <map>
#include <memory>


// #include "inc/ExecContext.h"
using namespace std;

class Instruction;
class ThreadItem;
class Warp;

namespace model {
class ComputeUnit;
};

using namespace model;

namespace model_gpu {

class ThreadBlock /*: public enable_shared_from_this<ThreadBlock>*/ {
    public:
        ThreadBlock(std::shared_ptr<DispatchInfo> disp_info, uint32_t warp_size, uint32_t blk_threads, ComputeUnit *cu, uint32_t hw_blk_id)
            : m_disp_info(disp_info)
            , m_blk_threads(blk_threads)
            , m_warp_size(warp_size)
            , m_cu(cu)
            , m_hw_blk_id(hw_blk_id)
        {
            m_warp_count = 0;

            for (unsigned i = 0; i < MAX_CTA_PER_SHADER; i++) {
                for (unsigned j = 0; j < MAX_BARRIERS_PER_CTA; j++) {
                    reduction_storage[i][j] = 0;
                }
            }

        }

        ~ThreadBlock() {
            warp_exit(0);
            delete[] m_liveThreadCount;
            delete[] m_warpAtBarrier;
        };

        void Execute();
        void warp_exit(unsigned warp_id);
        /*
        virtual bool warp_waiting_at_barrier(unsigned warp_id) const {
            return (m_warpAtBarrier[warp_id] || !(m_liveThreadCount[warp_id] > 0));
        }*/

        bool thread_done(unsigned hw_thread_id) ;
        // void HitBarrier(uint32_t abs_flat_id);

        // void ActivateAllThreadItems();
        void get_pdom_stack_top_info(unsigned warpId, address_type *pc, address_type *rpc) ;

        void initializeCTA();

        void executeInstruction(std::shared_ptr<Instruction> inst, unsigned warpId) ;
        std::shared_ptr<Instruction> GetInstruction(address_type pc);

    private:
        std::shared_ptr<Instruction> GetExecuteWarp(unsigned warpId, address_type &pc);
        void executeWarp(unsigned, bool &, bool &);
        // initializes threads in the CTA block which we are executing
        void checkExecutionStatusAndUpdate(std::shared_ptr<Instruction> inst, unsigned t, unsigned tid) ;
        void updateSIMTStack(unsigned warpId, std::shared_ptr<Instruction> inst, address_type pc);

        void createThread(uint32_t threadIdX, uint32_t threadIdY,
            uint32_t threadIdZ, uint32_t threadId, uint32_t warpId, uint32_t laneId);
        // lunches the stack and set the threads count
        void createWarp(unsigned warpId);
        void launchWarp(unsigned warpId, std::shared_ptr<DispatchInfo> disp_info);
        std::shared_ptr<Warp> GetWarp(unsigned warpId);

        std::shared_ptr<DispatchInfo> m_disp_info;

        // each warp live thread count and barrier indicator
        unsigned *m_liveThreadCount;
        bool *m_warpAtBarrier;

        // List of warps, map warp front id to warp fronts
        // warp contain the simt_stack
        std::map<uint32_t, std::shared_ptr<Warp>> m_warps;
        std::map<uint32_t, std::shared_ptr<ThreadItem>> m_threads;
        std::map<address_type, std::shared_ptr<Instruction>> m_insts;

        std::vector<uint32_t> m_warpReadCount;
        std::vector<uint32_t> m_warpWriteCount;

        // uint32_t m_thread_items_num;
        uint32_t m_blk_threads;
        uint32_t m_warp_size;
        uint32_t m_warp_count;
        unsigned reduction_storage[MAX_CTA_PER_SHADER][MAX_BARRIERS_PER_CTA];

        address_type m_kernel_addr;
        address_type m_kernel_args;

    public:
	    unsigned GetWarpsCount() const { return m_warp_count; }
	    unsigned GetWarpsAtBarrier() const { return warps_at_barrier; }
	    unsigned GetLocalMemBase() const { return m_lds_mem_base; }

	    void IncWarpsAtBarrier() { warps_at_barrier++; }
	    void SetWarpsAtBarrier(unsigned counter) { warps_at_barrier = counter; }

        void WriteMemory(uint32_t warpId, uint32_t addr, void* data, uint32_t size);
        void ReadMemory(uint32_t warpId, uint32_t addr, void* data, uint32_t size);

        void ReadMemoryResp(uint32_t warpId);
        void WriteMemoryResp(uint32_t warpId);

        std::map<uint32_t, std::shared_ptr<Warp>>::iterator WarpsBegin();
        std::map<uint32_t, std::shared_ptr<Warp>>::iterator WarpsEnd();

	    unsigned warps_at_barrier = 0;      // number of warps suspended at barrier
	    bool finished = false;      // tb is finished

        uint32_t m_lds_mem_base;
        ComputeUnit *m_cu;
        uint32_t m_hw_blk_id;
    friend class Instruction;
};

}
