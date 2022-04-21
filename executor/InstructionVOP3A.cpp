#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include <cmath>
#include <limits>

#define OPCODE bytes.VOP3A
#define INST InstructionVOP3A

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    m_size = 8;
}

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}
void INST::dumpExecBegin(WarpState *w) {
}

void INST::dumpExecEnd(WarpState *w) {
}


/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
void INST::V_CNDMASK_B32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	int vcci;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	vcci = ReadBitmaskSReg(OPCODE.src2, lane_id);

	// Perform "floating-point negation"
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Calculate the result.
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f + S1.f.
void INST::V_ADD_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Calculate the sum.
	sum.as_float = s0.as_float + s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);

}

// D.f = S1.f - S0.f
void INST::V_SUBREV_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register diff;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Calculate the diff.
	diff.as_float = s1.as_float - s0.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, diff.as_uint, lane_id);

}

// D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
/*
void INST::V_MUL_LEGACY_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Calculate the product.
	if (s0.as_float == 0.0f || s1.as_float == 0.0f)
	{
		product.as_float = 0.0f;
	}
	else
	{
		product.as_float = s0.as_float * s1.as_float;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, product.as_uint);

}*/

// D.f = S0.f * S1.f.
void INST::V_MUL_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0. * S1..
void INST::V_MUL_I32_I24_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend(s0.as_uint, 24);
	s1.as_uint = SignExtend(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
	WriteVReg(OPCODE.vdst, product.as_uint, lane_id);

}

// D.f = max(S0.f, S1.f).
void INST::V_MAX_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Calculate the result.
	result.as_float = (s0.as_float > s1.as_float) ?
		s0.as_float : s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f * S1.f + S2.f.
