#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.MTBUF

void InstructionMTBUF::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_size = 8;
}

int ISAGetNumElems(int data_format)
{
	int num_elems;

	switch (data_format)
	{
	case 1:
	case 2:
	case 4:
	{
		num_elems = 1;
		break;
	}

	case 3:
	case 5:
	case 11:
	{
		num_elems = 2;
		break;
	}

	case 13:
	{
		num_elems = 3;
		break;
	}

	case 10:
	case 12:
	case 14:
	{
		num_elems = 4;
		break;
	}

	default:

		throw std::runtime_error("Invalid data format");

	}

	return num_elems;
}

int ISAGetElemSize(int data_format)
{
	int elem_size;

	switch (data_format)
	{

	// 8-bit data
	case 1:
	case 3:
	case 10:
	{
		elem_size = 1;
		break;
	}

	// 16-bit data
	case 2:
	case 5:
	case 12:
	{
		elem_size = 2;
		break;
	}

	// 32-bit data
	case 4:
	case 11:
	case 13:
	case 14:
	{
		elem_size = 4;
		break;
	}

	default:
		throw std::runtime_error("Invalid data format");

	}

	return elem_size;
}


void Instruction::TBUFFER_LOAD_FORMAT_X(ThreadItem *item)
{

	assert(!opcode.addr64);
	assert(!opcode.tfe);
	assert(!opcode.slc);

	BufferDescriptor buffer_descriptor;
	Register value;

	int bytes_to_read;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

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

	// Calculate the address
	// XXX Need to know when to enable m_laneId
	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + 0/*work_item->m_laneId*/);


	ReadMemory(addr, bytes_to_read, (char *)&value);

	WriteVReg(opcode.vdata, value.as_uint);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_read;

	// TODO Print value based on type
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%u,%gf) ", id,
			opcode.vdata, addr, value.as_uint, value.as_float);
		if (opcode.offen)
			Emulator::isa_debug << misc::fmt("offen ");
		if (opcode.idxen)
			Emulator::isa_debug << misc::fmt("idxen ");
		Emulator::isa_debug << misc::fmt("%u,%u,%u,%u,%u,%u ", base, mem_offset,
			inst_offset, off_vgpr, idx_vgpr, stride);
	}
    */
}

void Instruction::TBUFFER_LOAD_FORMAT_XY(ThreadItem *item)
{

	assert(!opcode.addr64);

	BufferDescriptor buffer_descriptor;
	Register value;

	int i;
	int bytes_to_read;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

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

	// Calculate the address
	// XXX Need to know when to enable m_laneId
	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + 0/*work_item->m_laneId*/);

	for (i = 0; i < 2; i++)
	{

		ReadMemory(addr+4*i, 4, (char *)&value);

		WriteVReg(opcode.vdata + i, value.as_uint);

		// TODO Print value based on type
        /*
		if (Emulator::isa_debug)
		{
			Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%u,%gf) ", id,
				opcode.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}*/
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_read;
}

void Instruction::TBUFFER_LOAD_FORMAT_XYZ(ThreadItem *item)
{
	ISAUnimplemented(item);
}

void Instruction::TBUFFER_LOAD_FORMAT_XYZW(ThreadItem *item)
{

	assert(!opcode.addr64);

	BufferDescriptor buffer_descriptor;
	Register value;

	int i;
	int bytes_to_read;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;
	unsigned id_in_wavefront = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

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
    /*
	if (idx_vgpr && !stride)
		throw std::runtime_error("Probably invalid buffer descriptor");
        */

	// XXX Need to know when to enable id_in_wavefront
	id_in_wavefront = buffer_descriptor.add_tid_enable ?  id_in_wavefront : 0;

	// Calculate the address
	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + id_in_wavefront);

	for (i = 0; i < 4; i++)
	{

		ReadMemory(addr+4*i, 4, (char *)&value);

		WriteVReg(opcode.vdata + i, value.as_uint);

		// TODO Print value based on type
        /*
		if (Emulator::isa_debug)
		{
			Emulator::isa_debug << misc::fmt("t%d: V%u<=(%u)(%u,%gf) ", id,
				opcode.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}*/
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_read;
}

void Instruction::TBUFFER_STORE_FORMAT_X(ThreadItem *item)
{

	assert(!opcode.addr64);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	int bytes_to_write = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

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
		assert("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	value.as_uint = ReadVReg(opcode.vdata);

	WriteMemory(addr, bytes_to_write, (char *)&value);

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;

	// TODO Print value based on type
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: (%u)<=V%u(%u,%gf) ", id,
			addr, opcode.vdata, value.as_uint,
			value.as_float);
	}*/
}

void Instruction::TBUFFER_STORE_FORMAT_XY(ThreadItem *item)
{

	assert(!opcode.addr64);

	BufferDescriptor buffer_descriptor;
	Register value;


	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	int bytes_to_write = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

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
		assert("Probably invalid buffer descriptor");

	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	for (unsigned i = 0; i < 2; i++)
	{
		value.as_uint = ReadVReg(opcode.vdata + i);

		WriteMemory(addr+4*i, 4, (char *)&value);

		// TODO Print value based on type
        /*
		if (Emulator::isa_debug)
		{
			Emulator::isa_debug << misc::fmt("t%d: (%u)<=V%u(%u,%gf) ", id,
				addr, opcode.vdata+i, value.as_uint,
				value.as_float);
		}*/
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;
}

void Instruction::TBUFFER_STORE_FORMAT_XYZW(ThreadItem *item)
{

	assert(!opcode.addr64);

	BufferDescriptor buffer_descriptor;
	Register value;

	unsigned off_vgpr = 0;
	unsigned idx_vgpr = 0;

	int elem_size = ISAGetElemSize(opcode.dfmt);
	int num_elems = ISAGetNumElems(opcode.dfmt);
	int bytes_to_write = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

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
		assert("Probably invalid buffer descriptor");

	// Calculate the address
	unsigned addr = base + mem_offset + inst_offset + off_vgpr +
		stride * (idx_vgpr + item->m_laneId);

	for (unsigned i = 0; i < 4; i++)
	{
		value.as_uint = ReadVReg(opcode.vdata + i);

		WriteMemory(addr+4*i, 4, (char *)&value);

		// TODO Print value based on type
        /*
		if (Emulator::isa_debug)
			Emulator::isa_debug << misc::fmt("t%d: (%u)<=V%u(%u,%gf) ",
				id,
				addr,
				opcode.vdata + i,
				value.as_uint,
				value.as_float);
                */
	}

	// Record last memory access for the detailed simulator.
	item->global_memory_access_address = addr;
	item->global_memory_access_size = bytes_to_write;
}

