#include "inc/Compute.h"
#include "inc/ThreadItem.h"
#include "inc/Warp.h"
#include "inc/Instruction.h"
#include "inc/Memory.h"
#include "inc/Module.h"
#include "inc/ComputeUnit.h"
#include "common/intmath.h"
#include "common/utils.h"

using namespace model_gpu;

void ThreadBlock::initializeCTA() {
    uint32_t threadId = 0;
    uint32_t warpId = 0;
    uint32_t laneId = 0;

    m_kernel_addr = (uint64_t)m_disp_info->kernel_addr;
    m_kernel_args = (uint64_t)m_disp_info->kernel_args;

    for (uint32_t threadIdZ = 0;  threadIdZ < m_disp_info->blockDimZ; threadIdZ++) {
        for (uint32_t threadIdY = 0;  threadIdY < m_disp_info->blockDimY; threadIdY++) {
            for (uint32_t threadIdX = 0;  threadIdX < m_disp_info->blockDimX; threadIdX++) {
                threadId = threadIdX +
                           threadIdY * m_disp_info->blockDimX +
                           threadIdZ * m_disp_info->blockDimX * m_disp_info->blockDimY;
                warpId = threadId / m_warp_size;
                laneId = threadId % m_warp_size;

                createThread(threadIdX, threadIdY, threadIdZ, threadId, warpId, laneId);
                if (laneId == 0) createWarp(warpId);
            }
        }
    }
                // warp->AddThreadItem(item);
                // item->SetWarp(warp);

    m_warpAtBarrier = new bool[m_warp_count];
    m_liveThreadCount = new unsigned[m_warp_count];
    m_warpReadCount.resize(m_warp_count);
    m_warpWriteCount.resize(m_warp_count);

    assert(m_warp_count * m_warp_size > 0);
    for (uint32_t i = 0; i < m_warp_count; i++) {
        m_liveThreadCount[i] = 0;
        m_warpAtBarrier[i] = false;
        m_warpReadCount[i] = 0;
        m_warpWriteCount[i] = 0;
        launchWarp(i, m_disp_info);
    }
}

void ThreadBlock::createThread(uint32_t threadIdX, uint32_t threadIdY,
        uint32_t threadIdZ, uint32_t threadId, uint32_t warpId, uint32_t laneId) {
    m_threads[threadId] = make_shared<ThreadItem>(this,
         threadIdX, threadIdY, threadIdZ, threadId, warpId, laneId);
}

void ThreadBlock::createWarp(unsigned warpId) {
	auto it = m_warps.find(warpId);
	assert(it == m_warps.end());
    m_warps[warpId] = make_shared<Warp>(warpId, m_warp_size, this);
    m_warp_count++;
}

void ThreadBlock::launchWarp(unsigned warpId, shared_ptr<DispatchInfo> disp_info) {
    simt_mask_t initialMask;
    initialMask.set();
    int liveThreadsCount = 0;

    uint32_t sreg_offset = 0;
    void* kernel_args = (void*)disp_info->kernel_args;
    m_warps[warpId]->setSregUint(sreg_offset++, PtrLow32(kernel_args));
    m_warps[warpId]->setSregUint(sreg_offset++, PtrHigh32(kernel_args));
    for (uint32_t i = 0; i < disp_info->kernel_ctrl.bits.user_sreg_num; i++) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->userSreg[i]);
    }
    if (disp_info->kernel_ctrl.bits.grid_dim_x_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->gridDimX);
    }
    if (disp_info->kernel_ctrl.bits.grid_dim_y_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->gridDimY);
    }
    if (disp_info->kernel_ctrl.bits.grid_dim_z_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->gridDimZ);
    }
    sreg_offset = (sreg_offset + 1) & (~0x1);
    if (disp_info->kernel_ctrl.bits.block_dim_x_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockDimX);
    }
    if (disp_info->kernel_ctrl.bits.block_dim_y_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockDimY);
    }
    if (disp_info->kernel_ctrl.bits.block_dim_z_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockDimZ);
    }
    if (disp_info->kernel_ctrl.bits.block_idx_x_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockIdX);
    }
    if (disp_info->kernel_ctrl.bits.block_idx_y_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockIdY);
    }
    if (disp_info->kernel_ctrl.bits.block_idx_z_en) {
        m_warps[warpId]->setSregUint(sreg_offset++, disp_info->blockIdZ);
    }

    for (uint32_t i = warpId * m_warp_size; i < warpId * m_warp_size + m_warp_size; i++) {
        if (m_threads.find(i) != m_threads.end()) {
            m_threads[i]->WriteVReg(0, m_threads[i]->m_threadIdX);
            m_threads[i]->WriteVReg(1, m_threads[i]->m_threadIdY);
            m_threads[i]->WriteVReg(2, m_threads[i]->m_threadIdZ);
            m_warps[warpId]->AddThreadItem(m_threads[i]);
            m_threads[i]->SetWarp(m_warps[warpId]);
            liveThreadsCount++;
        } else {
            initialMask.reset(i - warpId * m_warp_size);
        }
    }

    assert(m_threads[warpId * m_warp_size] != NULL);
    m_warps[warpId]->launch(m_threads[warpId * m_warp_size]->GetPC(), initialMask);
    m_liveThreadCount[warpId] = liveThreadsCount;
    // return m_warps[warpId];
}

