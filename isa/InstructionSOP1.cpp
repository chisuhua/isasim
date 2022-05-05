#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "common/utils.h"


#define OPCODE bytes.SOP1
#define INST InstructionSOP1

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    m_is_warp_op = true;

	if (OPCODE.ext.e0_.ext_enc == 0x7) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
		m_size = 4;
    }

    num_dst_operands = 1;
    num_src_operands = 1;
    uint32_t reg_range = 1;

    if (info.op == OpcodeSOP1::S_MOV_B64 ||
        info.op == OpcodeSOP1::S_WQM_B64 ||
        info.op == OpcodeSOP1::S_SWAPPC_B64 ||
        info.op == OpcodeSOP1::S_AND_SAVEEXEC_B64
        ) {
        reg_range = 2;
    }

    operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.ssrc0, reg_range, Reg::Scalar));
    operands_[Operand::DST] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.sdst, reg_range, Reg::Scalar));
}

void INST::print() {
    Instruction::print();
    printSOP1(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}


// D.u = S0.u.
void INST::S_MOV_B64(WarpState *item, uint32_t lane_id)
{
	// Assert no literal constant with a 64 bit instruction.
	// assert(!(OPCODE.ssrc0 == 0xFF));

    std::vector<Register> s0 = operands_[Operand::SRC0]->getValueX();
	// Register s0_lo = s0[0];
	// Register s0_hi = s0[1];

    operands_[Operand::DST]->setValueX(s0);
}

// D.u = S0.u.
void INST::S_MOV_B32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue();

    operands_[Operand::DST]->setValue(s0);
}

// D.u = ~S0.u SCC = 1 if result non-zero.
void INST::S_NOT_B32(WarpState *item, uint32_t lane_id)
{
	Register s0 = operands_[Operand::SRC0]->getValue();
	// Register nonzero;
	Register result;

	// nonzero.as_uint = ! !s0.as_uint;
    result.as_uint = ~s0.as_uint;
    operands_[Operand::DST]->setValue(result);

	// Write the results.
	// Store the data in the destination register
	// WriteSReg(OPCODE.sdst, s0.as_uint);
	// Store the data in the destination register
	// WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", OPCODE.sdst, s0.as_uint);
}

// D.u = WholeQuadMode(S0.u). SCC = 1 if result is non-zero.
void INST::S_WQM_B64(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

// D.u = PC + 4, PC = S0.u
void INST::S_SWAPPC_B64(WarpState *item, uint32_t lane_id)
{
	Register s0_lo;
	Register s0_hi;

	// FIXME: cuurently PC is implemented as 32-bit offset
	// Load operands from registers
	s0_lo.as_uint = ReadSReg(OPCODE.ssrc0);
	s0_hi.as_uint = ReadSReg(OPCODE.ssrc0 + 1);

	// Write the results
	// Store the data in the destination register
	WriteSReg(OPCODE.sdst, item->getWarpPC() + 4);
	// Store the data in the destination register
	WriteSReg(OPCODE.sdst + 1, 0);

	// Set the new PC
	item->setWarpPC(s0_lo.as_uint - 4);

    debug_print("s%u <= (0x%x)", OPCODE.sdst+1, s0_hi.as_uint);
	// Print isa debug information.
    //	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", OPCODE.sdst, pc + 4);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", OPCODE.sdst + 1, s0_hi.as_uint);
    //	Emulator::isa_debug << misc::fmt("PC<=(0x%x)", GetWarp->GetWarpPC());
}

/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is
 * non-zero. */
void INST::S_AND_SAVEEXEC_B64(WarpState *item, uint32_t lane_id)
{
	// Assert no literal constant with a 64 bit instruction.
	assert(!(OPCODE.ssrc0 == 0xFF));

	Register exec_lo;
	Register exec_hi;
	Register s0_lo;
	Register s0_hi;
	Register exec_new_lo;
	Register exec_new_hi;
	Register nonzero;

	// Load operands from registers.
	exec_lo.as_uint = ReadSReg(RegisterExec);
	exec_hi.as_uint = ReadSReg(RegisterExec + 1);
	s0_lo.as_uint = ReadSReg(OPCODE.ssrc0);
	s0_hi.as_uint = ReadSReg(OPCODE.ssrc0 + 1);

	/* Bitwise AND exec and the first operand and determine if the result 
	 * is non-zero. */
	exec_new_lo.as_uint = s0_lo.as_uint & exec_lo.as_uint;
	exec_new_hi.as_uint = s0_hi.as_uint & exec_hi.as_uint;
	nonzero.as_uint = exec_new_lo.as_uint || exec_new_hi.as_uint;

	// Write the results.
	// Store the data in the destination register
	WriteSReg(OPCODE.sdst, exec_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(OPCODE.sdst + 1, exec_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterExec, exec_new_lo.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterExec + 1, exec_new_hi.as_uint);
	// Store the data in the destination register
	WriteSReg(RegisterScc, nonzero.as_uint);

	// Print isa debug information.
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", OPCODE.sdst, exec_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("S%u<=(0x%x) ", OPCODE.sdst + 1, exec_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("exec_lo<=(0x%x) ", exec_new_lo.as_uint);
	//	Emulator::isa_debug << misc::fmt("exec_hi<=(0x%x) ", exec_new_hi.as_uint);
	//	Emulator::isa_debug << misc::fmt("scc<=(%u)", nonzero.as_uint);
}

