#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include <cmath>
#include <limits>

#define OPCODE bytes.VOP2
#define INST InstructionVOP2

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    is_VOP2 = true;

	if (OPCODE.ext.e0_.ext_enc == 0x7) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
    num_dst_operands = 1;
    num_src_operands = 2;
    uint32_t src0_reg_range = 1;
    uint32_t src1_reg_range = 1;
    uint32_t dst_reg_range = 1;

    if (info.op == OpcodeVOP2::V_MAC_F32
        ) {
        num_src_operands = 3;
        operands_[Operand::SRC2] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.vdst, dst_reg_range, Reg::Vector));
    }

    if (info.op == OpcodeVOP2::V_LSHL_B64 ||
        info.op == OpcodeVOP2::V_ASHR_I64
            ) {
        src0_reg_range = 2;
        dst_reg_range = 2;
    }
    if (info.op == OpcodeVOP2::V_ADD_F64 ||
        info.op == OpcodeVOP2::V_ADD_I64) {
        src0_reg_range = 2;
        src1_reg_range = 2;
        dst_reg_range = 2;
    }
    if (info.op == OpcodeVOP2::V_MUL_I64_I32) {
        dst_reg_range = 2;
    }

    dsrc0_Decode(this, OPCODE.imm_, OPCODE.dsrc0_, OPCODE.src0, src1_reg_range, Operand::SRC1);
    makeOperand(Operand::SRC0, Reg(OPCODE.vsrc1, src0_reg_range, Reg::Vector));
    makeOperand(Operand::DST, Reg(OPCODE.vdst, dst_reg_range, Reg::Vector));

}

void INST::print() {
    Instruction::print();
    printVOP2(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
void INST::V_CNDMASK_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);

	int vcci = w->getBitmaskSreg(RegisterVcc, lane_id);

	Register result;
	// Calculate the result.
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.f = S0.f + S1.f.
void INST::V_ADD_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register sum;

	// Calculate the sum.
	sum.as_float = s0.as_float + s1.as_float;

	// Write the results.
    operands_[Operand::DST]->setValue(sum, lane_id);
}

// D.f = S0.f - S1.f.
void INST::V_SUB_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;

	// Calculate the difference.
	dif.as_float = s0.as_float - s1.as_float;

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);

}

// D.f = S1.f - S0.f.
void INST::V_SUBREV_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;

	// Calculate the difference.
	dif.as_float = s1.as_float - s0.as_float;

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);
}

// D.f = S0.F * S1.f + D.f.
// void INST::V_MAC_LEGACY_F32(WarpState *w, uint32_t lane_id)


