#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "inc/OperandUtil.h"
#include "inc/ExecTypes.h"
#include "inc/ExecCommon.h"
#include "common/DataTypes.h"
#include <cmath>
#include <limits>

#define OPCODE bytes.VOP1
#define INST InstructionVOP1

// TODO auto generate it from coasm
void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    is_VOP1 = true;
	if (OPCODE.ext.e0_.ext_enc == COMMON_ENC_ext1_enc) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
    num_dst_operands = 1;
    num_src_operands = 1;
    uint32_t src_reg_range = 1;
    uint32_t dst_reg_range = 1;
    if (info.op == OpcodeVOP1::V_CVT_F64_I32 ||
        info.op == OpcodeVOP1::V_CVT_F64_F32 ||
        info.op == OpcodeVOP1::V_CVT_F64_U32 ||
        info.op == OpcodeVOP1::V_SEXT_I64_I32 ||
        info.op == OpcodeVOP1::V_ZEXT_B64_B32 ||
        info.op == OpcodeVOP1::V_CVTA_SHARED_TO_FLAT
        ) {
        dst_reg_range = 2;
    }

    if (info.op == OpcodeVOP1::V_CVT_F32_F64 ||
        info.op == OpcodeVOP1::V_CVTA_SHARED_TO_FLAT) {
        src_reg_range = 2;
    }

    dsrc0_Decode(this, OPCODE.imm_, OPCODE.dsrc0_, OPCODE.src0, m_size, OPCODE.ext.e1_.imm, OPCODE.ext.e2_.src0, src_reg_range, Operand::SRC0);
#if 0
    // FIXME m_size == 8
    if (OPCODE.imm_ == 0x1) {
        uint32_t imm =  (OPCODE.dsrc0_ << COMMON_ENC_max_src0_32e_width) + OPCODE.src0;
        if (m_size == 8) {
            imm |= OPCODE.ext.e1_.imm << (COMMON_ENC_max_src0_32e_width  + 2);
        }
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0, imm);

    } else if (OPCODE.dsrc0_ == COMMON_ENC_dsrc0_l) {
	    int stride = 4;
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.src0, src_reg_range, Reg::Data, stride));
    } else if (OPCODE.dsrc0_ == COMMON_ENC_dsrc0_d) {
	    int stride = 0;
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.src0, src_reg_range, Reg::Data, stride));
    } else if (OPCODE.dsrc0_ == COMMON_ENC_dsrc0_s) {
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.src0, src_reg_range, Reg::Scalar));
    } else {
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.src0, src_reg_range, Reg::Vector));
    }

    operands_[Operand::DST] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.vdst, dst_reg_range, Reg::Vector));
#endif
    if (m_size == 8) {
        makeOperand(Operand::DST,
                Reg(OPCODE.vdst | (OPCODE.ext.e1_.vdst << COMMON_ENC_max_vdst_32e_width),
                    dst_reg_range, Reg::Vector));
    } else {
        makeOperand(Operand::DST, Reg(OPCODE.vdst, dst_reg_range, Reg::Vector));
    }
}

void INST::print() {
    Instruction::print();
    printVOP1(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}

// D.u = S0.u.
void INST::V_MOV_B32(WarpState *item, uint32_t lane_id)
{
	Register value = operands_[Operand::SRC0]->getValue(lane_id);
    operands_[Operand::DST]->setValue(value, lane_id);
}

// Copy one VGPR value to one SGPR.
void INST::V_READFIRSTLANE_B32(WarpState *item, uint32_t lane_id)
{
	Register value = operands_[Operand::SRC0]->getValue(lane_id);
    operands_[Operand::DST]->setValue(value, lane_id);
	// assert(OPCODE.ssrc0_ == 1 || OPCODE.src0 == RegisterM0);
    // value.as_uint = readSrc0(OPCODE, item, lane_id);
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

	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
    std::vector<Register> result(2);

	// Convert and separate value.
	value.as_double = (double) s0.as_int;

	// Write the results.
	result[0].as_uint = value.as_reg[0];
	result[1].as_uint = value.as_reg[1];
    operands_[Operand::DST]->setValueX(result, lane_id);
}

// D.f = (float)S0.i.
void INST::V_CVT_F32_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register value;
	value.as_float = (float) s0.as_int;

    operands_[Operand::DST]->setValue(value, lane_id);
}

// D.f = (float)S0.u.
void INST::V_CVT_F32_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register value;

	value.as_float = (float) s0.as_uint;
    operands_[Operand::DST]->setValue(value, lane_id);

}

// D.i = (uint)S0.f.
void INST::V_CVT_U32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
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
    operands_[Operand::DST]->setValue(value, lane_id);
}

