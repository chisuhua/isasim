#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.DLS

void InstructionDLS::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_size = 8;
}

// DS[A] = DS[A] + D0; uint add.
void InstructionDLS::D_ADD_U32(ThreadItem *item)
{
	ISAUnimplemented(item);
}


// DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment.
void InstructionDLS::D_INC_U32(ThreadItem *item)
{
	ISAUnimplemented(item);
}

// DS[ADDR+offset0*4] = D0; DS[ADDR+offset1*4] = D1; Write 2 Dwords
void InstructionDLS::D_WRITE2_B32(ThreadItem *item)
{
	Register addr0;
	Register addr1;
	Register data0;
	Register data1;

	assert(!opcode.gds);

	// Load address and data from registers.
	addr0.as_uint = ReadVReg(opcode.addr);
	addr0.as_uint += opcode.offset0*4;
	addr1.as_uint = ReadVReg(opcode.addr);
	addr1.as_uint += opcode.offset1*4;
	data0.as_uint = ReadVReg(opcode.data0);
	data1.as_uint = ReadVReg(opcode.data1);

	if (addr0.as_uint >
            std::min(GetBlock->GetLocalMemBase(), ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}
	if (addr1.as_uint >
            std::min(GetBlock->GetLocalMemBase(), ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}

	// Write Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		WriteLDS(addr0.as_uint, 4, (char *)&data0.as_uint);
		WriteLDS(addr1.as_uint, 4, (char *)&data1.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		// If offset1 != 1, then the following is incorrect
		assert(opcode.offset0 == 0);
		assert(opcode.offset1 == 1);
		item->lds_access_count = 2;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr0.as_uint;
		item->lds_access[0].size = 4;
		item->lds_access[1].type = MemoryAccessWrite;
		item->lds_access[1].addr = addr0.as_uint + 4;
		item->lds_access[1].size = 4;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && opcode.gds)
	{
		Emulator::isa_debug << misc::fmt("t%d: GDS[%u]<=(%u,%f) ", id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		Emulator::isa_debug << misc::fmt("GDS[%u]<=(%u,%f) ", addr1.as_uint, data0.as_uint,
			data0.as_float);
	}
	else
	{
		Emulator::isa_debug << misc::fmt("t%d: LDS[%u]<=(%u,%f) ", id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		Emulator::isa_debug << misc::fmt("LDS[%u]<=(%u,%f) ", addr1.as_uint, data1.as_uint, 
			data1.as_float);
	}
    */
}

// DS[A] = D0; write a Dword.
void InstructionDLS::D_WRITE_B32(ThreadItem *item)
{
	Register addr;
	Register data0;

	assert(!opcode.offset0);
	//assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(opcode.addr);
	data0.as_uint = ReadVReg(opcode.data0);

	if (addr.as_uint >
            std::min(GetBlock->GetLocalMemBase(), ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}

	// Global data store not supported
	assert(!opcode.gds);

	// Write Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 4, (char *)&data0.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && opcode.gds)
	{
		Emulator::isa_debug << misc::fmt("t%d: GDS[%u]<=(%u,%f) ", id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}
	else
	{
		Emulator::isa_debug << misc::fmt("t%d: LDS[%u]<=(%u,%f) ", id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}*/
}

// DS[A] = D0[7:0]; byte write. 
void InstructionDLS::D_WRITE_B8(ThreadItem *item)
{
	Register addr;
	Register data0;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(opcode.addr);
	data0.as_uint = ReadVReg(opcode.data0);

	// Global data store not supported
	assert(!opcode.gds);

	// Write Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 1, (char *)data0.as_ubyte);
	}

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && opcode.gds)
	{
		Emulator::isa_debug << misc::fmt("t%d: GDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ubyte[0]);
	}
	else
	{
		Emulator::isa_debug << misc::fmt("t%d: LDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ubyte[0]);
	}*/
}

// DS[A] = D0[15:0]; short write. 
void InstructionDLS::D_WRITE_B16(ThreadItem *item)
{
	Register addr;
	Register data0;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(opcode.addr);
	data0.as_uint = ReadVReg(opcode.data0);

	// Global data store not supported
	assert(!opcode.gds);

	// Write Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 2, (char *)data0.as_ushort);
	}

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && opcode.gds)
	{
		Emulator::isa_debug << misc::fmt("t%d: GDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ushort[0]);
	}
	else
	{
		Emulator::isa_debug << misc::fmt("t%d: LDS[%u]<=(0x%x) ", id, 
			addr.as_uint, data0.as_ushort[0]);
	}*/

}

// R = DS[A]; Dword read.
void InstructionDLS::D_READ_B32(ThreadItem *item)
{
	Register addr;
	Register data;

	assert(!opcode.offset0);
	//assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 4, (char *)&data.as_uint);
	}

	// Write results.
	WriteVReg(opcode.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			opcode.vdst, addr.as_uint, data.as_uint);
	}*/

}

// R = DS[ADDR+offset0*4], R+1 = DS[ADDR+offset1*4]. Read 2 Dwords.
void InstructionDLS::D_READ2_B32(ThreadItem *item)
{
	Register addr;
	Register data0;
	Register data1;

	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		ReadLDS(
			addr.as_uint + opcode.offset0*4, 4, (char *)&data0.as_uint);
		ReadLDS(
			addr.as_uint + opcode.offset1*4, 4, (char *)&data1.as_uint);
	}

	// Write results.
	WriteVReg(opcode.vdst, data0.as_uint);
	WriteVReg(opcode.vdst+1, data1.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		// If offset1 != 1, then the following is incorrect
		assert(opcode.offset0 == 0);
		assert(opcode.offset1 == 1);
		item->lds_access_count = 2;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
		item->lds_access[1].type = MemoryAccessRead;
		item->lds_access[1].addr = addr.as_uint + 4;
		item->lds_access[1].size = 4;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			opcode.vdst, addr.as_uint+opcode.offset0*4, 
			data0.as_uint);
		Emulator::isa_debug << misc::fmt("V%u<=(0x%x)(0x%x) ", opcode.vdst+1, 
			addr.as_uint+opcode.offset1*4, data1.as_uint);
	}*/
}

// R = signext(DS[A][7:0]}; signed byte read.
void InstructionDLS::D_READ_I8(ThreadItem *item)
{
	Register addr;
	Register data;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 1,
			&data.as_byte[0]);
	}

	// Extend the sign.
	data.as_int = (int) data.as_byte[0];

	// Write results.
	WriteVReg(opcode.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%d) ", id, opcode.vdst,
			addr.as_uint, data.as_int);
	}*/
}

