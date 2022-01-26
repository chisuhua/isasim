#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define opcode bytes.VOP3B

void InstructionVOP3B::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    m_size = 8;
}

/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out,
 * S2.u=carry-in). */
void InstructionVOP3B::V_ADDC_U32(ThreadItem *item)
{
	Register s0;
	Register s1;
	Register sum;
	Register carry_in;
	Register carry_out;

    // assert(!opcode.omod);
	// assert(!opcode.neg);

	// Load operands from registers.
	s0.as_uint = ReadReg(opcode.src0);
	s1.as_uint = ReadVReg(opcode.vsrc1);
	carry_in.as_uint = ReadBitmaskSReg(opcode.ssrc2);

	// Calculate sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint + carry_in.as_uint;
	carry_out.as_uint =
		! !(((unsigned long long) s0.as_uint +
			(unsigned long long) s1.as_uint +
			(unsigned long long) carry_in.as_uint) >> 32);

	// Write the results.
	WriteVReg(opcode.vdst, sum.as_uint);
	WriteBitmaskSReg(opcode.sdst, carry_out.as_uint);

}

/*
 *D.d = Special case divide preop and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator)
 *s0 must equal s1 or s2.
 */
/*
void InstructionVOP3B::V_DIV_SCALE_F64(ThreadItem *item)
{
	ISAUnimplemented(item);
}
*/



