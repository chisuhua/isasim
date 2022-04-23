#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "inc/OperandUtil.h"
#include <cmath>
#include <limits>

#define OPCODE bytes.VOP1
#define INST InstructionVOP1

// TODO auto generate it from coasm
void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
	if (OPCODE.ext.e0_.ext_enc == 0x7) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
    is_VOP1 = true;
    num_dst_operands = 1;
    num_src_operands = 1;
    uint32_t reg_range = 1;

    operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.src0, reg_range, OPCODE.ssrc0_ == 1? Reg::Scalar : Reg::Vector));

    operands_[Operand::DST] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.vdst, reg_range, Reg::Vector));

}

void INST::print() {
    printf("decode as: %s(%lx), ", opcode_str[info.op].c_str(), info.op);
    printVOP1(OPCODE);
}

void INST::dumpExecBegin(WarpState *w) {
    Instruction::dumpExecBegin(w);
}

void INST::dumpExecEnd(WarpState *w) {
}


uint32_t readSrc0(Instruction::BytesVOP1 op, WarpState *item, uint32_t lane_id, uint32_t offset = 0) {
	// Load operand from register or as a literal constant.
    Register value;
    if (op.ssrc0_ == 0)
        value.as_uint = item->getSreg(op.src0 + offset);
    else
	    value.as_uint = item->getVreg(op.src0 + offset, lane_id);
    return value.as_uint;
}

// Do nothing
void INST::V_NOP(WarpState *item, uint32_t lane_id)
{
	// Do nothing
}

// D.u = S0.u.
void INST::V_MOV_B32(WarpState *item, uint32_t lane_id)
{
	Register value;
    value.as_uint = readSrc0(OPCODE, item, lane_id);

	// Write the results.
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);
}

// Copy one VGPR value to one SGPR.
void INST::V_READFIRSTLANE_B32(WarpState *item, uint32_t lane_id)
{
	Register value;
	// Load operand from register.
	assert(OPCODE.ssrc0_ == 1 || OPCODE.src0 == RegisterM0);
    value.as_uint = readSrc0(OPCODE, item, lane_id);

	// Write the results.
	// Store the data in the destination register
	WriteSReg(OPCODE.vdst, value.as_uint);

}

// D.i = (int)S0.d.
void INST::V_CVT_I32_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = (double)S0.i.
void INST::V_CVT_F64_I32(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register result_lo;
	Register result_hi;

	// Load operand from register or as a literal constant.
	// if (OPCODE.src0 == 0xFF)
	//	s0.as_uint = OPCODE.lit_const;
	//else {
	//    s0.as_uint = ReadReg(OPCODE.src0 + OPCODE.ssrc0_ * 0x100);
    //}


	// Convert and separate value.
	value.as_double = (double) s0.as_int;

	// Write the results.
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);

}

// D.f = (float)S0.i.
void INST::V_CVT_F32_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register value;

	value.as_float = (float) s0.as_int;

	// Write the results.
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}

// D.f = (float)S0.u.
void INST::V_CVT_F32_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register value;

	value.as_float = (float) s0.as_uint;
	// Write the results.
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}

// D.i = (uint)S0.f.
void INST::V_CVT_U32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register value;

	float fvalue;

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
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}

// D.i = (int)S0.f.
void INST::V_CVT_I32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register value;

	float fvalue;


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
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}

// D.f = (float)S0.d.
void INST::V_CVT_F32_F64(WarpState *item, uint32_t lane_id)
{
	union
	{
		double as_double;
		unsigned as_reg[2];

	} s0;
	Register value;

	// Load operand from registers.
	s0.as_reg[0] = readSrc0(OPCODE, item, lane_id);
	s0.as_reg[1] = readSrc0(OPCODE, item, lane_id, 1);

	// Cast to a single precision float
	value.as_float = (float) s0.as_double;

	// Write the results.
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}


