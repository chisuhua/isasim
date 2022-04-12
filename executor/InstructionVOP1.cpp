#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "inc/OperandUtil.h"
#include <cmath>
#include <limits>

#define opcode bytes.VOP1

// TODO auto generate it from coasm
void InstructionVOP1::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
	if (opcode.src0 == 0xFF) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
    // FIXME on E32 format 
}

void InstructionVOP1::print() {
    printf("op_enc(%lx): %s ", info.op, opcode_str[info.op].c_str());
    printf("\tv%d", opcode.vdst);
    uint32_t src0;
	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
        printf(" v%d", opcode.lit_const);
	else {
        if (opcode.ssrc0_ == 0)
            printf(", s%d", opcode.src0);
        else
            printf(", v%d", opcode.src0);
    }
    printf("\n");
}

// Do nothing
void InstructionVOP1::V_NOP(WarpState *item, uint32_t lane_id)
{
	// Do nothing
}

// D.u = S0.u.
void InstructionVOP1::V_MOV_B32(WarpState *item, uint32_t lane_id)
{
	Register value;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		value.as_uint = opcode.lit_const;
	else {
        if (opcode.ssrc0_ == 0)
		    value.as_uint = ReadSReg(opcode.src0);
        else
		    value.as_uint = ReadVReg(opcode.src0, lane_id);
    }

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// Copy one VGPR value to one SGPR.
void InstructionVOP1::V_READFIRSTLANE_B32(WarpState *item, uint32_t lane_id)
{
	Register value;

	// Load operand from register.
	assert(opcode.ssrc0_ == 1 || opcode.src0 == RegisterM0);
    if (opcode.ssrc0_ == 1) {
	    value.as_uint = ReadVReg(opcode.src0, lane_id);
    } else {
	    value.as_uint = ReadSReg(opcode.src0);
    }

	// Write the results.
	// Store the data in the destination register
	WriteSReg(opcode.vdst, value.as_uint);

}

// D.i = (int)S0.d.
void InstructionVOP1::V_CVT_I32_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = (double)S0.i.
void InstructionVOP1::V_CVT_F64_I32(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	Register s0;
	Register result_lo;
	Register result_hi;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else {
	    s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);
    }


	// Convert and separate value.
	value.as_double = (double) s0.as_int;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(opcode.vdst, result_lo.as_uint, lane_id);
	WriteVReg(opcode.vdst + 1, result_hi.as_uint, lane_id);

}

// D.f = (float)S0.i.
void InstructionVOP1::V_CVT_F32_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register value;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else {
        if (opcode.ssrc0_ == 0)
		    s0.as_uint = ReadSReg(opcode.src0);
        else
		    s0.as_uint = ReadVReg(opcode.src0, lane_id);
    }

	value.as_float = (float) s0.as_int;

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// D.f = (float)S0.u.
void InstructionVOP1::V_CVT_F32_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 ;
	Register value;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else {
        if (opcode.ssrc0_ == 0)
		    s0.as_uint = ReadSReg(opcode.src0);
        else
		    s0.as_uint = ReadVReg(opcode.src0, lane_id);
    }


	value.as_float = (float) s0.as_uint;

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// D.i = (uint)S0.f.
void InstructionVOP1::V_CVT_U32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register value;

	float fvalue;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else {
        if (opcode.ssrc0_ == 0)
		    s0.as_uint = ReadSReg(opcode.src0);
        else
		    s0.as_uint = ReadVReg(opcode.src0, lane_id);
    }

	fvalue = s0.as_float;

	// Handle special number cases and cast to an unsigned

	// -inf, NaN, 0, -0 --> 0
	if ((std::isinf(fvalue) && fvalue < 0.0f) || std::isnan(fvalue)
		|| fvalue == 0.0f || fvalue == -0.0f)
		value.as_uint = 0;
	// inf, > max_uint --> max_uint
	else if (std::isinf(fvalue) || fvalue >= std::numeric_limits<unsigned int>::max())
		value.as_uint = std::numeric_limits<unsigned int>::max();
	else
		value.as_uint = (unsigned) fvalue;

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// D.i = (int)S0.f.
void InstructionVOP1::V_CVT_I32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register value;

	float fvalue;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else {
        if (opcode.ssrc0_ == 0)
		    s0.as_uint = ReadSReg(opcode.src0);
        else
		    s0.as_uint = ReadVReg(opcode.src0, lane_id);
    }


	fvalue = s0.as_float;

	// Handle special number cases and cast to an int

	// inf, > max_int --> max_int
	if ((std::isinf(fvalue) && fvalue > 0.0f) || fvalue >= std::numeric_limits<int>::max())
		value.as_int = std::numeric_limits<int>::max();
	// -inf, < -max_int --> -max_int
	else if (std::isinf(fvalue) || fvalue < std::numeric_limits<int>::min())
		value.as_int = std::numeric_limits<int>::min();
	// NaN, 0, -0 --> 0
	else if (std::isnan(fvalue) || fvalue == 0.0f || fvalue == -0.0f)
		value.as_int = 0;
	else
		value.as_int = (int) fvalue;

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// D.f = (float)S0.d.
void InstructionVOP1::V_CVT_F32_F64(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0;
	Register value;

	assert(opcode.src0 != 0xFF);

	// Load operand from registers.
	s0.as_reg[0] = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);
	s0.as_reg[1] = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100 + 1);

	// Cast to a single precision float
	value.as_float = (float) s0.as_double;

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}


