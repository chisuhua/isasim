#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.SOPC

void InstructionSOPC::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_is_warp_op = true;
	/* Only one source field may use a literal constant,
	 * which is indicated by 0xFF. */
	if (bytes.SOPC.ssrc0 == 0xFF ||
		bytes.SOPC.ssrc1 == 0xFF) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 8;
    }

}

// scc = (S0.i == S1.i).
void InstructionSOPC::S_CMP_EQ_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("wf%d: scc<=(%u) (%u ==? %u)", GetWarp->getId(), result.as_uint, s0.as_int, s1.as_int);
}

// scc = (S0.i > S1.i).
void InstructionSOPC::S_CMP_GT_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) (%u >? %u) ", result.as_uint,
	//		s0.as_uint, s1.as_uint);
}

// scc = (S0.i >= S1.i).
void InstructionSOPC::S_CMP_GE_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.i < S1.i).
void InstructionSOPC::S_CMP_LT_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.i <= S1.i).
void InstructionSOPC::S_CMP_LE_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u > S1.u).
void InstructionSOPC::S_CMP_GT_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u >= S1.u).
void InstructionSOPC::S_CMP_GE_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u <= S1.u).
void InstructionSOPC::S_CMP_LE_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadSReg(opcode.ssrc0);
	if (opcode.ssrc1 == 0xFF)
		s1.as_uint = opcode.lit_const;
	else
		s1.as_uint = ReadSReg(opcode.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

