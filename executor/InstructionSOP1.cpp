#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "common/utils.h"


#define opcode bytes.SOP1

void InstructionSOP1::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_is_warp_op = true;
	/* 0xFF indicates the use of a literal constant as a
	 * source operand. */
	if (bytes.SOP1.ssrc0 == 0xFF) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
}

// D.u = S0.u.
void InstructionSOP1::S_MOV_B64(ThreadItem *item)
{
	// Assert no literal constant with a 64 bit instruction.
	// assert(!(opcode.ssrc0 == 0xFF));

	Register s0_lo;
	Register s0_hi;

	// Load operand from registers.
	if (opcode.ssrc0 == 0xFF){
		s0_lo.as_uint = opcode.lit_const;
		s0_hi.as_uint = 0x0;
	}
	else{
		s0_lo.as_uint = ReadSReg(opcode.ssrc0);
		s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, s0_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, s0_hi.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, s0_lo.as_uint);
//		Emulator::isa_debug << misc::fmt("S%u<=(0x%x)", opcode.sdst + 1, s0_hi.as_uint);
}

// D.u = S0.u.
void InstructionSOP1::S_MOV_B32(ThreadItem *item)
{
	Register s0;

	// Load operand from registers or as a literal constant.
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, s0.as_uint);

	// Print isa debug information.
    //		Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, s0.as_uint);
}

// D.u = ~S0.u SCC = 1 if result non-zero.
void InstructionSOP1::S_NOT_B32(ThreadItem *item)
{
	Register s0;
	Register nonzero;

	// Load operand from registers or as a literal constant.
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = ~opcode.lit_const;
	else
		s0.as_uint = ~ReadSReg(opcode.ssrc0);
	nonzero.as_uint = ! !s0.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, s0.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, s0.as_uint);
}

// D.u = WholeQuadMode(S0.u). SCC = 1 if result is non-zero.
void InstructionSOP1::S_WQM_B64(ThreadItem *item)
{
	ISAUnimplemented(item);
}

// D.u = PC + 4, PC = S0.u
void InstructionSOP1::S_SWAPPC_B64(ThreadItem *item)
{
	Register s0_lo;
	Register s0_hi;

	// FIXME: cuurently PC is implemented as 32-bit offset
	// Load operands from registers
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);

	// Write the results
	// Store the data in the destination register
	WriteSReg(opcode.sdst, GetWarp->GetWarpPC() + 4);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, 0);

	// Set the new PC
	GetWarp->SetWarpPC(s0_lo.as_uint - 4);

    debug_print("s%u <= (0x%x)", opcode.sdst+1, s0_hi.as_uint);
	// Print isa debug information.
    //	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, pc + 4);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, s0_hi.as_uint);
    //	Emulator::isa_debug << misc::fmt("PC<=(0x%x)", GetWarp->GetWarpPC());
}

/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is
 * non-zero. */
void InstructionSOP1::S_AND_SAVEEXEC_B64(ThreadItem *item)
{
	// Assert no literal constant with a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF));

	Register exec_lo;
	Register exec_hi;
	Register s0_lo;
	Register s0_hi;
	Register exec_new_lo;
	Register exec_new_hi;
	Register nonzero;

	// Load operands from registers.
	exec_lo.as_uint = ReadSReg(RegisterExec);
	exec_hi.as_uint = ReadSReg(RegisterExec + 1);
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);

	/* Bitwise AND exec and the first operand and determine if the result 
	 * is non-zero. */
	exec_new_lo.as_uint = s0_lo.as_uint & exec_lo.as_uint;
	exec_new_hi.as_uint = s0_hi.as_uint & exec_hi.as_uint;
	nonzero.as_uint = exec_new_lo.as_uint || exec_new_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, exec_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, exec_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterExec, exec_new_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterExec + 1, exec_new_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, exec_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, exec_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("exec_lo<=(0x%x) ", exec_new_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("exec_hi<=(0x%x) ", exec_new_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