// D.i = (int)S0.f.
void INST::V_CVT_I32_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
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
    operands_[Operand::DST]->setValue(value, lane_id);

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

    std::vector<Register> src = operands_[Operand::SRC0]->getValueX(lane_id);
	// Load operand from registers.
	s0.as_reg[0] = src[0].as_uint;
	s0.as_reg[1] = src[1].as_uint;

	// Cast to a single precision float
	value.as_float = (float) s0.as_double;

    operands_[Operand::DST]->setValue(value, lane_id);
}


// D.d = (double)S0.f.
void INST::V_CVT_F64_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	union
	{
		double as_double;
		unsigned as_reg[2];

	} value;
    std::vector<Register> result(2);

	// Cast to a single precision float
	value.as_double = (double) s0.as_float;

	// Write the results.
	result[0].as_uint = value.as_reg[0];
	result[1].as_uint = value.as_reg[1];
    operands_[Operand::DST]->setValueX(result, lane_id);
}

// D.d = (double)S0.u.
void INST::V_CVT_F64_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.f = trunc(S0.f), return integer part of S0.
void INST::V_TRUNC_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register value;

	// Truncate decimal portion
	value.as_float = (float)((int)s0.as_float);

    operands_[Operand::DST]->setValue(value, lane_id);

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
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register rcp;

	rcp.as_float = 1.0f / s0.as_float;

    operands_[Operand::DST]->setValue(rcp, lane_id);
}

// D.f = 1.0 / sqrt(S0.f).
void INST::V_RSQ_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register rsq;

	rsq.as_float = 1.0f / sqrt(s0.as_float);

    operands_[Operand::DST]->setValue(rsq, lane_id);
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
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register srt;

	// Load operand from register or as a literal constant.
	srt.as_float = sqrtf(s0.as_float);

	// Write the results.
    operands_[Operand::DST]->setValue(srt, lane_id);
}

// D.f = sin(S0.f)
void INST::V_SIN_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
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
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.f = cos(S0.f)
void INST::V_COS_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
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
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.u = ~S0.u.
void INST::V_NOT_B32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register result;

	// Bitwise not
	result.as_uint = ~s0.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);

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
	ISAUnimplemented(item);
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register m0;

	assert(OPCODE.src0 != 0xFF);

	m0.as_uint = item->getSreg(RegisterM0);

	// Write the results.
    operands_[Operand::DST]->setValue(m0, lane_id);
	// WriteVReg(OPCODE.vdst+m0.as_uint, s0.as_uint, lane_id);

}


// VGPR[D.u] = VGPR[S0.u + M0.u].
void INST::V_MOVRELS_B32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
	Register m0;
	assert(OPCODE.src0 != 0xFF);

	// Load operand from register or as a literal constant.
	m0.as_uint = item->getSreg(RegisterM0);
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
    // s0.as_uint = readSrc0(OPCODE, item, lane_id, m0.as_uint);
	// s0.as_uint = item->getSreg(OPCODE.src0 + OPCODE.ssrc0_ * 0x100 + m0.as_uint);

	// Write the results.
    operands_[Operand::DST]->setValue(s0, lane_id);
}

// D.i = (I32)S0.i.
void INST::V_SEXT_I32_I8(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = sext(x, 8);

    operands_[Operand::DST]->setValue(x, lane_id);
}

// D.i = (I32)S0.i.
void INST::V_SEXT_I32_I16(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = sext(x, 16);

    operands_[Operand::DST]->setValue(x, lane_id);
}

// D.i = (double)S0.i.
void INST::V_SEXT_I64_I32(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = sext(x, 32);

    operands_[Operand::DST]->setValue(x, lane_id);
}

void INST::V_ZEXT_B32_B8(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = zext(x, 8);

    operands_[Operand::DST]->setValue(x, lane_id);
}

void INST::V_ZEXT_B32_B16(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = zext(x, 16);

    operands_[Operand::DST]->setValue(x, lane_id);
}

void INST::V_ZEXT_B64_B32(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = zext(x, 32);

    operands_[Operand::DST]->setValue(x, lane_id);
}

void INST::V_CHOP_B16_B32(WarpState *w, uint32_t lane_id)
{
	// Load operand from register or as a literal constant.
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x = chop(x, 16);
    operands_[Operand::DST]->setValue(x, lane_id);
}

// D.i = (int)S0.d.
void INST::V_CVTA_SHARED_TO_FLAT(WarpState *w, uint32_t lane_id)
{
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x.u64 = shared_to_generic(0, x.u64);
    operands_[Operand::DST]->setValue(x, lane_id);
}

void INST::V_CVTA_FLAT_TO_GLOBAL(WarpState *w, uint32_t lane_id)
{
    reg_t x = operands_[Operand::SRC0]->getRegValue(lane_id);
    x.u64 = generic_to_global(x.u64);
    operands_[Operand::DST]->setValue(x, lane_id);
}