// D.d = (double)S0.f.
void InstructionVOP1::V_CVT_F64_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	Register value_lo;
	Register value_hi;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	// Cast to a single precision float
	value.as_double = (double) s0.as_float;

	// Write the results.
	value_lo.as_uint = value.as_reg[0];
	value_hi.as_uint = value.as_reg[1];
	WriteVReg(opcode.vdst, value_lo.as_uint, lane_id);
	WriteVReg(opcode.vdst + 1, value_hi.as_uint, lane_id);

}

// D.d = (double)S0.u.
void InstructionVOP1::V_CVT_F64_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = trunc(S0.f), return integer part of S0.
void InstructionVOP1::V_TRUNC_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register value;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	// Truncate decimal portion
	value.as_float = (float)((int)s0.as_float);

	// Write the results.
	WriteVReg(opcode.vdst, value.as_uint, lane_id);

}

// D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0.
void InstructionVOP1::V_FLOOR_F32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = log2(S0.f).
void InstructionVOP1::V_LOG_F32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = 1.0 / S0.f.
void InstructionVOP1::V_RCP_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register rcp;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	rcp.as_float = 1.0f / s0.as_float;

	// Write the results.
	WriteVReg(opcode.vdst, rcp.as_uint, lane_id);

}

// D.f = 1.0 / sqrt(S0.f).
void InstructionVOP1::V_RSQ_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register rsq;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	rsq.as_float = 1.0f / sqrt(s0.as_float);

	// Write the results.
	WriteVReg(opcode.vdst, rsq.as_uint, lane_id);

}


// D.d = 1.0 / (S0.d).
void InstructionVOP1::V_RCP_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = 1.0 / sqrt(S0.f).
void InstructionVOP1::V_RSQ_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = sqrt(S0.f).
void InstructionVOP1::V_SQRT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register srt;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	srt.as_float = sqrtf(s0.as_float);

	// Write the results.
	WriteVReg(opcode.vdst, srt.as_uint, lane_id);

}

// D.f = sin(S0.f)
void InstructionVOP1::V_SIN_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register result;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	// Normalize input
	// XXX Should it be module instead of dividing?
	s0.as_float = s0.as_float * (2 * M_PI);

	if (inRange(s0.as_float, -256, 256))
	{
		result.as_float = sinf(s0.as_float);
	}
	else
	{
		assert(0); // Haven't debugged this yet
		result.as_float = 0;
	}

	// Write the results.
	WriteVReg(opcode.vdst, result.as_uint, lane_id);

}

// D.f = cos(S0.f)
void InstructionVOP1::V_COS_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register result;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	// Normalize input
	// XXX Should it be module instead of dividing?
	s0.as_float = s0.as_float * (2 * M_PI);

	if (inRange(s0.as_float, -256, 256))
	{
		result.as_float = cosf(s0.as_float);
	}
	else
	{
		assert(0); // Haven't debugged this yet
		result.as_float = 1;
	}

	// Write the results.
	WriteVReg(opcode.vdst, result.as_uint, lane_id);

}

// D.u = ~S0.u.
void InstructionVOP1::V_NOT_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register result;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);

	// Bitwise not
	result.as_uint = ~s0.as_uint;

	// Write the results.
	WriteVReg(opcode.vdst, result.as_uint, lane_id);

}

// D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if S0==0.
void InstructionVOP1::V_FFBH_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.d = FRAC64(S0.d);
void InstructionVOP1::V_FRACT_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// VGPR[D.u + M0.u] = VGPR[S0.u].
void InstructionVOP1::V_MOVRELD_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register m0;

	assert(opcode.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);
	m0.as_uint = ReadReg(RegisterM0);

	// Write the results.
	WriteVReg(opcode.vdst+m0.as_uint, s0.as_uint, lane_id);

}


// VGPR[D.u] = VGPR[S0.u + M0.u].
void InstructionVOP1::V_MOVRELS_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register m0;

	assert(opcode.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	m0.as_uint = ReadReg(RegisterM0);
	if (opcode.src0 == 0xFF)
		s0.as_uint = opcode.lit_const;
	else
		s0.as_uint = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100 + m0.as_uint);

	// Write the results.
	WriteVReg(opcode.vdst, s0.as_uint, lane_id);

}

// D.i = (double)S0.i.
void InstructionVOP1::V_SEXT_I64_I32(WarpState *item, uint32_t lane_id)
{
    Int s0;

	// Load operand from register or as a literal constant.
	if (opcode.src0 == 0xFF)
		s0.u32 = opcode.lit_const;
	else {
	    s0.u32 = ReadReg(opcode.src0 + opcode.ssrc0_ * 0x100);
    }

    Long value;
    value.i64 = SEXT_I64_I32(s0.i32);

	// Write the results.
	WriteVReg(opcode.vdst, value.int32[0].u32, lane_id);
	WriteVReg(opcode.vdst + 1, value.int32[1].u32, lane_id);

}

