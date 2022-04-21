#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VOP2
#define INST InstructionVOP2

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
	/* 0xFF indicates the use of a literal constant as a
	 * source operand. */
	if (OPCODE.src0 == 0xFF) {
		m_size = 8;
	} else if (OPCODE.op == 32) {
	 /* the instruction */
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
}

void INST::print() {
    printf("op_enc(%lx): %s", info.op, opcode_str[info.op].c_str());
    printf("\tv%d", OPCODE.vdst);
    uint32_t src0;
	// Load operand from register or as a literal constant.
	if (OPCODE.src0 == 0xFF)
        printf(" v%d", OPCODE.lit_const);
	else {
        if (OPCODE.ssrc0_ == 0)
            printf(", s%d", OPCODE.src0);
        else
            printf(", v%d", OPCODE.src0);
    }
    printf(", v%d\n", OPCODE.vsrc1);
}

void INST::dumpExecBegin(WarpState *w) {
}

void INST::dumpExecEnd(WarpState *w) {
}

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
void INST::V_CNDMASK_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	int vcci;

	// Load operands from register or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);
	vcci = ReadBitmaskSReg(RegisterVcc, lane_id);

	// Calculate the result.
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f + S1.f.
void INST::V_ADD_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the sum.
	sum.as_float = s0.as_float + s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);

}

// D.f = S0.f - S1.f.
void INST::V_SUB_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference.
	dif.as_float = s0.as_float - s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);

}

// D.f = S1.f - S0.f.
void INST::V_SUBREV_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference.
	dif.as_float = s1.as_float - s0.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);

}

// D.f = S0.F * S1.f + D.f.
// void INST::V_MAC_LEGACY_F32(WarpState *item, uint32_t lane_id)


// D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
/*
void INST::V_MUL_LEGACY_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1);

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

}
*/

// D.f = S0.f * S1.f.
void INST::V_MUL_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the product.
	product.as_float = s0.as_float * s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, product.as_uint, lane_id);

}

// D.i = S0.i[23:0] * S1.i[23:0].
void INST::V_MUL_I32_I24(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend(s0.as_uint, 24);
	s1.as_uint = SignExtend(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
	WriteVReg(OPCODE.vdst, product.as_uint, lane_id);

}

void INST::V_MULLO_I32_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register product;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend(s0.as_uint, 24);
	s1.as_uint = SignExtend(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
	WriteVReg(OPCODE.vdst, product.as_uint, lane_id);

}

// D.f = min(S0.f, S1.f).
void INST::V_MIN_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register min;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the minimum operand.
	if (s0.as_float < s1.as_float)
	{
		min.as_float = s0.as_float;
	}
	else
	{
		min.as_float = s1.as_float;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, min.as_uint, lane_id);

}

// D.f = max(S0.f, S1.f).
void INST::V_MAX_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register max;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the minimum operand.
	if (s0.as_float > s1.as_float)
	{
		max.as_float = s0.as_float;
	}
	else
	{
		max.as_float = s1.as_float;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, max.as_uint, lane_id);

}

// D.i = max(S0.i, S1.i).
void INST::V_MAX_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register max;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the minimum operand.
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
	}
	else
	{
		max.as_int = s1.as_int;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, max.as_uint, lane_id);

}

// D.i = min(S0.i, S1.i).
void INST::V_MIN_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register min;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the minimum operand.
	if (s0.as_int < s1.as_int)
	{
		min.as_int = s0.as_int;
	}
	else
	{
		min.as_int = s1.as_int;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, min.as_uint, lane_id);

}

// D.u = min(S0.u, S1.u).
void INST::V_MIN_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register min;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the minimum operand.
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
	}
	else
	{
		min.as_uint = s1.as_uint;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, min.as_uint, lane_id);

}

// D.u = max(S0.u, S1.u).
void INST::V_MAX_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register max;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the maximum operand.
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
	}
	else
	{
		max.as_uint = s1.as_uint;
	}

	// Write the results.
	WriteVReg(OPCODE.vdst, max.as_uint, lane_id);

}

