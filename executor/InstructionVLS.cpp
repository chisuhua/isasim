#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.VLS

void InstructionVLS::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    bytes.word[1] = 0;
	m_size = 4;
}

void InstructionVLS::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}

#if 0
void InstructionVLS::V_LOAD_WORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 2;

	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;

	uint64_t m_offset = opcode.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value;
	{
		// Read value from global memory
        value.as_uint = 0;
		ReadVMEM(m_addr, 2, (char *)&value);
		WriteVReg(opcode.vdata, value.as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}
#endif

void InstructionVLS::V_LOAD_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 2;

	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;

	uint64_t m_offset = opcode.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_LOAD_DWORDX2(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	// MemoryPointer memory_pointer;
	// ReadVRegMemPtr(vaddr, memory_pointer, lane_id);
	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;
	uint64_t m_offset = opcode.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[2];
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_LOAD_DWORDX4(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	// MemoryPointer memory_pointer;
	// ReadVRegMemPtr(vaddr, memory_pointer, lane_id);
	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;
	uint64_t m_offset = opcode.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[4];
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadVMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_STORE_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	MemoryPointer memory_pointer;
	ReadVRegMemPtr(vaddr, memory_pointer, lane_id);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		value[i].as_uint = ReadVReg(opcode.vdata + i, lane_id);
		// Store the data in the destination register
		WriteVMEM(m_addr + i * 4, 4, (char *)&value[i]);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}
