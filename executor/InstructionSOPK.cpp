#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.SOPK

void InstructionSOPK::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    bytes.word[1] = 0;
    m_size = 4;
    m_is_warp_op = true;
}

// D.i = signext(simm16).
void InstructionSOPK::S_MOVK_I32(ThreadItem *item)
{
	Register simm16;
	Register result;

	// Load constant operand from instruction.
	simm16.as_ushort[0] = opcode.simm16;

	// Sign extend the short constant to an integer.
	result.as_int = (int) simm16.as_short[0];

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.sdst, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", opcode.sdst, result.as_int);
}

//
void InstructionSOPK::S_CMPK_LE_U32(ThreadItem *item)
{
	ISAUnimplemented(item);
}

// D.i = D.i + signext(SIMM16). scc = overflow.
void InstructionSOPK::S_ADDK_I32(ThreadItem *item)
{
	Register simm16;
	Register sum;
	Register ovf;
	Register dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = opcode.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(opcode.sdst);

	// Add the two operands and determine overflow.
	sum.as_int = dst.as_int + se_simm16;
	ovf.as_uint = (dst.as_int >> 31 != se_simm16 >> 31) ? 0 :
		((dst.as_int > 0 && sum.as_int < 0) || 
		 (dst.as_int < 0 && sum.as_int > 0));

	// Write the results.
		// Store the data in the destination register
	WriteSReg(opcode.sdst, sum.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", opcode.sdst, sum.as_int);
//		Emulator::isa_debug << misc::fmt("scc<=(%u)", ovf.as_uint);
}

// D.i = D.i * signext(SIMM16). scc = overflow.
void InstructionSOPK::S_MULK_I32(ThreadItem *item)
{
	Register simm16;
	Register product;
	Register ovf;
	Register dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = opcode.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(opcode.sdst);

	// Multiply the two operands and determine overflow.
	product.as_int = dst.as_int * se_simm16;
	ovf.as_uint = ((long long) dst.as_int * (long long) se_simm16) > 
		(long long) product.as_int;

	// Write the results.
		// Store the data in the destination register
	WriteSReg(opcode.sdst, product.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", opcode.sdst, product.as_int);
//		Emulator::isa_debug << misc::fmt("scc<=(%u)", ovf.as_uint);
}


