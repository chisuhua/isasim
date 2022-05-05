#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.DLS
#define INST InstructionDLS

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    m_size = 8;
}

void INST::print() {
    Instruction::print();
    printDLS(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}

// DS[A] = DS[A] + D0; uint add.
void INST::D_ADD_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}


// DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment.
void INST::D_INC_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// DS[ADDR+offset0*4] = D0; DS[ADDR+offset1*4] = D1; Write 2 Dwords
void INST::D_WRITE2_B32(WarpState *item, uint32_t lane_id)
{
#if 0
	Register addr0;
	Register addr1;
	Register data0;
	Register data1;

	assert(!OPCODE.gds);

	// Load address and data from registers.
	addr0.as_uint = ReadVReg(OPCODE.addr);
	addr0.as_uint += OPCODE.offset0*4;
	addr1.as_uint = ReadVReg(OPCODE.addr);
	addr1.as_uint += OPCODE.offset1*4;
	data0.as_uint = ReadVReg(OPCODE.data0);
	data1.as_uint = ReadVReg(OPCODE.data1);

	if (addr0.as_uint >
            std::min(GetBlock->GetLocalMemBase(), ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}
	if (addr1.as_uint >
            std::min(GetBlock->GetLocalMemBase(), ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}

	// Write Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		WriteLDS(addr0.as_uint, 4, (char *)&data0.as_uint);
		WriteLDS(addr1.as_uint, 4, (char *)&data1.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
		// If offset1 != 1, then the following is incorrect
		assert(OPCODE.offset0 == 0);
		assert(OPCODE.offset1 == 1);
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
	if (Emulator::isa_debug && OPCODE.gds)
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
#endif
}

// DS[A] = D0; write a Dword.
void INST::D_WRITE_B32(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data0;

	assert(!OPCODE.offset0);
	//assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);
	data0.as_uint = ReadVReg(OPCODE.data0, lane_id);
#if 0
	if (addr.as_uint >
        // FIXME 
        // std::min(item->m_shared_mem, ReadSReg(RegisterM0))) {
		assert("Invalid local memory address");
	}
#endif
	// Global data store not supported
	assert(!OPCODE.gds);

	// Write Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 4, (char*)&data0.as_uint);
	}

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && OPCODE.gds)
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
void INST::D_WRITE_B8(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data0;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);
	data0.as_uint = ReadVReg(OPCODE.data0, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Write Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 1, (char *)data0.as_ubyte);
	}

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && OPCODE.gds)
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
void INST::D_WRITE_B16(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data0;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address and data from registers.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);
	data0.as_uint = ReadVReg(OPCODE.data0, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Write Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		WriteLDS(addr.as_uint, 2, (char *)data0.as_ushort);
	}

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessWrite;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug && OPCODE.gds)
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
void INST::D_READ_B32(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data;

	assert(!OPCODE.offset0);
	//assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 4, (char *)&data.as_uint);
	}

	// Write results.
	WriteVReg(OPCODE.vdst, data.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			OPCODE.vdst, addr.as_uint, data.as_uint);
	}*/

}

// R = DS[ADDR+offset0*4], R+1 = DS[ADDR+offset1*4]. Read 2 Dwords.
void INST::D_READ2_B32(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data0;
	Register data1;

	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		ReadLDS(
			addr.as_uint + OPCODE.offset0*4, 4, (char *)&data0.as_uint);
		ReadLDS(
			addr.as_uint + OPCODE.offset1*4, 4, (char *)&data1.as_uint);
	}

	// Write results.
	WriteVReg(OPCODE.vdst, data0.as_uint, lane_id);
	WriteVReg(OPCODE.vdst+1, data1.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
		// If offset1 != 1, then the following is incorrect
		assert(OPCODE.offset0 == 0);
		assert(OPCODE.offset1 == 1);
#if 0
		item->lds_access_count = 2;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 4;
		item->lds_access[1].type = MemoryAccessRead;
		item->lds_access[1].addr = addr.as_uint + 4;
		item->lds_access[1].size = 4;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(0x%x) ", id, 
			OPCODE.vdst, addr.as_uint+OPCODE.offset0*4, 
			data0.as_uint);
		Emulator::isa_debug << misc::fmt("V%u<=(0x%x)(0x%x) ", OPCODE.vdst+1, 
			addr.as_uint+OPCODE.offset1*4, data1.as_uint);
	}*/
}

// R = signext(DS[A][7:0]}; signed byte read.
void INST::D_READ_I8(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 1,
			&data.as_byte[0]);
	}

	// Extend the sign.
	data.as_int = (int) data.as_byte[0];

	// Write results.
	WriteVReg(OPCODE.vdst, data.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%d) ", id, OPCODE.vdst,
			addr.as_uint, data.as_int);
	}*/
}

// R = {24’h0,DS[A][7:0]}; unsigned byte read.
void INST::D_READ_U8(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds) {
		assert(0);
	} else {
		ReadLDS(addr.as_uint, 1,
			(char *)&data.as_ubyte[0]);
	}

	// Make sure to use only bits [7:0].
	data.as_uint = (unsigned) data.as_ubyte[0];

	// Write results.
	WriteVReg(OPCODE.vdst, data.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds)
	{
		assert(0);
	}
	else
	{
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 1;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%u) ", id, OPCODE.vdst,
			addr.as_uint, data.as_uint);
	}*/
}

// R = signext(DS[A][15:0]}; signed short read.
void INST::D_READ_I16(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds)
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
	WriteVReg(OPCODE.vdst, data.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds)
	{
		assert(0);
	}
	else
	{
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%d) ", id, OPCODE.vdst,
			addr.as_uint, data.as_int);
	}*/

}

// R = {16’h0,DS[A][15:0]}; uint16_t read.
void INST::D_READ_U16(WarpState *item, uint32_t lane_id)
{
	Register addr;
	Register data;

	assert(!OPCODE.offset0);
	assert(!OPCODE.offset1);
	assert(!OPCODE.gds);

	// Load address from register.
	addr.as_uint = ReadVReg(OPCODE.addr, lane_id);

	// Global data store not supported
	assert(!OPCODE.gds);

	// Read Dword.
	if (OPCODE.gds) {
		assert(0);
	} else
	{
		ReadLDS(addr.as_uint, 2,
			(char *)&data.as_ushort[0]);
	}

	// Make sure to use only bits [15:0].
	data.as_uint = (unsigned) data.as_ushort[0];

	// Write results.
	WriteVReg(OPCODE.vdst, data.as_uint, lane_id);

	// Record last memory access for the detailed simulator.
	if (OPCODE.gds) {
		assert(0);
	} else {
#if 0
		item->lds_access_count = 1;
		item->lds_access[0].type = MemoryAccessRead;
		item->lds_access[0].addr = addr.as_uint;
		item->lds_access[0].size = 2;
#endif
	}

	// Print isa debug information.
    /*
	if (Emulator::isa_debug)
	{
		Emulator::isa_debug << misc::fmt("t%d: V%u<=(0x%x)(%u) ", id, OPCODE.vdst,
			addr.as_uint, data.as_uint);
	}*/
}

