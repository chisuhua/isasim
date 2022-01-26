#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.MUBUF

void InstructionMUBUF::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_size = 8;
}

void Instruction::BUFFER_LOAD_SBYTE(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.glc);
	assert(!opcode.slc);
	assert(!opcode.tfe);
	assert(!opcode.lds);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 1;

	// srsrc is in units of 4 registers
	ReadBufferResource(opcode.srsrc * 4, buffer_descriptor);

	unsigned base = buffer_descriptor.base_addr;
	unsigned mem_offset = ReadSReg(opcode.soffset);
	unsigned inst_offset = opcode.offset;
	unsigned stride = buffer_descriptor.stride;

	// Table 8.3 from SI ISA
	if (!opcode.idxen && opcode.offen)
	{
		off_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && !opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
		off_vgpr = ReadVReg(opcode.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
		throw std::runtime_error("The buffer descriptor is probably invalid");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);


	ReadMemory(addr, bytes_to_read, (char *)&value);

	// Sign extend
	value.as_int = (int) value.as_byte[0];

	WriteVReg(opcode.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_read;

	/*if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%d) ", id,
			opcode.vdata, addr, value.as_int);
	}*/
}

void Instruction::BUFFER_LOAD_DWORD(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.glc);
	assert(!opcode.slc);
	assert(!opcode.tfe);
	assert(!opcode.lds);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 4;

	// srsrc is in units of 4 registers
	ReadBufferResource(opcode.srsrc * 4, buffer_descriptor);

	unsigned base = buffer_descriptor.base_addr;
	unsigned mem_offset = ReadSReg(opcode.soffset);
	unsigned inst_offset = opcode.offset;
	unsigned stride = buffer_descriptor.stride;

	// Table 8.3 from SI ISA
	if (!opcode.idxen && opcode.offen)
	{
		off_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && !opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
		off_vgpr = ReadVReg(opcode.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
		throw std::runtime_error("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);


	ReadMemory(addr, bytes_to_read, (char *)&value);

	// Sign extend
	value.as_int = (int) value.as_byte[0];

	WriteVReg(opcode.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_read;

	/*if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%d) ", id,
			opcode.vdata, addr, value.as_int);
	}*/
}

void Instruction::BUFFER_STORE_SBYTE(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.slc);
	assert(!opcode.tfe);
	assert(!opcode.lds);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_write = 1;

	if (opcode.glc)
	{
		GetWarp->SetVectorMemoryGlobalCoherency(true); // FIXME redundant
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(opcode.srsrc * 4, buffer_descriptor);

	unsigned base = buffer_descriptor.base_addr;
	unsigned mem_offset = ReadSReg(opcode.soffset);
	unsigned inst_offset = opcode.offset;
	unsigned stride = buffer_descriptor.stride;

	// Table 8.3 from SI ISA
	if (!opcode.idxen && opcode.offen)
	{
		off_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && !opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
		off_vgpr = ReadVReg(opcode.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
		throw std::runtime_error("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	value.as_int = ReadVReg(opcode.vdata);

	WriteMemory(addr, bytes_to_write, (char *)&value);

	// Sign extend
	//value.as_int = (int) value.as_byte[0];

	WriteVReg(opcode.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;

	/*if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%d) ", id,
			opcode.vdata, addr, value.as_int);
	}*/
}

void Instruction::BUFFER_STORE_DWORD(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.slc);
	assert(!opcode.tfe);
	assert(!opcode.lds);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_write = 4;

	if (opcode.glc)
	{
		GetWarp->SetVectorMemoryGlobalCoherency(true); // FIXME redundant
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(opcode.srsrc * 4, buffer_descriptor);

	unsigned base = buffer_descriptor.base_addr;
	unsigned mem_offset = ReadSReg(opcode.soffset);
	unsigned inst_offset = opcode.offset;
	unsigned stride = buffer_descriptor.stride;

	// Table 8.3 from SI ISA
	if (!opcode.idxen && opcode.offen)
	{
		off_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && !opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
		off_vgpr = ReadVReg(opcode.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
		throw std::runtime_error("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	value.as_int = ReadVReg(opcode.vdata);

	WriteMemory(addr, bytes_to_write, (char *)&value);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;

	/*if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: (%u)<=V%u(%d) ", id,
			addr, opcode.vdata, value.as_int);
	}*/
}

void Instruction::BUFFER_ATOMIC_ADD(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.slc);
	assert(!opcode.tfe);
	assert(!opcode.lds);

	BufferDescriptor buffer_descriptor;
	Register value;
	Register prev_value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int bytes_to_read = 4;
	int bytes_to_write = 4;

	if (opcode.glc)
	{
		GetWarp->SetVectorMemoryGlobalCoherency(true);
	}
	else
	{
		/* NOTE Regardless of whether the glc bit is set by the AMD
		 * compiler, for the NMOESI protocol correctness , the glc bit
		 * must be set. */
		GetWarp->SetVectorMemoryGlobalCoherency(true);
	}

	// srsrc is in units of 4 registers
	ReadBufferResource(opcode.srsrc * 4, buffer_descriptor);

	unsigned base = buffer_descriptor.base_addr;
	unsigned mem_offset = ReadSReg(opcode.soffset);
	unsigned inst_offset = opcode.offset;
	unsigned stride = buffer_descriptor.stride;

	// Table 8.3 from SI ISA
	if (!opcode.idxen && opcode.offen)
	{
		off_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && !opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
	}
	else if (opcode.idxen && opcode.offen)
	{
		idx_vgpr = ReadVReg(opcode.vaddr);
		off_vgpr = ReadVReg(opcode.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
		throw std::runtime_error("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	// Read existing value from global memory

	ReadMemory(addr, bytes_to_read, prev_value.as_byte);

	// Read value to add to existing value from a register
	value.as_int = ReadVReg(opcode.vdata);

	// Compute and store the updated value
	value.as_int += prev_value.as_int;
	WriteMemory(addr, bytes_to_write, (char *)&value);

	// If glc bit set, return the previous value in a register
	if (opcode.glc)
	{
		WriteVReg(opcode.vdata, prev_value.as_uint);
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;

    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%d) ", id,
			opcode.vdata, addr, value.as_int);
	}*/
}