void INST::V_MAD_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (OPCODE.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	if (OPCODE.neg & 4)
		s2.as_float = -s2.as_float;

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float + s2.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S0.u[23:0] * S1.u[23:0] + S2.u[31:0].
void INST::V_MAD_U32_U24(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	s0.as_uint = s0.as_uint & 0x00FFFFFF;
	s1.as_uint = s1.as_uint & 0x00FFFFFF;

	// Calculate the result.
	result.as_uint = s0.as_uint * s1.as_uint + s2.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

/* D.u = (S0.u >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
void INST::V_BFE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

	// Calculate the result.
	result.as_uint = (s0.as_uint >> s1.as_uint) & ((1 << s2.as_uint) - 1);

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
void INST::V_BFE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

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

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.u & S1.u) | (~S0.u & S2.u).
void INST::V_BFI_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	// Calculate the result.
	result.as_uint = (s0.as_uint & s1.as_uint) |
		(~s0.as_uint & s2.as_uint);

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f * S1.f + S2.f
void INST::V_FMA_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (OPCODE.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	if (OPCODE.neg & 4)
		s2.as_float = -s2.as_float;

	// FMA
	result.as_float = (s0.as_float * s1.as_float) + s2.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.d = S0.d * S1.d + S2.d
void INST::V_FMA_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF.
void INST::V_ALIGNBIT_B32(WarpState *item, uint32_t lane_id)
{
	Register src2;
	Register result;

	union
	{
		unsigned long long as_b64;
		unsigned as_reg[2];

	} src;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	src.as_reg[0] = ReadReg(OPCODE.src1);
	src.as_reg[1] = ReadReg(OPCODE.src0);
	src2.as_uint = ReadReg(OPCODE.src2);
	src2.as_uint = src2.as_uint & 0x1F;

	// ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF.
	result.as_uint = (src.as_b64 >> src2.as_uint) & 0xFFFFFFFF;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

/*
 *D.d = Special case divide fixup and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator).  void INST::V_DIV_FIXUP_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}
 */

void INST::V_LSHL_B64(WarpState *item, uint32_t lane_id)
{
	// Input operands
	union
	{
		double as_double;
		unsigned int as_reg[2];
		unsigned int as_uint;
		unsigned long as_ulong;

	} s0, s1, dst;

	Register result_lo;
	Register result_hi;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(OPCODE.src0);
	s0.as_reg[1] = ReadReg(OPCODE.src0 + 1);
	s1.as_reg[0] = ReadReg(OPCODE.src1);
	s1.as_reg[1] = ReadReg(OPCODE.src1 + 1);

	// LSHFT_B64
	// Mask s1 to return s1[4:0]
	// to extract left shift right operand
	dst.as_ulong = s0.as_ulong << (s1.as_uint & 0x001F);

	// Write the results.
	// Cast uint32 to unsigned int
	result_lo.as_uint = (unsigned int)dst.as_reg[0];
	result_hi.as_uint = (unsigned int)dst.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);
}


// D.d = min(S0.d, S1.d).
void INST::V_MIN_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.d = max(S0.d, S1.d).
void INST::V_MAX_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = S0.u * S1.u.
void INST::V_MUL_LO_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Calculate the product.
	result.as_uint = s0.as_uint * s1.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

/*
 *D.d = Special case divide FMA with scale and flags(s0.d = Quotient, s1.d = Denominator,
 *s2.d = Numerator).
void INST::V_DIV_FMAS_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0].
void INST::V_TRIG_PREOP_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}
 */

// D.u = (S0.u * S1.u)>>32
void INST::V_MUL_HI_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Calculate the product and shift right.
	result.as_uint = (unsigned)
		(((unsigned long long)s0.as_uint *
		(unsigned long long)s1.as_uint) >> 32);

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.i = S0.i * S1.i.
void INST::V_MUL_LO_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Calculate the product.
	result.as_int = s0.as_int * s1.as_int;


	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f - floor(S0.f).
void INST::V_FRACT_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);

	// Apply negation modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	assert (!(OPCODE.abs & 2));
	assert (!(OPCODE.abs & 4));

	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	assert (!(OPCODE.neg & 2));
	assert (!(OPCODE.neg & 4));

	// Calculate the product.
	result.as_float = s0.as_float - floorf(s0.as_float);

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.f < S1.f).
void INST::V_CMP_LT_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.f == S1.f).
void INST::V_CMP_EQ_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// vcc = (S0.f <= S1.f).
void INST::V_CMP_LE_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = (S0.f > S1.f).
void INST::V_CMP_GT_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = !(S0.f <= S1.f).
void INST::V_CMP_NLE_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = !(S0.f == S1.f).
void INST::V_CMP_NEQ_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = !(S0.f < S1.f).
void INST::V_CMP_NLT_F32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (OPCODE.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_float = -s0.as_float;
	if (OPCODE.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// Comparison Operations
void INST::V_CMP_OP16_F64_VOP3A(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = (S0.i < S1.i).
void INST::V_CMP_LT_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i == S1.i).
void INST::V_CMP_EQ_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i <= S1.i).
void INST::V_CMP_LE_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i > S1.i).
void INST::V_CMP_GT_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i <> S1.i).
void INST::V_CMP_NE_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = !(s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i >= S1.i).
void INST::V_CMP_GE_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.i == S1.i). Also write EXEC
void INST::V_CMPX_EQ_I32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Apply absolute value modifiers.
	if (OPCODE.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (OPCODE.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(OPCODE.abs & 4));

	// Apply negation modifiers.
	if (OPCODE.neg & 1)
		s0.as_int = -s0.as_int;
	if (OPCODE.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(OPCODE.neg & 4));

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

	// Write EXEC
	WriteBitmaskSReg(RegisterExec, result.as_uint, lane_id);

}

// D = IEEE numeric class function specified in S1.u, performed on S0.d.
void INST::V_CMP_CLASS_F64_VOP3A(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = (S0.u < S1.u).
void INST::V_CMP_LT_U32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint < s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.u <= S1.u).
void INST::V_CMP_LE_U32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.u > S1.u).
void INST::V_CMP_GT_U32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

