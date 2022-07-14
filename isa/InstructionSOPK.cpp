#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.SOPK
#define INST InstructionSOPK

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    bytes.word[1] = 0;
    m_size = 4;
    m_is_warp_op = true;
    m_decoded = true;
}

void INST::print() {
    Instruction::print();
    printSOPK(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}


// D.i = signext(simm16).
void INST::S_MOVK_I32(WarpState *item, uint32_t lane_id)
{
	Register simm16;
	Register result;

	// Load constant operand from instruction.
	simm16.as_ushort[0] = OPCODE.simm16;

	// Sign extend the short constant to an integer.
	result.as_int = (int) simm16.as_short[0];

	// Write the results.
	// Store the data in the destination register
	WriteSReg(OPCODE.sdst, result.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", OPCODE.sdst, result.as_int);
}

//
void INST::S_CMPK_LE_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.i = D.i + signext(SIMM16). scc = overflow.
void INST::S_ADDK_I32(WarpState *item, uint32_t lane_id)
{
	Register simm16;
	Register sum;
	Register ovf;
	Register dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = OPCODE.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(OPCODE.sdst);

	// Add the two operands and determine overflow.
	sum.as_int = dst.as_int + se_simm16;
	ovf.as_uint = (dst.as_int >> 31 != se_simm16 >> 31) ? 0 :
		((dst.as_int > 0 && sum.as_int < 0) || 
		 (dst.as_int < 0 && sum.as_int > 0));

	// Write the results.
	WriteSReg(OPCODE.sdst, sum.as_uint);
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", OPCODE.sdst, sum.as_int);
//		Emulator::isa_debug << misc::fmt("scc<=(%u)", ovf.as_uint);
}

// D.i = D.i * signext(SIMM16). scc = overflow.
void INST::S_MULK_I32(WarpState *item, uint32_t lane_id)
{
	Register simm16;
	Register product;
	Register ovf;
	Register dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = OPCODE.simm16;
	se_simm16 = (int) simm16.as_short[0];

	// Load operand from destination register.
	dst.as_uint = ReadSReg(OPCODE.sdst);

	// Multiply the two operands and determine overflow.
	product.as_int = dst.as_int * se_simm16;
	ovf.as_uint = ((long long) dst.as_int * (long long) se_simm16) > 
		(long long) product.as_int;

	// Write the results.
	WriteSReg(OPCODE.sdst, product.as_uint);
	WriteSReg(RegisterScc, ovf.as_uint);

	// Print isa debug information.
//		Emulator::isa_debug << misc::fmt("S%u<=(%d)", OPCODE.sdst, product.as_int);
//		Emulator::isa_debug << misc::fmt("scc<=(%u)", ovf.as_uint);
}


