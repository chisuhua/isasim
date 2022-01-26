#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.SOP2

void InstructionSOP2::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_is_warp_op = true;
	/* Only one source field may use a literal constant,
	 * which is indicated by 0xFF. */
	assert(!(bytes.SOP2.ssrc0 == 0xFF &&
		bytes.SOP2.ssrc1 == 0xFF));
	if (bytes.SOP2.ssrc0 == 0xFF ||
		bytes.SOP2.ssrc1 == 0xFF) {
		m_size = 8;
	} else{
        bytes.word[1] = 0;
		m_size = 4;
    }
}

// D.u = S0.u + S1.u. SCC = carry out.
void InstructionSOP2::S_ADD_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register sum;
	Register carry;

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

	// Calculate the sum and carry out.
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint = ((unsigned long long) s0.as_uint + 
		(unsigned long long) s1.as_uint) >> 32;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, carry.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%u) ", opcode.sdst, sum.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", carry.as_uint);
}


// D.u = S0.i + S1.i. scc = overflow.
void InstructionSOP2::S_ADD_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register sum;
	Register ovf;

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

	// Calculate the sum and overflow.
	sum.as_int = s0.as_int + s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 0 : 
		((s0.as_int > 0 && sum.as_int < 0) || 
		(s0.as_int < 0 && sum.as_int > 0));

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%u) ", opcode.sdst, sum.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", ovf.as_uint);
}

// D.u = S0.i - S1.i. scc = overflow.
void InstructionSOP2::S_SUB_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register diff;
	Register ovf;

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

	// Calculate the sum and overflow.
	diff.as_int = s0.as_int - s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 
		((s0.as_int > 0 && diff.as_int < 0) ||
		(s0.as_int < 0 && diff.as_int > 0)) : 0;

	// Write the results.
		// Store the data in the destination register
	WriteSReg(opcode.sdst, diff.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%d) ", opcode.sdst, diff.as_int);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", ovf.as_uint);
}

// D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min.
void InstructionSOP2::S_MIN_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register min;
	Register s0_min;

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

	// Calculate the minimum operand.
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
		s0_min.as_uint = 1;
	}
	else
	{
		min.as_uint = s1.as_uint;
		s0_min.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, min.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, s0_min.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%u) ", opcode.sdst, min.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%d) ", s0_min.as_uint);
}

// D.i = (S0.i > S1.i) ? S0.i : S1.i, scc = 1 if S0 is max.
void InstructionSOP2::S_MAX_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register max;
	Register s0_max;

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

	// Calculate the maximum operand.
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_int = s1.as_int;
		s0_max.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, max.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, s0_max.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%d) ", opcode.sdst, max.as_int);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", s0_max.as_uint);
}

// D.u = (S0.u > S1.u) ? S0.u : S1.u, scc = 1 if S0 is max.
void InstructionSOP2::S_MAX_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register max;
	Register s0_max;

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

	// Calculate the maximum operand.
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_uint = s1.as_uint;
		s0_max.as_uint = 0;
	}

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, max.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, s0_max.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%u) ", opcode.sdst, max.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", s0_max.as_uint);
}

// D.u = SCC ? S0.u : S1.u
void InstructionSOP2::S_CSELECT_B32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register scc;
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
	scc.as_uint = ReadSReg(RegisterScc);

	// Calculate the result
	result.as_uint = scc.as_uint ? s0.as_uint : s1.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
}

// D.u = S0.u & S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_AND_B32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;
	Register nonzero;

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

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint & s1.as_uint;
	nonzero.as_uint = ! !result.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u & S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_AND_B64(ThreadItem *item)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF || opcode.ssrc1 == 0xFF));

	Register s0_lo;
	Register s0_hi;
	Register s1_lo;
	Register s1_hi;
	Register result_lo;
	Register result_hi;
	Register nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(opcode.ssrc1);
	s1_hi.as_uint = ReadSReg(opcode.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint & s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, result_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u | S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_OR_B32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;
	Register nonzero;

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

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint | s1.as_uint;
	nonzero.as_uint = ! !result.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u | S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_OR_B64(ThreadItem *item)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF || opcode.ssrc1 == 0xFF));

	Register s0_lo;
	Register s0_hi;
	Register s1_lo;
	Register s1_hi;
	Register result_lo;
	Register result_hi;
	Register nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(opcode.ssrc1);
	s1_hi.as_uint = ReadSReg(opcode.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint | s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint | s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, result_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u ^ S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_XOR_B64(ThreadItem *item)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF || opcode.ssrc1 == 0xFF));

	Register s0_lo;
	Register s0_hi;
	Register s1_lo;
	Register s1_hi;
	Register result_lo;
	Register result_hi;
	Register nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(opcode.ssrc1);
	s1_hi.as_uint = ReadSReg(opcode.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint ^ s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint ^ s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, result_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u & ~S1.u. scc = 1 if result is non-zero.
void InstructionSOP2::S_ANDN2_B64(ThreadItem *item)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF || opcode.ssrc1 == 0xFF));

	Register s0_lo;
	Register s0_hi;
	Register s1_lo;
	Register s1_hi;
	Register result_lo;
	Register result_hi;
	Register nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(opcode.ssrc1);
	s1_hi.as_uint = ReadSReg(opcode.ssrc1 + 1);

	/* Bitwise AND the first operand with the negation of the second and
	 * determine if the result is non-zero. */
	result_lo.as_uint = s0_lo.as_uint & ~s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & ~s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, result_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