// R = {24’h0,DS[A][7:0]}; unsigned byte read.
void InstructionDLS::D_READ_U8(ThreadItem *item)
{
	Register addr;
	Register data;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 1,
			(char *)&data.as_ubyte[0]);
	}

	// Make sure to use only bits [7:0].
	data.as_uint = (unsigned) data.as_ubyte[0];

	// Write results.
	WriteVReg(opcode.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds)
	{
		assert(0);
	}
	else
	{
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%u) ", id, opcode.vdst,
			addr.as_uint, data.as_uint);
	}*/
}

// R = signext(DS[A][15:0]}; signed short read.
void InstructionDLS::D_READ_I16(ThreadItem *item)
{
	Register addr;
	Register data;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds)
	{
		assert(0);
	}
	else
	{
		ReadLDS(addr.as_uint, 2, (char *)&data.as_short[0]);
	}

	// Extend the sign.
	data.as_int = (int) data.as_short[0];

	// Write results.
	WriteVReg(opcode.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds)
	{
		assert(0);
	}
	else
	{
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%d) ", id, opcode.vdst,
			addr.as_uint, data.as_int);
	}*/

}

// R = {16’h0,DS[A][15:0]}; uint16_t read.
void InstructionDLS::D_READ_U16(ThreadItem *item)
{
	Register addr;
	Register data;

	assert(!opcode.offset0);
	assert(!opcode.offset1);
	assert(!opcode.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(opcode.addr);

	// Global data store not supported
	assert(!opcode.gds);

	// Read Dword.
	if (opcode.gds) {
		assert(0);
	} else
	{
		ReadLDS(addr.as_uint, 2,
			(char *)&data.as_ushort[0]);
	}

	// Make sure to use only bits [15:0].
	data.as_uint = (unsigned) data.as_ushort[0];

	// Write results.
	WriteVReg(opcode.vdst, data.as_uint);

	// Record last memory access for the detailed simulator.
	if (opcode.gds) {
		assert(0);
	} else {
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%u) ", id, opcode.vdst,
			addr.as_uint, data.as_uint);
	}*/
}