void INST::V_CMP_LG_U32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Calculate result.
	result.as_uint = ((s0.as_uint < s1.as_uint) ||
		(s0.as_uint > s1.as_uint));

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0.u >= S1.u).
void INST::V_CMP_GE_U32_VOP3A(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = (S0 < S1)
void INST::V_CMP_LT_U64_VOP3A(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// Max of three numbers.
void INST::V_MAX3_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register max;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	// Determine the max.
	// max3(s0, s1, s2) == s0
	if (s0.as_int >= s1.as_int && s0.as_int >= s2.as_int)
	{
		max.as_int = s0.as_int;
	}
	// max3(s0, s1, s2) == s1
	else if (s1.as_int >= s0.as_int && s1.as_int >= s2.as_int)
	{
		max.as_int = s1.as_int;
	}
	// max3(s0, s1, s2) == s2
	else if (s2.as_int >= s0.as_int && s2.as_int >= s1.as_int)
	{
		max.as_int = s2.as_int;
	}
	else
	{
		throw std::runtime_error("Max algorithm failed");
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, max.as_uint, lane_id);

}

// Median of three numbers.
void INST::V_MED3_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register s2;
	Register median;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadReg(OPCODE.src1);
	s2.as_uint = ReadReg(OPCODE.src2);

	// Determine the median.
	// max3(s0, s1, s2) == s0
	if (s0.as_int >= s1.as_int && s0.as_int >= s2.as_int)
	{
		// max(s1, s2)
		median.as_int = (s1.as_int >= s2.as_int) ?
			s1.as_int : s2.as_int;
	}
	// max3(s0, s1, s2) == s1
	else if (s1.as_int >= s0.as_int && s1.as_int >= s2.as_int)
	{
		// max(s0, s2)
		median.as_int = (s0.as_int >= s2.as_int) ?
			s0.as_int : s2.as_int;
	}
	// max3(s0, s1, s2) == s2
	else if (s2.as_int >= s0.as_int && s2.as_int >= s1.as_int)
	{
		// max(s0, s1)
		median.as_int = (s0.as_int >= s1.as_int) ?
			s0.as_int : s1.as_int;
	}
	else
	{
		throw std::runtime_error("Median algorithm failed");
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, median.as_uint, lane_id);

}

// D = S0.u >> S1.u[4:0].
void INST::V_LSHR_B64(WarpState *item, uint32_t lane_id)
{
	union
	{
		unsigned long long as_b64;
		unsigned as_reg[2];

	} s0, value;

	Register s1;
	Register result_lo;
	Register result_hi;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(OPCODE.src0);
	s0.as_reg[1] = ReadReg(OPCODE.src0 + 1);
	s1.as_uint = ReadReg(OPCODE.src1);
	s1.as_uint = s1.as_uint & 0x1F;

	// Shift s0.
	value.as_b64 = s0.as_b64 >> s1.as_uint;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);

}

// D = S0.u >> S1.u[4:0] (Arithmetic shift)
void INST::V_ASHR_I64(WarpState *item, uint32_t lane_id)
{
	union
	{
		long long as_i64;
		unsigned as_reg[2];

	} s0, value;

	Register s1;
	Register result_lo;
	Register result_hi;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(OPCODE.src0);
	s0.as_reg[1] = ReadReg(OPCODE.src0 + 1);
	s1.as_uint = ReadReg(OPCODE.src1);
	s1.as_uint = s1.as_uint & 0x1F;

	// Shift s0.
	value.as_i64 = s0.as_i64 >> s1.as_uint;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);

}