// D.u = S1.u >> S0.u[4:0].
void INST::V_LSHRREV_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
	{
		assert(OPCODE.lit_const < 32);
		s0.as_uint = OPCODE.lit_const;
	}
	else
	{
		s0.as_uint = ReadReg(OPCODE.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Right shift s1 by s0.
	result.as_uint = s1.as_uint >> s0.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.i = S1.i >> S0.i[4:0].
void INST::V_ASHRREV_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
	{
		assert(OPCODE.lit_const < 32);
		s0.as_uint = OPCODE.lit_const & 0x1F;
	}
	else
	{
		s0.as_uint = ReadReg(OPCODE.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Right shift s1 by s0.
	result.as_int = s1.as_int >> s0.as_int;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S0.u << S1.u[4:0].
void INST::V_LSHL_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id) & 0x1F;

	// Left shift s1 by s0.
	result.as_uint = s0.as_uint << s1.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S1.u << S0.u[4:0].
void INST::V_LSHLREV_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
	{
		assert(OPCODE.lit_const < 32);
		s0.as_uint = OPCODE.lit_const;
	}
	else
	{
		s0.as_uint = ReadReg(OPCODE.src0) & 0x1F;
	}
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Left shift s1 by s0.
	result.as_uint = s1.as_uint << s0.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S0.u & S1.u.
void INST::V_AND_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
	{
		s0.as_uint = OPCODE.lit_const;
	}
	else
	{
		s0.as_uint = ReadReg(OPCODE.src0);
	}
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint & s1.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S0.u | S1.u.
void INST::V_OR_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint | s1.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = S0.u ^ S1.u.
void INST::V_XOR_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint ^ s1.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

//D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset.
void INST::V_BFM_B32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = S0.f * S1.f + D.f.
void INST::V_MAC_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dst;
	Register result;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);
	dst.as_uint = ReadVReg(OPCODE.vdst, lane_id);

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float + dst.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = S0.f * K + S1.f; K is a 32-bit inline constant
void INST::V_MADMK_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register K;
	Register dst;

	// Load operands from registers or as a literal constant.
	s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);
	K.as_uint = OPCODE.lit_const;

	// Calculate the result
	dst.as_float = s0.as_float * K.as_float + s1.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, dst.as_uint, lane_id);

}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADD_I32_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the sum and carry.
	sum.as_int = s0.as_int + s1.as_int;
	carry.as_uint =
		! !(((long long) s0.as_int + (long long) s1.as_int) >> 32);

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADD_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;
	// Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint;
	// carry.as_uint =
	//	! !(((long long) s0.as_uint + (long long) s1.as_uint) >> 32);

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);
	// WriteBitmaskSReg(RegisterVcc, carry.as_uint);

}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADDCO_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register sum;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint =
		! !(((long long) s0.as_uint + (long long) s1.as_uint) >> 32);

	// Write the results.
	WriteVReg(OPCODE.vdst, sum.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

// D.u = S0.u - S1.u; vcc = carry-out.
void INST::V_SUB_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference and carry.
	dif.as_uint = s0.as_int - s1.as_int;
	carry.as_uint = (s1.as_int > s0.as_int);

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

// D.u = S0.u - S1.u; vcc = carry-out.
void INST::V_SUB_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference and carry.
	dif.as_uint = s0.as_uint - s1.as_uint;
	carry.as_uint = (s1.as_uint > s0.as_uint);

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

// D.u = S1.u - S0.u; vcc = carry-out.
void INST::V_SUBREV_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference and carry.
	dif.as_int = s1.as_int - s0.as_int;
	carry.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

void INST::V_SUBREV_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register dif;
	Register carry;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Calculate the difference and carry.
	dif.as_uint = s1.as_uint - s0.as_uint;
	carry.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	WriteVReg(OPCODE.vdst, dif.as_uint, lane_id);
	WriteBitmaskSReg(RegisterVcc, carry.as_uint, lane_id);

}

// D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero.
/*
void INST::V_CVT_PKRTZ_F16_F32(WarpState *item, uint32_t lane_id)
{
	union hfpack
	{
		unsigned as_uint32;
		struct
		{
			unsigned short s1f;
			unsigned short s0f;
		} as_f16f16;
	};

	Register s0;
	Register s1;
	unsigned short s0f;
	unsigned short s1f;
	union hfpack float_pack;

	// Load operands from registers or as a literal constant.
	if (OPCODE.src0 == 0xFF)
		s0.as_uint = OPCODE.lit_const;
	else
		s0.as_uint = ReadReg(OPCODE.src0);
	s1.as_uint = ReadVReg(OPCODE.vsrc1);

	// Convert to half float
	s0f = Float32to16(s0.as_float);
	s1f = Float32to16(s1.as_float);
	float_pack.as_f16f16.s0f = s0f;
	float_pack.as_f16f16.s1f = s1f;

	// Write the results.
	WriteVReg(OPCODE.vdst, float_pack.as_uint32);

}
*/

