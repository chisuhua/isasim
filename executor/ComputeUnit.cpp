#include "inc/ComputeUnit.h"
#include "inc/Message.h"
#include "inc/Memory.h"
#include <thread>

using namespace model_gpu;
using namespace model;

bool ComputeUnit::Execute() {

    bool busy = Module::Execute();

    // for (auto& msg : m_msg_active_list ) {
    for (auto msg = m_msg_active_list.begin(); msg != m_msg_active_list.end(); ) {
        switch ((*msg)->GetMessageType()) {
            case MessageType::CU_REQ: {
                 ProcessDispatch(*msg);
                 msg = m_msg_active_list.erase(msg);
                 break;
            }
            case MessageType::CU_RESP: {
                 msg++;
                 break;
            }
            default: {
                 msg++;
                 break;
            }
        }
        busy = true;
    }

    for (auto itr = m_msg_disp_list.begin(); itr != m_msg_disp_list.end();) {
        uint32_t hw_blk_id = (*itr).second;
        if (m_blocks[hw_blk_id]->finished) {
            PushResp((*itr).first, make_shared<RespDoneMessage>((*itr).first));
            m_blocks[hw_blk_id] = nullptr;
            itr = m_msg_disp_list.erase(itr);
        } else {
            itr++;
        }
        busy = true;
    }
    return busy;
}

void ComputeUnit::ProcessRegAccess(MsgPtr msg) {
}


void ComputeUnit::ProcessDispatch(MsgPtr msg) {
    auto cu_msg = static_pointer_cast<CuReqMessage>(msg);
    auto pkt = cu_msg->GetDispatchPacket();
    shared_ptr<DispatchInfo> disp_info = make_shared<DispatchInfo>(pkt->disp_info);
    uint32_t blk_threads = disp_info->blockDimX * disp_info->blockDimY * disp_info->blockDimZ;

    for (uint32_t i = 0; i < m_max_blocks; i++) {
        if (m_blocks[i] == nullptr) {
            m_blocks[i] = make_shared<ThreadBlock>(disp_info, 32, blk_threads, this, i);
            m_blocks[i]->initializeCTA();
            std::thread t(&ThreadBlock::Execute, m_blocks[i]);
            // TODO m_blocks[i]->Execute();
            // m_blocks[i] = nullptr;
            // TODO use lambda to pass it to thread_block execute function
            t.detach();
            m_msg_disp_list.insert(std::make_pair(msg, i));

            break;
        }
    }
}

void ComputeUnit::FetchInstruction(uint32_t addr, void* data, uint32_t size) {
    std::string block = "mc";
    auto pkt = make_shared<MemReadPacket>();
    pkt->addr = addr;
    pkt->is_pa = false;
    pkt->data = data;
    pkt->length = size;

    auto msg = make_shared<MemAccessMessage>(pkt);
    PushReq("mc", msg);

    m_response_queue->Push(msg);

    while(!msg->IsRespDone()) {};
}

void ComputeUnit::ReadMemory(uint32_t hw_blk_id, uint32_t warp_id,  uint32_t addr, void* data, uint32_t size) {
    std::string block = "mc";
    auto pkt = make_shared<MemReadPacket>();
    pkt->addr = addr;
    pkt->is_pa = false;
    pkt->data = data;
    pkt->length = size;

    auto msg = make_shared<MemAccessMessage>(pkt);
    PushReq("mc", msg);

    msg_status_t msg_status {hw_blk_id, warp_id, true};

    m_msg_status_list.insert(std::make_pair(msg, msg_status));

    m_response_queue->Push(msg);

    while(!msg->IsRespDone()) {};
}

void ComputeUnit::WriteMemory(uint32_t hw_blk_id, uint32_t warp_id, uint32_t addr, void* data, uint32_t size) {

    std::string block = "mc";
    auto pkt = make_shared<MemWritePacket>();
    pkt->addr = addr;
    pkt->is_pa = false;
    pkt->data = data;
    pkt->length = size;

    auto msg = make_shared<MemAccessMessage>(pkt);
    PushReq("mc", msg);

    msg_status_t msg_status {hw_blk_id, warp_id, true};
    m_msg_status_list.insert(std::make_pair(msg, msg_status));

    m_response_queue->Push(msg);

    while(!msg->IsRespDone()) {};
};

void ComputeUnit::ProcessRespMsg(MsgPtr msg)  {

    auto itr = m_msg_status_list.find(msg);

    // FetchInstruction don't check the status
    if (itr != m_msg_status_list.end())
    {
        return;
    }
    uint32_t hw_blk_id = itr->second.hw_blk_id;
    uint32_t warp_id = itr->second.warp_id;
    bool is_read = itr->second.is_read;

    if (is_read) {
        m_blocks[hw_blk_id]->ReadMemoryResp(warp_id);
    } else {
        m_blocks[hw_blk_id]->WriteMemoryResp(warp_id);
    }
}
