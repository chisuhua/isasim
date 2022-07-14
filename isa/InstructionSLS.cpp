#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.SLS
#define INST InstructionSLS

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    bytes.word[1] = 0;
	m_size = 4;
    m_is_warp_op = true;
    m_decoded = true;
}

void INST::print() {
    Instruction::print();
    printSLS(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}

void INST::S_LOAD_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	// GetWarp->SetScalarMemoryRead(true);
    item->m_smem_read_counter++;

	assert(OPCODE.imm);

	int sbase = OPCODE.sbase << 1;

	MemoryPointer memory_pointer;
	item->getSregMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = OPCODE.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[1];
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		ReadSMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(OPCODE.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", OPCODE.sdst, OPCODE.sdst+3,
			m_addr);
		for (int i = 0; i < 2; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", OPCODE.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::S_LOAD_DWORDX2(WarpState *item, uint32_t lane_id)
{
	// Record access
	// GetWarp->SetScalarMemoryRead(true);
    item->m_smem_read_counter++;

	assert(OPCODE.imm);

	int sbase = OPCODE.sbase << 1;

	MemoryPointer memory_pointer;
	item->getSregMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = OPCODE.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[2];
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadSMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(OPCODE.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", OPCODE.sdst, OPCODE.sdst+3,
			m_addr);
		for (int i = 0; i < 2; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", OPCODE.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::S_LOAD_DWORDX4(WarpState *item, uint32_t lane_id)
{
	// Record access
	// GetWarp->SetScalarMemoryRead(true);
    item->m_smem_read_counter++;

	assert(OPCODE.imm);

	int sbase = OPCODE.sbase << 1;

	MemoryPointer memory_pointer;
	item->getSregMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = OPCODE.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[4];
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadSMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(OPCODE.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", OPCODE.sdst, OPCODE.sdst+3,
			m_addr);
		for (int i = 0; i < 4; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", OPCODE.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 4;
}

void INST::S_LOAD_DWORDX8(WarpState *item, uint32_t lane_id)
{
	// Record access
	// GetWarp->SetScalarMemoryRead(true);
    item->m_smem_read_counter++;

	assert(OPCODE.imm);

	int sbase = OPCODE.sbase << 1;

	MemoryPointer memory_pointer;
	item->getSregMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = OPCODE.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[8];
	for (int i = 0; i < 8; i++)
	{
		// Read value from global memory
		ReadSMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(OPCODE.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", OPCODE.sdst, OPCODE.sdst+3,
			m_addr);
		for (int i = 0; i < 8; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", OPCODE.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 8;
}

void INST::S_LOAD_DWORDX16(WarpState *item, uint32_t lane_id)
{
	// Record access
	// GetWarp->SetScalarMemoryRead(true);
    item->m_smem_read_counter++;

	assert(OPCODE.imm);

	int sbase = OPCODE.sbase << 1;

	MemoryPointer memory_pointer;
	item->getSregMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = OPCODE.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[16];
	for (int i = 0; i < 16; i++)
	{
		// Read value from global memory
		ReadSMEM(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(OPCODE.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", OPCODE.sdst, OPCODE.sdst+3,
			m_addr);
		for (int i = 0; i < 8; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", OPCODE.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 16;
}