// D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
/*
void INST::V_MUL_LEGACY_F32(WarpState *w, uint32_t lane_id)
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
void INST::V_MUL_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register product;

	// Calculate the product.
	product.as_float = s0.as_float * s1.as_float;

	// Write the results.
    operands_[Operand::DST]->setValue(product, lane_id);
}

// D.i = S0.i[23:0] * S1.i[23:0].
void INST::V_MUL_I32_I24(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register product;

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend(s0.as_uint, 24);
	s1.as_uint = SignExtend(s1.as_uint, 24);

	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
    operands_[Operand::DST]->setValue(product, lane_id);
}

void INST::V_MUL_I64_I32(WarpState *w, uint32_t lane_id)
{
	// Load operands from registers or as a literal constant.
	reg_t s0 = operands_[Operand::SRC0]->getRegValue(lane_id);
	reg_t s1 = operands_[Operand::SRC1]->getRegValue(lane_id);

	reg_t dst;
	// Calculate the product.
	dst.i64 = s0.i32 * s1.i32;

	// Write the results.
    operands_[Operand::DST]->setValue(dst , lane_id);
}

void INST::V_MULLO_I32_I32(WarpState *w, uint32_t lane_id)
{
	// Load operands from registers or as a literal constant.
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);

	// Truncate operands to 24-bit signed integers
	s0.as_uint = SignExtend(s0.as_uint, 24);
	s1.as_uint = SignExtend(s1.as_uint, 24);

	Register product;
	// Calculate the product.
	product.as_int = s0.as_int * s1.as_int;

	// Write the results.
    operands_[Operand::DST]->setValue(product, lane_id);
}

// D.f = min(S0.f, S1.f).
void INST::V_MIN_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register min;

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
    operands_[Operand::DST]->setValue(min, lane_id);
}

// D.f = max(S0.f, S1.f).
void INST::V_MAX_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register max;

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
    operands_[Operand::DST]->setValue(max, lane_id);
}

// D.i = max(S0.i, S1.i).
void INST::V_MAX_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register max;

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
    operands_[Operand::DST]->setValue(max, lane_id);
}

// D.i = min(S0.i, S1.i).
void INST::V_MIN_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register min;

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
    operands_[Operand::DST]->setValue(min, lane_id);
}

// D.u = min(S0.u, S1.u).
void INST::V_MIN_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register min;

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
    operands_[Operand::DST]->setValue(min, lane_id);
}

// D.u = max(S0.u, S1.u).
void INST::V_MAX_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register max;

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
    operands_[Operand::DST]->setValue(max, lane_id);
}

// D.u = S1.u >> S0.u[4:0].
void INST::V_LSHRREV_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Right shift s1 by s0.
	result.as_uint = s1.as_uint >> s0.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.i = S1.i >> S0.i[4:0].
void INST::V_ASHRREV_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Right shift s1 by s0.
	result.as_int = s1.as_int >> s0.as_int;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.u = S0.u << S1.u[4:0].
void INST::V_LSHL_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Left shift s1 by s0.
	result.as_uint = s0.as_uint << s1.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.u = S1.u << S0.u[4:0].
void INST::V_LSHLREV_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Left shift s1 by s0.
	result.as_uint = s1.as_uint << s0.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);

}

// D.u = S0.u & S1.u.
void INST::V_AND_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint & s1.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

// D.u = S0.u | S1.u.
void INST::V_OR_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint | s1.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);

}

// D.u = S0.u ^ S1.u.
void INST::V_XOR_B32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Bitwise OR the two operands.
	result.as_uint = s0.as_uint ^ s1.as_uint;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);

}

//D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset.
void INST::V_BFM_B32(WarpState *w, uint32_t lane_id)
{
	ISAUnimplemented(w);
}

// D.f = S0.f * S1.f + D.f.
void INST::V_MAC_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dst = operands_[Operand::SRC2]->getValue(lane_id);
	Register result;

	// Calculate the result.
	result.as_float = s0.as_float * s1.as_float + dst.as_float;

	// Write the results.
    operands_[Operand::DST]->setValue(result, lane_id);
}

#if 0
// D.f = S0.f * K + S1.f; K is a 32-bit inline constant
void INST::V_MADMK_F32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
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
#endif

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADD_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register sum;
	Register carry;


	// Calculate the sum and carry.
	// sum.as_int = s0.as_int + s1.as_int;
	//carry.as_uint =
	//	! !(((long long) s0.as_int + (long long) s1.as_int) >> 32);
    sum.as_uint = hw_op_->add(s0.as_int, s1.as_int, carry.as_uint, 0);
    // static int run_times {0};
    // run_times++;

    // hw_op_->adder32_sim->toText(internal::stringf("adder%d.log", run_times));
    // hw_op_->adder32_sim->toVCD(internal::stringf("adder%d.vcd", run_times));

	// Write the results.
    operands_[Operand::DST]->setValue(sum, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);
}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADD_I64(WarpState *w, uint32_t lane_id)
{
    reg_t s0, s1, sum, carry;

    std::vector<Register> src0 = operands_[Operand::SRC0]->getValueX(lane_id);
    std::vector<Register> src1 = operands_[Operand::SRC1]->getValueX(lane_id);
    std::vector<Register> result(2);

	// Register carry;
    s0.reg[0] = src0[0].as_uint;
    s0.reg[1] = src0[1].as_uint;
    s1.reg[0] = src1[0].as_uint;
    s1.reg[1] = src1[1].as_uint;

	// Calculate the sum and carry.
	sum.i64 = s0.i64 + s1.i64;
    result[0].as_uint = sum.reg[0];
    result[1].as_uint = sum.reg[1];
    // FIXME
	carry.u32 =
		! !(((uint64_t) s0.i32 + (uint64_t) s1.i32) >> 32);

	// Write the results.
    operands_[Operand::DST]->setValueX(result, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.u32, lane_id);
}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADD_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register sum;
	Register carry;

	// Calculate the sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint =
		! !(((uint64_t) s0.as_uint + (uint64_t) s1.as_uint) >> 32);

	// Write the results.
    operands_[Operand::DST]->setValue(sum, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);
}

// D.u = S0.u + S1.u, vcc = carry-out.
void INST::V_ADDCO_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register sum;
	Register carry;

	// Calculate the sum and carry.
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint =
		! !(((uint64_t) s0.as_uint + (uint64_t) s1.as_uint) >> 32);

	// Write the results.
    operands_[Operand::DST]->setValue(sum, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);
}

// D.u = S0.u - S1.u; vcc = carry-out.
void INST::V_SUB_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;
	Register carry;

	// Calculate the difference and carry.
	dif.as_uint = s0.as_int - s1.as_int;
	carry.as_uint = (s1.as_int > s0.as_int);

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);

}

// D.u = S0.u - S1.u; vcc = carry-out.
void INST::V_SUB_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;
	Register carry;

	// Calculate the difference and carry.
	dif.as_uint = s0.as_uint - s1.as_uint;
	carry.as_uint = (s1.as_uint > s0.as_uint);

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);
}

// D.u = S1.u - S0.u; vcc = carry-out.
void INST::V_SUBREV_I32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;
	Register carry;

	// Calculate the difference and carry.
	dif.as_int = s1.as_int - s0.as_int;
	carry.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);

}

void INST::V_SUBREV_U32(WarpState *w, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register dif;
	Register carry;

	// Calculate the difference and carry.
	dif.as_uint = s1.as_uint - s0.as_uint;
	carry.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setValue(dif, lane_id);
	w->setBitmaskSreg(RegisterVcc, carry.as_uint, lane_id);

}

// D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero.
/*
void INST::V_CVT_PKRTZ_F16_F32(WarpState *w, uint32_t lane_id)
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
// D = S0.u >> S1.u[4:0] (Arithmetic shift)
void INST::V_ASHR_I64(WarpState *item, uint32_t lane_id)
{
	reg_t s0, value;

    std::vector<Register> src0 = operands_[Operand::SRC0]->getValueX(lane_id);
    Register s1 = operands_[Operand::SRC1]->getValue(lane_id);

    std::vector<Register> result(2);
/* TODO
	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);
*/
	// Load operands from registers.
	s0.reg[0] = src0[0].as_uint;
	s0.reg[1] = src0[1].as_uint;
	s1.as_uint = s1.as_uint & 0x1F;

	// Shift s0.
	value.i64 = s0.i64 >> s1.as_uint;

	// Write the results.
	result[0].as_uint = value.reg[0];
	result[1].as_uint = value.reg[1];

    operands_[Operand::DST]->setValueX(result, lane_id);
}