shared_ptr<Warp> ThreadBlock::GetWarp(unsigned warpId) {
    return m_warps[warpId];
}

void ThreadBlock::Execute() {
    int count = 0;
    while (true) {
        bool someOneLive = false;
        bool allAtBarrier = true;
        for (unsigned i = 0; i < m_warp_count; i++) {
            executeWarp(i, allAtBarrier, someOneLive);
            count++;
        }
        /*
        if (inst_count > 0 && count > inst_count) {
            someOneLive = false;
            break;
        }
        */
        if (!someOneLive) break;
        if (allAtBarrier) {
            for (unsigned i = 0; i < m_warp_count; i++) m_warpAtBarrier[i] = false;
        }
    }
    finished = true;
}

void ThreadBlock::executeWarp(unsigned warpId, bool &allAtBarrier,
                                    bool &someOneLive) {
  if (!m_warpAtBarrier[warpId] && m_liveThreadCount[warpId] != 0) {
    address_type warp_pc;
    shared_ptr<Instruction> inst = GetExecuteWarp(warpId, warp_pc);
	// Instruction::Opcode opcode = inst->getOpcode();
	// Instruction::Format format = inst->getFormat();
	// Instruction::Bytes *bytes = inst->getBytes();
	// int op = instruction->getOp();

    executeInstruction(inst, warpId);
    // TODO if (inst->isatomic()) inst->do_atomic(true);
    if (inst->GetOpType() == BARRIER_OP || inst->GetOpType() == MEMORY_BARRIER_OP)
        m_warpAtBarrier[warpId] = true;
    updateSIMTStack(warpId, inst, warp_pc);
  }
  if (m_liveThreadCount[warpId] > 0) someOneLive = true;
  if (!m_warpAtBarrier[warpId] && m_liveThreadCount[warpId] > 0) allAtBarrier = false;
}

void ThreadBlock::warp_exit(unsigned warp_id) {
    for (uint32_t i = 0; i < m_warp_count * m_warp_size; i++) {
        auto it = m_threads.find(i);
        if (it->second != nullptr) {
            it = m_threads.erase(it);
        }
    }
}

void ThreadBlock::executeInstruction(shared_ptr<Instruction> inst, unsigned warpId) {
    active_mask_t active_mask = GetWarp(warpId)->get_warp_active_mask();
    bool leading_thread = true;
    for (unsigned t = 0; t < m_warp_size; t++) {
        if (active_mask.test(t)) {
            // ?if (warpId == (unsigned(-1))) warpId = inst->GetWarp()->GetWarpId();
            unsigned tid = m_warp_size * warpId + t;
            printf("Info: Warp%d, thread%d, executeInstruction %lx\n", warpId, t, inst->bytes.dword);
            m_threads[tid]->m_leading_thread = leading_thread;
            leading_thread = false;
            m_threads[tid]->Execute(inst);

            // virtual function
            checkExecutionStatusAndUpdate(inst, t, tid);
        }
    }
}

bool ThreadBlock::thread_done(uint32_t hw_thread_id) {
  // return ((m_threads[hw_thread_id] == nullptr) ||
  //        m_threads[hw_thread_id]->is_done());
    return m_threads[hw_thread_id]->is_done();
}

