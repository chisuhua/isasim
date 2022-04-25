#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VOP3B
#define INST InstructionVOP3B

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    m_size = 8;
}

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}

void INST::OperandCollect(WarpState *w) {
}

void INST::WriteBack(WarpState *w) {
}


/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out,
 * S2.u=carry-in). */
void INST::V_ADDC_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;
	Register carry_in;
	Register carry_out;

    // assert(!OPCODE.omod);
	// assert(!OPCODE.neg);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);
	carry_in.as_uint = ReadBitmaskSReg(OPCODE.ssrc2, lane_id);

	// Calculate sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint + carry_in.as_uint;
	carry_out.as_uint =
		! !(((unsigned long long) s0.as_uint +
			(unsigned long long) s1.as_uint +
			(unsigned long long) carry_in.as_uint) >> 32);

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);
	WriteBitmaskSReg(OPCODE.sdst, carry_out.as_uint, lane_id);

}

/*
 *D.d = Special case divide preop and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator)
 *s0 must equal s1 or s2.
 */
/*
void INST::V_DIV_SCALE_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}
*/



