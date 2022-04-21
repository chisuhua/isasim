#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VOPC
#define INST InstructionVOPC

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
	if (bytes.VOPC.src0 == 0xFF) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }
}

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}

void INST::dumpExecBegin(WarpState *w) {
}

void INST::dumpExecEnd(WarpState *w) {
}

uint32_t readSrc0(Instruction::BytesVOPC op, WarpState *item, uint32_t lane_id, uint32_t offset = 0) {
	// Load operand from register or as a literal constant.
    Register value;
    if (op.ssrc0_ == 0)
        value.as_uint = item->getSreg(op.src0 + offset);
    else
	    value.as_uint = item->getVreg(op.src0 + offset, lane_id);
    return value.as_uint;
}

// vcc = (S0.f < S1.f).
void INST::V_CMP_LT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_float < s1.as_float);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.f > S1.f).
void INST::V_CMP_GT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.f >= S1.f).
void INST::V_CMP_GE_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_float >= s1.as_float);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = !(S0.f > S1.f).
void INST::V_CMP_NGT_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = !(s0.as_float > s1.as_float);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = !(S0.f == S1.f).
void INST::V_CMP_NEQ_F32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = !(s0.as_float == s1.as_float);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

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
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int < s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.i == S1.i).
void INST::V_CMP_EQ_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int == s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.i <= S1.i).
void INST::V_CMP_LE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int <= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.i > S1.i).
void INST::V_CMP_GT_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int > s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.i <> S1.i).
void INST::V_CMP_NE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int != s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// D.u = (S0.i >= S1.i).
void INST::V_CMP_GE_I32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_int >= s1.as_int);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

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
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_uint < s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.u == S1.u).
void INST::V_CMP_EQ_U32(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// vcc = (S0.u <= S1.u).
void INST::V_CMP_LE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_uint <= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// vcc = (S0.u > S1.u).
void INST::V_CMP_GT_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_uint > s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}

// D.u = (S0.f != S1.f).
void INST::V_CMP_NE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_uint != s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}


void INST::V_CMP_GE_U32(WarpState *item, uint32_t lane_id)
{
	Register s0;
	Register s1;
	Register result;

	// Load operands from registers or as a literal constant.
	s0.as_uint = readSrc0(OPCODE, item, lane_id);
	s1.as_uint = ReadVReg(OPCODE.vsrc1, lane_id);

	// Compare the operands.
	result.as_uint = (s0.as_uint >= s1.as_uint);

	// Write the results.
	WriteBitmaskSReg(RegisterVcc, result.as_uint, lane_id);

}


