#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VOPC
#define INST InstructionVOPC

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    is_VOPC = true;

	if (OPCODE.ext.e0_.ext_enc == COMMON_ENC_ext1_enc) {
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

    dsrc0_Decode(this, OPCODE.imm_, OPCODE.dsrc0_, OPCODE.src0, m_size, OPCODE.ext.e1_.imm, OPCODE.ext.e1_.src0, src0_reg_range, Operand::SRC1);

    if (m_size == 8) {
        makeOperand(Operand::SRC0,
                Reg(OPCODE.vsrc1 | (OPCODE.ext.e1_.vsrc1 << COMMON_ENC_max_vsrc1_32e_width),
                    src0_reg_range, Reg::Vector));
        makeOperand(Operand::DST,
                Reg((OPCODE.tcc + RegisterTcc) | (OPCODE.ext.e1_.tcc << COMMON_ENC_max_tcc_32e_width),
                    dst_reg_range, Reg::TCC));
    } else {
        makeOperand(Operand::SRC0, Reg(OPCODE.vsrc1, src0_reg_range, Reg::Vector));
        makeOperand(Operand::DST, Reg(OPCODE.tcc + RegisterTcc, dst_reg_range, Reg::TCC));
        /*
        operands_[Operand::SRC1] = std::make_shared<Operand>(Operand::SRC1,
                Reg(OPCODE.vsrc1, src1_reg_range, Reg::Vector));
        operands_[Operand::DST] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.tcc + RegisterTcc, dst_reg_range, Reg::TCC));
                */
    }
}

void INST::print() {
    Instruction::print();
    printVOPC(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}

// vcc = (S0.f < S1.f).
void INST::V_CMP_LT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_float < s1.as_float);

    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.f > S1.f).
void INST::V_CMP_GT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_float > s1.as_float);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.f >= S1.f).
void INST::V_CMP_GE_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_float >= s1.as_float);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = !(S0.f > S1.f).
void INST::V_CMP_NGT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = !(s0.as_float > s1.as_float);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = !(S0.f == S1.f).
void INST::V_CMP_NEQ_F32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = !(s0.as_float == s1.as_float);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

#if 0
// vcc = (S0.d < S1.d).
void INST::V_CMP_LT_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = (S0.d == S1.d).
void INST::V_CMP_EQ_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = (S0.d <= S1.d).
void INST::V_CMP_LE_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = (S0.d > S1.d).
void INST::V_CMP_GT_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = !(S0.d >= S1.d).
void INST::V_CMP_NGE_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = !(S0.d == S1.d).
void INST::V_CMP_NEQ_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = !(S0.d < S1.d). 
void INST::V_CMP_NLT_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}
#endif


// vcc = (S0.i < S1.i).
void INST::V_CMP_LT_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.i == S1.i).
void INST::V_CMP_EQ_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.i <= S1.i).
void INST::V_CMP_LE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.i > S1.i).
void INST::V_CMP_GT_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.i <> S1.i).
void INST::V_CMP_NE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int != s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// D.u = (S0.i >= S1.i).
void INST::V_CMP_GE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// D = IEEE numeric class function specified in S1.u, performed on S0.d.
/*
void INST::V_CMP_CLASS_F64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}
*/

// vcc = (S0.u < S1.u).
void INST::V_CMP_LT_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_uint < s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.u == S1.u).
void INST::V_CMP_EQ_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = (S0.u <= S1.u).
void INST::V_CMP_LE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// vcc = (S0.u > S1.u).
void INST::V_CMP_GT_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}

// D.u = (S0.f != S1.f).
void INST::V_CMP_NE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;

	// Compare the operands.
	result.as_uint = (s0.as_uint != s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}


void INST::V_CMP_GE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue(lane_id);
	Register s1 = operands_[Operand::SRC1]->getValue(lane_id);
	Register result;


	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
    operands_[Operand::DST]->setBitmask(result, lane_id);
}