// D.d = S0.d + S1.d.
void INST::V_ADD_F64(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0, s1, value;

	Register result_lo;
	Register result_hi;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(OPCODE.src0);
	s0.as_reg[1] = ReadReg(OPCODE.src0 + 1);
	s1.as_reg[0] = ReadReg(OPCODE.src1);
	s1.as_reg[1] = ReadReg(OPCODE.src1 + 1);

	// Add the operands, take into account special number cases.

	// s0 == NaN64 || s1 == NaN64
	if (std::fpclassify(s0.as_double) == FP_NAN ||
		std::fpclassify(s1.as_double) == FP_NAN)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0,s1 == infinity
	else if (std::fpclassify(s0.as_double) == FP_INFINITE &&
		std::fpclassify(s1.as_double) == FP_INFINITE)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0,!s1 == infinity
	else if (std::fpclassify(s0.as_double) == FP_INFINITE)
	{
		// value <-- s0(+-infinity)
		value.as_double = s0.as_double;
	}
	// s1,!s0 == infinity
	else if (std::fpclassify(s1.as_double) == FP_INFINITE)
	{
		// value <-- s1(+-infinity)
		value.as_double = s1.as_double;
	}
	// s0 == +-denormal, +-0
	else if (std::fpclassify(s0.as_double) == FP_SUBNORMAL ||
		std::fpclassify(s0.as_double) == FP_ZERO)
	{
		// s1 == +-denormal, +-0
		if (std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s1.as_double) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (std::signbit(s0.as_double)
				&& std::signbit(s1.as_double))
				// value <-- -0
				value.as_double = -0;
			else
				// value <-- +0
				value.as_double = +0;
		// s1 == F
		else
			// value <-- s1
			value.as_double = s1.as_double;
	}
	// s1 == +-denormal, +-0
	else if (std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
		std::fpclassify(s1.as_double) == FP_ZERO)
	{
		// s0 == +-denormal, +-0
		if (std::fpclassify(s0.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s0.as_double) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (std::signbit(s0.as_double)
				&& std::signbit(s1.as_double))
				// value <-- -0
				value.as_double = -0;
			else
				// value <-- +0
				value.as_double = +0;
		// s0 == F
		else
			// value <-- s1
			value.as_double = s0.as_double;
	}
	// s0 && s1 == F
	else
	{
		value.as_double = s0.as_double + s1.as_double;
	}

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);

}

// D.d = S0.d * S1.d.
void INST::V_MUL_F64(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0, s1, value;

	Register result_lo;
	Register result_hi;

	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);

	// Load operands from registers.
	s0.as_reg[0] = ReadReg(OPCODE.src0);
	s0.as_reg[1] = ReadReg(OPCODE.src0 + 1);
	s1.as_reg[0] = ReadReg(OPCODE.src1);
	s1.as_reg[1] = ReadReg(OPCODE.src1 + 1);

	// Multiply the operands, take into account special number cases.

	// s0 == NaN64 || s1 == NaN64
	if (std::fpclassify(s0.as_double) == FP_NAN ||
		std::fpclassify(s1.as_double) == FP_NAN)
	{
		// value <-- NaN64
		value.as_double = NAN;
	}
	// s0 == +denormal, +0
	else if ((std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s1.as_double) == FP_ZERO) &&
		!std::signbit(s0.as_double))
	{
		// s1 == +-infinity
		if (std::isinf(s1.as_double))
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!std::signbit(s1.as_double))
			// value <-- +0
			value.as_double = +0;
		// s1 < 0
		else if (std::signbit(s1.as_double))
			// value <-- -0
			value.as_double = -0;
	}
	// s0 == -denormal, -0
	else if ((std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s1.as_double) == FP_ZERO) &&
		std::signbit(s0.as_double))
	{
		// s1 == +-infinity
		if (std::isinf(s1.as_double))
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!std::signbit(s1.as_double))
			// value <-- -0
			value.as_double = -0;
		// s1 < 0
		else if (std::signbit(s1.as_double))
			// value <-- +0
			value.as_double = +0;
	}
	// s0 == +infinity
	else if (std::fpclassify(s0.as_double) == FP_INFINITE &&
		!std::signbit(s0.as_double))
	{
		// s1 == +-denormal, +-0
		if (std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s1.as_double) == FP_ZERO)
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!std::signbit(s1.as_double))
			// value <-- +infinity
			value.as_double = +INFINITY;
		// s1 < 0
		else if (std::signbit(s1.as_double))
			// value <-- -infinity
			value.as_double = -INFINITY;
	}
	// s0 == -infinity
	else if (std::fpclassify(s0.as_double) == FP_INFINITE &&
		std::signbit(s0.as_double))
	{
		// s1 == +-denormal, +-0
		if (std::fpclassify(s1.as_double) == FP_SUBNORMAL ||
			std::fpclassify(s1.as_double) == FP_ZERO)
			// value <-- NaN64
			value.as_double = NAN;
		// s1 > 0
		else if (!std::signbit(s1.as_double))
			// value <-- -infinity
			value.as_double = -INFINITY;
		// s1 < 0
		else if (std::signbit(s1.as_double))
			// value <-- +infinity
			value.as_double = +INFINITY;
	}
	else
	{
		value.as_double = s0.as_double * s1.as_double;
	}

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);

}

// D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0].
void INST::V_LDEXP_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

