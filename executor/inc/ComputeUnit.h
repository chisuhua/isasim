#pragma once
#include "inc/Module.h"
#include "inc/MsgQueue.h"
#include "inc/Engine.h"

#include "inc/Compute.h"
#include "inc/Kernel.h"

#include <vector>

namespace model_gpu {
    class ThreadBlock;
}

namespace model {

enum class CU_REQ_TYPE {
    REG   = 0,
    DISP  = 1
};

enum CU_RESP_TYPE {
    BLOCK_DONE  = 0
};

class CuDispatchPacket : public Packet {
    public:
    PacketType GetType() {
        return PacketType::CuDispatch;
    }

    uint32_t disp_id;
    DispatchInfo disp_info;
};

class CuReqMessage : public Message {
    public:
        CuReqMessage(shared_ptr<Packet> pkt, shared_ptr<Message> ref_msg = nullptr)
            : Message(MessageType::CU_REQ, pkt, ref_msg)
        {};

        ~CuReqMessage() {};

        shared_ptr<CuDispatchPacket> GetDispatchPacket() {
            return static_pointer_cast<CuDispatchPacket>(this->m_pkt);
        }
};


/*
struct CuDispDonePacket : public Packet{
    uint32_t disp_id;
    uint32_t block_id;
    uint32_t cu_id;
};
*/

class ComputeUnit : public Module {
    public:
        ComputeUnit(Engine* engine, const std::string &instance_name = "")
            : Module(engine, "cu", instance_name)
        {
            m_blocks.resize(m_max_blocks);
            for (uint32_t i = 0; i < m_max_blocks; i++) {
                m_blocks[i] = nullptr;
            }
        };
        virtual ~ComputeUnit() {};

        // virtual void FetchInput();

        virtual bool Execute();

        void FetchInstruction(uint32_t addr, void* data, uint32_t size);

        void WriteMemory(uint32_t hw_blk_id, uint32_t warp_id, uint32_t addr, void* data, uint32_t size);
        void ReadMemory(uint32_t hw_blk_id, uint32_t warp_id, uint32_t addr, void* data, uint32_t size);


    private:
        // void Reset();
        // bool Busy();
        // void WriteRegister(uint32_t offset, uint32_t value);
        // void ReadRegister(uint32_t offset, uint32_t* value);

        void ProcessDispatch(MsgPtr msg);
        void ProcessRegAccess(MsgPtr msg);
        void ProcessRespMsg(MsgPtr msg);


    private:
        // vector<CommandPipe> m_cmd_queue;
        vector<shared_ptr<model_gpu::ThreadBlock>> m_blocks;
        uint32_t m_max_blocks = 4;

        struct msg_status_t {
            uint32_t hw_blk_id;
            uint32_t warp_id;
            bool is_read;
        };


        std::map<MsgPtr, msg_status_t> m_msg_status_list;
        std::map<MsgPtr, uint32_t> m_msg_disp_list;

    private:
	    // std::list<std::shared_ptr<ThreadGrid>> grids_;
        // std::function<RootFunc*(void)> m_root_function;
	    // std::list<queue_t*> queues_;
        // aco_t* main_co_;
};
}
