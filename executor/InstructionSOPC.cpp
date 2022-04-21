#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.SOPC
#define INST InstructionSOPC

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
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

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}

void INST::dumpExecBegin(WarpState *w) {
}

void INST::dumpExecEnd(WarpState *w) {
}


// scc = (S0.i == S1.i).
void INST::S_CMP_EQ_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("wf%d: scc<=(%u) (%u ==? %u)", GetWarp->getId(), result.as_uint, s0.as_int, s1.as_int);
}

// scc = (S0.i > S1.i).
void INST::S_CMP_GT_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

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
void INST::S_CMP_GE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.i < S1.i).
void INST::S_CMP_LT_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.i <= S1.i).
void INST::S_CMP_LE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u > S1.u).
void INST::S_CMP_GT_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u >= S1.u).
void INST::S_CMP_GE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

// scc = (S0.u <= S1.u).
void INST::S_CMP_LE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	assert(!(OPCODE.ssrc0 == 0xFF && OPCODE.ssrc1 == 0xFF));
	if (OPCODE.ssrc0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadSReg(OPCODE.ssrc0);
	if (OPCODE.ssrc1 == 0xFF)
		s1.as_uint = OPCODE.lit_const;
	else
		s1.as_uint = ReadSReg(OPCODE.ssrc1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(RegisterScc, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("scc<=(%u) ", result.as_uint);
}

