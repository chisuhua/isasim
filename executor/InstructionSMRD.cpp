#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.SMRD

void InstructionSMRD::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    bytes.word[1] = 0;
	m_size = 4;
    m_is_warp_op = true;
}

void Instruction::S_BUFFER_LOAD_DWORD(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);
	int sbase = opcode.sbase << 1;

	// sbase holds the first of 4 registers containing the buffer
	// resource descriptor
	BufferDescriptor buffer_descriptor;
	ReadBufferResource(sbase, buffer_descriptor);

	// Calculate effective address
	unsigned m_base = buffer_descriptor.base_addr;
	unsigned m_offset = opcode.imm ? opcode.offset * 4 : ReadSReg(opcode.offset);
	unsigned addr = m_base + m_offset;

	// Read value from global memory
	Register value;
	ReadMemory(addr, 4, (char *)&value);

	// Store the data in the destination register
	WriteSReg(opcode.sdst, value.as_uint);

	// Debug
    /*
	if (Emulator::isa_debug)
		Emulator::isa_debug << misc::fmt("wf%d: S%u<=(%u)(%u,%gf)",
				GetWarp->getId(), opcode.sdst, addr,
				value.as_uint, value.as_float);
                */

	// Record last memory access for timing simulation purposes
	item->global_memory_access_address = addr;
	item->global_memory_access_size = 4;
}

void Instruction::S_BUFFER_LOAD_DWORDX2(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);
	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = (opcode.imm) ? (opcode.offset * 4) : ReadSReg(opcode.offset);
	unsigned addr = m_base + m_offset;

	Register value[2];
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadMemory(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("wf%d: ", GetWarp->getId());
		for (int i = 0; i < 2; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u)(%u,%gf) ", opcode.sdst + i,
				addr + i * 4, value[i].as_uint,
				value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = 4 * 2;
}


void Instruction::S_BUFFER_LOAD_DWORDX4(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);
	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = (opcode.imm) ? (opcode.offset * 4) : ReadSReg(opcode.offset);
	unsigned addr = m_base + m_offset;

	Register value[4];
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadMemory(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("wf%d: ", GetWarp->getId());
		for (int i = 0; i < 4; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u)(%u,%gf) ", opcode.sdst + i,
				addr + i*4, value[i].as_uint,
				value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = 4 * 4;
}

void Instruction::S_BUFFER_LOAD_DWORDX8(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);
	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = (opcode.imm) ? (opcode.offset * 4) : ReadSReg(opcode.offset);
	unsigned addr = m_base + m_offset;

	Register value[8];
	for (int i = 0; i < 8; i++)
	{
		// Read value from global memory
		ReadMemory(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("wf%d: ", GetWarp->getId());
		for (int i = 0; i < 8; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u)(%u,%gf) ", opcode.sdst + i,
				addr + i*4, value[i].as_uint,
				value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = 4 * 8;
}

void Instruction::S_BUFFER_LOAD_DWORDX16(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);
	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = (opcode.imm) ? (opcode.offset * 4) : ReadSReg(opcode.offset);
	unsigned addr = m_base + m_offset;

	Register value[16];
	for (int i = 0; i < 16; i++)
	{
		// Read value from global memory
		ReadMemory(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("wf%d: ", GetWarp->getId());
		for (int i = 0; i < 16; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u)(%u,%gf) ", opcode.sdst + i,
				addr + i*4, value[i].as_uint,
				value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = 4 * 16;
}

void Instruction::S_LOAD_DWORD(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	assert(opcode.imm);

	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

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
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", opcode.sdst, opcode.sdst+3,
			m_addr);
		for (int i = 0; i < 2; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", opcode.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}

void Instruction::S_LOAD_DWORDX2(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	assert(opcode.imm);

	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

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
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", opcode.sdst, opcode.sdst+3,
			m_addr);
		for (int i = 0; i < 2; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", opcode.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 2;
}

void Instruction::S_LOAD_DWORDX4(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	assert(opcode.imm);

	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

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
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", opcode.sdst, opcode.sdst+3,
			m_addr);
		for (int i = 0; i < 4; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", opcode.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 4;
}

void Instruction::S_LOAD_DWORDX8(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	assert(opcode.imm);

	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[8];
	for (int i = 0; i < 8; i++)
	{
		// Read value from global memory
		ReadMemory(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", opcode.sdst, opcode.sdst+3,
			m_addr);
		for (int i = 0; i < 8; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", opcode.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 8;
}

void Instruction::S_LOAD_DWORDX16(ThreadItem *item)
{
	// Record access
	GetWarp->SetScalarMemoryRead(true);

	assert(opcode.imm);

	int sbase = opcode.sbase << 1;

	MemoryPointer memory_pointer;
	ReadMemPtr(sbase, memory_pointer);

	// Calculate effective address
	unsigned m_base = memory_pointer.addr;
	unsigned m_offset = opcode.offset * 4;
	unsigned m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value[16];
	for (int i = 0; i < 16; i++)
	{
		// Read value from global memory
		ReadMemory(m_addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
		WriteSReg(opcode.sdst + i, value[i].as_uint);
	}

	// Debug
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("S[%u,%u] <= (addr %u): ", opcode.sdst, opcode.sdst+3,
			m_addr);
		for (int i = 0; i < 8; i++)
		{
			Emulator::isa_debug << misc::fmt("S%u<=(%u,%gf) ", opcode.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}*/

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = m_addr;
	item->global_memory_access_size = 4 * 16;
}