// D.d = S0.d + S1.d.
void INST::V_ADD_F64(WarpState *item, uint32_t lane_id)
{
	reg_t s0, s1, value;

    std::vector<Register> src0 = operands_[Operand::SRC0]->getValueX(lane_id);
    std::vector<Register> src1 = operands_[Operand::SRC1]->getValueX(lane_id);

/* FIXME
	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
	assert(!OPCODE.neg);
	assert(!OPCODE.abs);
*/
	s0.reg[0] = src0[0].as_uint;
	s0.reg[1] = src0[1].as_uint;
	s1.reg[0] = src1[0].as_uint;
	s1.reg[1] = src1[1].as_uint;

	// Add the operands, take into account special number cases.

	// s0 == NaN64 || s1 == NaN64
	if (std::fpclassify(s0.f64) == FP_NAN ||
		std::fpclassify(s1.f64) == FP_NAN)
	{
		// value <-- NaN64
		value.f64 = NAN;
	}
	// s0,s1 == infinity
	else if (std::fpclassify(s0.f64) == FP_INFINITE &&
		std::fpclassify(s1.f64) == FP_INFINITE)
	{
		// value <-- NaN64
		value.f64 = NAN;
	}
	// s0,!s1 == infinity
	else if (std::fpclassify(s0.f64) == FP_INFINITE)
	{
		// value <-- s0(+-infinity)
		value.f64 = s0.f64;
	}
	// s1,!s0 == infinity
	else if (std::fpclassify(s1.f64) == FP_INFINITE)
	{
		// value <-- s1(+-infinity)
		value.f64 = s1.f64;
	}
	// s0 == +-denormal, +-0
	else if (std::fpclassify(s0.f64) == FP_SUBNORMAL ||
		std::fpclassify(s0.f64) == FP_ZERO)
	{
		// s1 == +-denormal, +-0
		if (std::fpclassify(s1.f64) == FP_SUBNORMAL ||
			std::fpclassify(s1.f64) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (std::signbit(s0.f64)
				&& std::signbit(s1.f64))
				// value <-- -0
				value.f64 = -0;
			else
				// value <-- +0
				value.f64 = +0;
		// s1 == F
		else
			// value <-- s1
			value.f64 = s1.f64;
	}
	// s1 == +-denormal, +-0
	else if (std::fpclassify(s1.f64) == FP_SUBNORMAL ||
		std::fpclassify(s1.f64) == FP_ZERO)
	{
		// s0 == +-denormal, +-0
		if (std::fpclassify(s0.f64) == FP_SUBNORMAL ||
			std::fpclassify(s0.f64) == FP_ZERO)
			// s0 && s1 == -denormal, -0
			if (std::signbit(s0.f64)
				&& std::signbit(s1.f64))
				// value <-- -0
				value.f64 = -0;
			else
				// value <-- +0
				value.f64 = +0;
		// s0 == F
		else
			// value <-- s1
			value.f64 = s0.f64;
	}
	// s0 && s1 == F
	else
	{
		value.f64 = s0.f64 + s1.f64;
	}

    std::vector<Register> result(2);
	// Write the results.
	result[0].as_uint = value.reg[0];
	result[1].as_uint = value.reg[1];

    operands_[Operand::DST]->setValueX(result, lane_id);
}

void INST::V_LSHL_B64(WarpState *item, uint32_t lane_id)
{
	// Input operands
	reg_t s0, s1, dst;

    std::vector<Register> src0 = operands_[Operand::SRC0]->getValueX(lane_id);
    Register src1 = operands_[Operand::SRC1]->getValue(lane_id);

    std::vector<Register> result(2);
/*
	assert(!OPCODE.clamp);
	assert(!OPCODE.omod);
*/
	// Load operands from registers.
	s0.reg[0] = src0[0].as_uint;
	s0.reg[1] = src0[1].as_uint;

	// LSHFT_B64
	// Mask s1 to return s1[4:0]
	// to extract left shift right operand
	dst.u64 = s0.u64 << (src1.as_uint & 0x001F);

	// Write the results.
	// Cast uint32 to unsigned int
	result[0].as_uint = dst.reg[0];
	result[1].as_uint = dst.reg[1];
    operands_[Operand::DST]->setValueX(result, lane_id);
	//WriteVReg(OPCODE.vdst, result_lo.as_uint, lane_id);
	// WriteVReg(OPCODE.vdst + 1, result_hi.as_uint, lane_id);
}