void ThreadBlock::updateSIMTStack(unsigned warpId, shared_ptr<Instruction> inst, address_type warp_pc) {
    simt_mask_t thread_done;
    addr_vector_t next_pc;
    unsigned wtid = warpId * m_warp_size;
    for (unsigned i = 0; i < m_warp_size; i++) {
       if (this->thread_done(wtid + i)) {
           thread_done.set(i);
           next_pc.push_back((address_type)-1);
       } else {
           if (inst->reconvergence_pc == RECONVERGE_RETURN_PC)
               inst->reconvergence_pc = m_threads[wtid + i]->get_return_PC();
           next_pc.push_back(m_threads[wtid + i]->GetPC());
       }
    }
    m_warps[warpId]->update(thread_done, next_pc, inst->reconvergence_pc,
                               inst->GetOpType(), inst->GetSize(), warp_pc);
}
shared_ptr<Instruction> ThreadBlock::GetInstruction(address_type pc) {
    if (m_insts.find(pc) == m_insts.end()) {
        uint64_t pc_address = m_kernel_addr + pc;
        // FIXME
        // uint64_t opcode = *(uint64_t*)(pc_address);
        uint64_t opcode;
        m_cu->FetchInstruction(pc_address, &opcode, 8);
        debug_print("Fetch PC%lx Instr: opcode %lx\n", pc, opcode);
        m_insts[pc] = make_instruction(opcode, pc_address);
        m_insts[pc]->Decode(opcode);
        return m_insts[pc];
    }
    // FIXME modify m_insts to instrubuffer
    return m_insts[pc];
}

//! Get the warp to be executed using the data taken form the SIMT stack
shared_ptr<Instruction> ThreadBlock::GetExecuteWarp(unsigned warpId, address_type &pc) {
    address_type rpc;
    m_warps[warpId]->get_pdom_stack_top_info(&pc, &rpc);
    shared_ptr<Instruction> inst = GetInstruction(pc);
    m_warps[warpId]->update_warp_active_mask(inst->isatomic());
    return inst;
}

void ThreadBlock::get_pdom_stack_top_info(unsigned warpId, address_type *pc, address_type *rpc) {
    m_warps[warpId]->get_pdom_stack_top_info(pc, rpc);
}
/*
bool ThreadBlock::Execute() {
	bool active = false;
	auto it = m_warps.begin();

	while (it != m_warps.end()) {
		if (it->second->Execute()) {
			active = true;
			it++;
		}
		else {
			it = m_warps.erase(it);
		}
	}

	return active;
}
*/
/*
void ThreadBlock::HitBarrier(uint32_t abs_flat_id)
{
	// Add an record to the set of suspended thread items
	m_items_on_hold.insert(abs_flat_id);

	// Check if all the thread item reaches the barrier
	if (m_items_on_hold.size() == m_thread_items_num)
	{
		ActivateAllThreadItems();
		m_items_on_hold.clear();
	}
}

void ThreadBlock::ActivateAllThreadItems()
{
	for (auto it = m_warps.begin(); it != m_warps.end(); it++)
	{
		it->second->ActivateAllThreadItems();
	}
}
*/

void ThreadBlock::checkExecutionStatusAndUpdate(shared_ptr<Instruction> inst, unsigned t, unsigned tid) {
    if (m_threads[tid] == NULL || m_threads[tid]->is_done()) {
        m_liveThreadCount[tid / m_warp_size]--;
    }
}

std::map<uint32_t, std::shared_ptr<Warp>>::iterator ThreadBlock::WarpsBegin()
{
	return m_warps.begin();
}

	/// Return a past-the-end iterator to the list of wavefronts
std::map<uint32_t, std::shared_ptr<Warp>>::iterator ThreadBlock::WarpsEnd()
{
	return m_warps.end();
}

void ThreadBlock::ReadMemory(uint32_t warpId, uint32_t addr, void* data, uint32_t size) {
    m_warpReadCount[warpId] +=1;
    m_cu->ReadMemory(m_hw_blk_id, warpId, addr, data, size);
/*
    std::string block = "mc";
    auto pkt = make_shared<MemReadPacket>();
    pkt->addr = addr;
    pkt->is_pa = true;
    pkt->data = data;
    pkt->length = size;

    auto msg = make_shared<MemAccessMessage>(pkt);
    m_cu->PushReq("mc", msg);

    m_cu->m_response_queue->Push(msg);
    */
}

void ThreadBlock::WriteMemory(uint32_t warpId, uint32_t addr, void* data, uint32_t size){

    m_warpWriteCount[warpId] +=1;
    m_cu->WriteMemory(m_hw_blk_id, warpId, addr, data, size);
    /*
    std::string block = "mc";
    auto pkt = make_shared<MemWritePacket>();
    pkt->addr = addr;
    pkt->is_pa = true;
    pkt->data = data;
    pkt->length = size;

    auto msg = make_shared<MemAccessMessage>(pkt);
    m_cu->PushReq("mc", msg);

    m_cu->m_response_queue->Push(msg);
    */
}

void ThreadBlock::ReadMemoryResp(uint32_t warpId){
    m_warpReadCount[warpId] +=1;
    assert(m_warpReadCount[warpId] >= 0);
}

void ThreadBlock::WriteMemoryResp(uint32_t warpId){
    m_warpWriteCount[warpId] -=1;
    assert(m_warpWriteCount[warpId] >= 0);
}
