#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.VLS

void InstructionVLS::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    bytes.word[1] = 0;
	m_size = 4;
}

void InstructionVLS::V_LOAD_DWORD(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 2;

	MemoryPointer memory_pointer;
	ReadVRegMemPtr(vaddr, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;

	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		ReadMemory(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint);
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_LOAD_DWORDX2(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	MemoryPointer memory_pointer;
	ReadVRegMemPtr(vaddr, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[2];
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadMemory(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint);
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_LOAD_DWORDX4(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	MemoryPointer memory_pointer;
	ReadVRegMemPtr(vaddr, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[4];
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadMemory(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteVReg(opcode.vdata + i, value[i].as_uint);
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}

void InstructionVLS::V_STORE_DWORD(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	// assert(opcode.imm);

	int vaddr = opcode.vaddr; //  << 1;

	MemoryPointer memory_pointer;
	ReadVRegMemPtr(vaddr, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		value[i].as_uint = ReadVReg(opcode.vdata + i);
		// Store the data in the destination register
		WriteMemory(m_addr + i * 4, 4, (char *)&value[i]);
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}