// D.u = ~(S0.u & S1.u). scc = 1 if result is non-zero.
void InstructionSOP2::S_NAND_B64(ThreadItem *item)
{
	// Assert no literal constants for a 64 bit instruction.
	assert(!(opcode.ssrc0 == 0xFF || opcode.ssrc1 == 0xFF));

	Register s0_lo;
	Register s0_hi;
	Register s1_lo;
	Register s1_hi;
	Register result_lo;
	Register result_hi;
	Register nonzero;

	// Load operands from registers.
	s0_lo.as_uint = ReadSReg(opcode.ssrc0);
	s0_hi.as_uint = ReadSReg(opcode.ssrc0 + 1);
	s1_lo.as_uint = ReadSReg(opcode.ssrc1);
	s1_hi.as_uint = ReadSReg(opcode.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = ~(s0_lo.as_uint & s1_lo.as_uint);
	result_hi.as_uint = ~(s0_hi.as_uint & s1_hi.as_uint);
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(opcode.sdst + 1, result_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst + 1, result_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u) ", nonzero.as_uint);
}

// D.u = S0.u << S1.u[4:0]. scc = 1 if result is non-zero.
void InstructionSOP2::S_LSHL_B32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;
	Register nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
	{
		s0.as_uint = opcode.lit_const;
	}
	else
	{
		s0.as_uint = ReadSReg(opcode.ssrc0);
	}
	if (opcode.ssrc1 == 0xFF)
	{
		assert(opcode.lit_const < 32);
		s1.as_uint = opcode.lit_const;
	}
	else
	{
		s1.as_uint = ReadSReg(opcode.ssrc1) & 0x1F;
	}

	/* Left shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint << s1.as_uint;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

// D.u = S0.u >> S1.u[4:0]. scc = 1 if result is non-zero.
void InstructionSOP2::S_LSHR_B32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;
	Register nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
	{
		s0.as_uint = opcode.lit_const;
	}
	else
	{
		s0.as_uint = ReadSReg(opcode.ssrc0);
	}
	if (opcode.ssrc1 == 0xFF)
	{
		assert(opcode.lit_const < 32);
		s1.as_uint = opcode.lit_const;
	}
	else
	{
		s1.as_uint = ReadSReg(opcode.ssrc1) & 0x1F;
	}

	/* Right shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint >> s1.as_uint;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

// D.i = signext(S0.i) >> S1.i[4:0]. scc = 1 if result is non-zero.
void InstructionSOP2::S_ASHR_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register result;
	Register nonzero;

	// Load operands from registers or as a literal constant.
	assert(!(opcode.ssrc0 == 0xFF && opcode.ssrc1 == 0xFF));
	if (opcode.ssrc0 == 0xFF)
	{
		s0.as_uint = opcode.lit_const;
	}
	else
	{
		s0.as_uint = ReadSReg(opcode.ssrc0);
	}
	if (opcode.ssrc1 == 0xFF)
	{
		assert(opcode.lit_const < 32);
		s1.as_uint = opcode.lit_const;
	}
	else
	{
		s1.as_uint = ReadSReg(opcode.ssrc1) & 0x1F;
	}

	/* Right shift the first operand sign extended by the second and
	 * determine if the result is non-zero. */
	result.as_int = s0.as_int >> s1.as_int;
	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%d) ", opcode.sdst, result.as_int);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

// D.i = S0.i * S1.i.
void InstructionSOP2::S_MUL_I32(ThreadItem *item)
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

	// Multiply the two operands.
	result.as_int = s0.as_int * s1.as_int;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(%d)", opcode.sdst, result.as_int);
}

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
void InstructionSOP2::S_BFE_I32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;
	Register full_reg;
	Register nonzero;

	// Load operands from registers.
	s0.as_uint = ReadSReg(opcode.ssrc0);
	full_reg.as_uint = ReadSReg(opcode.ssrc1);

	/* s1 (offset) should be [4:0] of ssrc1 and s2 (width) should be [22:16] of ssrc1*/
	s1.as_uint = full_reg.as_uint & 0x1F;
	s2.as_uint = (full_reg.as_uint >> 16) & 0x7F;

	// Calculate the result.
	if (s2.as_uint == 0)
	{
		result.as_int = 0;
	}
	else if (s2.as_uint + s1.as_uint < 32)
	{
		result.as_int = (s0.as_int << (32 - s1.as_uint - s2.as_uint)) >> 
			(32 - s2.as_uint);
	}
	else
	{
		result.as_int = s0.as_int >> s1.as_uint;
	}

	nonzero.as_uint = result.as_uint != 0;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);


	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", opcode.sdst, result.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}