// D.d = (double)S0.f.
void INST::V_CVT_F64_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
	Register value_lo;
	Register value_hi;

	// Cast to a single precision float
	value.as_double = (double) s0.as_float;

	// Write the results.
	value_lo.as_uint = value.as_reg[0];
	value_hi.as_uint = value.as_reg[1];
	WriteVReg(OPCODE.vdst, value_lo.as_uint, lane_id);
	WriteVReg(OPCODE.vdst + 1, value_hi.as_uint, lane_id);

}

// D.d = (double)S0.u.
void INST::V_CVT_F64_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = trunc(S0.f), return integer part of S0.
void INST::V_TRUNC_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register value;

	// Truncate decimal portion
	value.as_float = (float)((int)s0.as_float);

	// Write the results.
	WriteVReg(OPCODE.vdst, value.as_uint, lane_id);

}

// D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0.
void INST::V_FLOOR_F32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = log2(S0.f).
void INST::V_LOG_F32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = 1.0 / S0.f.
void INST::V_RCP_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register rcp;

	rcp.as_float = 1.0f / s0.as_float;

	// Write the results.
	WriteVReg(OPCODE.vdst, rcp.as_uint, lane_id);

}

// D.f = 1.0 / sqrt(S0.f).
void INST::V_RSQ_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register rsq;

	rsq.as_float = 1.0f / sqrt(s0.as_float);

	// Write the results.
	WriteVReg(OPCODE.vdst, rsq.as_uint, lane_id);

}


// D.d = 1.0 / (S0.d).
void INST::V_RCP_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = 1.0 / sqrt(S0.f).
void INST::V_RSQ_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = sqrt(S0.f).
void INST::V_SQRT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register srt;

	// Load operand from register or as a literal constant.
	srt.as_float = sqrtf(s0.as_float);

	// Write the results.
	WriteVReg(OPCODE.vdst, srt.as_uint, lane_id);

}

// D.f = sin(S0.f)
void INST::V_SIN_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register result;

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
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.f = cos(S0.f)
void INST::V_COS_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register result;

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
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = ~S0.u.
void INST::V_NOT_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register result;

	// Bitwise not
	result.as_uint = ~s0.as_uint;

	// Write the results.
	WriteVReg(OPCODE.vdst, result.as_uint, lane_id);

}

// D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if S0==0.
void INST::V_FFBH_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.d = FRAC64(S0.d);
void INST::V_FRACT_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// VGPR[D.u + M0.u] = VGPR[S0.u].
void INST::V_MOVRELD_B32(WarpState *item, uint32_t lane_id)
{
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id);
	Register m0;

	assert(OPCODE.src0 != 0xFF);

	m0.as_uint = item->getSreg(RegisterM0);

	// Write the results.
	WriteVReg(OPCODE.vdst+m0.as_uint, s0.as_uint, lane_id);

}


// VGPR[D.u] = VGPR[S0.u + M0.u].
void INST::V_MOVRELS_B32(WarpState *item, uint32_t lane_id)
{
	Register m0;

	assert(OPCODE.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	m0.as_uint = item->getSreg(RegisterM0);
	Register s0;
    s0.as_uint = readSrc0(OPCODE, item, lane_id, m0.as_uint);
	// s0.as_uint = item->getSreg(OPCODE.src0 + OPCODE.ssrc0_ * 0x100 + m0.as_uint);

	// Write the results.
	WriteVReg(OPCODE.vdst, s0.as_uint, lane_id);
}

// D.i = (double)S0.i.
void INST::V_SEXT_I64_I32(WarpState *item, uint32_t lane_id)
{
    Int s0;

	// Load operand from register or as a literal constant.
    s0.u32 = readSrc0(OPCODE, item, lane_id);

    Long value;
    value.i64 = SEXT_I64_I32(s0.i32);

	// Write the results.
	WriteVReg(OPCODE.vdst, value.int32[0].u32, lane_id);
	WriteVReg(OPCODE.vdst + 1, value.int32[1].u32, lane_id);
}

