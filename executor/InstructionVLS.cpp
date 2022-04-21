#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VLS
#define INST InstructionVLS

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    bytes.word[1] = 0;
	m_size = 4;
}

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
    printVLS(OPCODE);
}

void INST::dumpExecBegin(WarpState *w) {
    std::stringstream ss;
	uint32_t vaddr = OPCODE.vaddr; //  << 2;
	int offset = OPCODE.vaddr; //  << 2;
	int vdst = OPCODE.vdata; //  << 2;
    if (OPCODE.ssrc0_){
        ss << "saddr" << std::hex << vaddr << "offset" << offset << "\n";
        ss << "sreg" << std::dec << vaddr << ":";
        w->dumpSreg(ss, vaddr);
        ss << "\n";
    } else {
        ss << "vaddr" << std::hex << vaddr << "offset" << offset << "\n";
        ss << "vreg" << std::dec << vaddr << ":";
        w->dumpVreg(ss, vaddr);
        ss << "\n";
    }
    w->out() << ss.str();
}

void INST::dumpExecEnd(WarpState *w) {
    this->dumpExecBegin(w);
}


uint64_t calculateAddr(Instruction::BytesVLS _opcode, WarpState* item, uint32_t lane_id) {
	MemoryPointer mem_ptr;
	int vaddr = _opcode.vaddr; //  << 2;
    if (_opcode.ssrc0_){
	    item->getSregMemPtr(vaddr, mem_ptr);
    } else {
	    item->getVregMemPtr(vaddr, mem_ptr, lane_id);
    }

	// Calculate effective address
	uint64_t m_base = mem_ptr.addr;
	uint64_t m_offset = _opcode.offset * 4;
	uint64_t m_addr = m_base + m_offset;
    return m_addr;
}

#if 0
void INST::V_LOAD_WORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);

	int vaddr = OPCODE.vaddr; //  << 2;

	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;

	uint64_t m_offset = OPCODE.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value;
	{
		// Read value from global memory
        value.as_uint = 0;
		ReadVMEM(m_addr, 2, (char *)&value);
		WriteVReg(OPCODE.vdata, value.as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}
#endif


void INST::V_LOAD_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t m_addr = calculateAddr(OPCODE, item, lane_id);
	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(OPCODE.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_LOAD_DWORDX2(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t m_addr = calculateAddr(OPCODE, item, lane_id);
	assert(!(m_addr & 0x3));

	Register value[2];
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(OPCODE.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_LOAD_DWORDX4(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t m_addr = calculateAddr(OPCODE, item, lane_id);
	assert(!(m_addr & 0x3));

	Register value[4];
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(OPCODE.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_STORE_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

    uint64_t m_addr = calculateAddr(OPCODE, item, lane_id);

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		value[i].as_uint = ReadVReg(OPCODE.vdata + i, lane_id);
		// Store the data in the destination register
		WriteVMEM(m_addr + i * 4, 4, (char *)&value[i]);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}
