#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "common/utils.h"

#define OPCODE bytes.SOPP
#define INST InstructionSOPP

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
    bytes.word[1] = 0;
	m_size = 4;
    m_is_warp_op = true;

    num_dst_operands = 0;
    num_src_operands = 1;
    uint32_t reg_range = 1;

    if (OPCODE.op == OpcodeSOPP::S_EXIT) {
        m_is_warp_op = false;
        num_src_operands = 0;
    }

    if (OPCODE.op <= OpcodeSOPP::S_BRANCH) {
        operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                uint32_t(OPCODE.simm12));

        if (OPCODE.op <= OpcodeSOPP::S_CBRANCH_EXECNZ) {
            operands_[Operand::SRC1] = std::make_shared<Operand>(Operand::SRC1,
                Reg(OPCODE.tcc + RegisterTcc, reg_range, Reg::TCC));
            num_src_operands = 2;
        }
    }
    if (OPCODE.op == OpcodeSOPP::BAR_SYNC) {
        makeOperand(Operand::SRC0, int32_t(OPCODE.tcc), "bar_id");
        makeOperand(Operand::SRC1, int32_t(OPCODE.simm12), "bar_count");
    }
}

void INST::print() {
    Instruction::print();
    printSOPP(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}


// End the program.
void INST::S_EXIT(WarpState *item, uint32_t lane_id)
{
	item->setFinished(lane_id);     // TODO s_exit make all active thread done
    // item->m_thread_done = true;
	// GetBlock->incWavefrontsCompletedEmu();
}

// PC = PC + signext(SIMM16 * 4) + 4
void INST::S_BRANCH(WarpState *item, uint32_t lane_id)
{
	int se_simm;

	Register simm = operands_[Operand::SRC0]->getValue();
	// Load the short constant operand and sign extend into an integer.
	se_simm = simm.as_int;

	// Relative jump
	item->incWarpPC(se_simm * 4 + 4 - m_size);
}

// if(TCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_TCCZ(WarpState *item, uint32_t lane_id)
{
	int se_simm;

	Register simm = operands_[Operand::SRC0]->getValue();
	se_simm = simm.as_int;

	Register cc = operands_[Operand::SRC1]->getValue();
	if (!cc.as_uint)
	{
	    se_simm = simm.as_int;

		// Determine the program counter to branch to.
		item->incWarpPC(se_simm * 4 + 4 - m_size);
	}
}


// if(TCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_TCCNZ(WarpState *item, uint32_t lane_id)
{
	int se_simm;

	Register simm = operands_[Operand::SRC0]->getValue();
	se_simm = simm.as_int;

	Register cc = operands_[Operand::SRC1]->getValue();
	if (cc.as_uint)
	{
	    se_simm = simm.as_int;

		// Determine the program counter to branch to.
		item->incWarpPC(se_simm * 4 + 4 - m_size);
	}
}

// if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_SCCZ(WarpState *item, uint32_t lane_id)
{
	int se_simm;

	Register simm = operands_[Operand::SRC0]->getValue();
	se_simm = simm.as_int;

	Register cc = operands_[Operand::SRC1]->getValue();
	if (!cc.as_uint)
	{
	    se_simm = simm.as_int;

		// Determine the program counter to branch to.
		item->incWarpPC(se_simm * 4 + 4 - m_size);
	}
}

// if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_SCCNZ(WarpState *item, uint32_t lane_id)
{
	int se_simm;

	Register simm = operands_[Operand::SRC0]->getValue();
	se_simm = simm.as_int;

	Register cc = operands_[Operand::SRC1]->getValue();
	if (cc.as_uint)
	{
	    se_simm = simm.as_int;

		// Determine the program counter to branch to.
		item->incWarpPC(se_simm * 4 + 4 - m_size);
	}
}

// if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_EXECZ(WarpState *item, uint32_t lane_id)
{
}


// if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_EXECNZ(WarpState *item, uint32_t lane_id)
{
}

/* Suspend current wavefront at the barrier. If all wavefronts in work-group
 * reached the barrier, wake them up */
void INST::S_BARRIER(WarpState *item, uint32_t lane_id)
{
#if 0
	// Suspend current wavefront at the barrier
	item->SetBarrierInstruction(true);
	item->SetAtBarrier(true);
	GetBlock->IncWarpsAtBarrier();

/*	Emulator::isa_debug << misc::fmt("Group %d wavefront %d reached barrier "
		"(%d reached, %d left)\n",
		GetBlock->getId(), wavefront->getId(), 
		GetBlock->getWavefrontsAtBarrier(),
		GetBlock->getWavefrontsInWorkgroup() - 
		GetBlock->getWavefrontsAtBarrier());
        */


	// If all wavefronts in work-group reached the barrier, wake them up
	if (GetBlock->itemsAtBarrier() == GetBlock->itemsCount())
	{
		for (auto i = GetBlock->WarpsBegin(),
                e = GetBlock->WarpsEnd(); i != e; ++i)
			i->second->SetAtBarrier(false);

		GetBlock->SetWarpsAtBarrier(0);

		// Emulator::isa_debug << misc::fmt("Group %d completed barrier\n", GetBlock->getId());
	}
#endif
}

void INST::S_WAITCNT(WarpState *item, uint32_t lane_id)
{
	// Nothing to do in emulation
	item->SetMemoryWait(true);
}

void INST::S_PHI(WarpState *item, uint32_t lane_id)
{
	ISAUnimplemented(item);
}

void INST::BAR_SYNC(WarpState *item, uint32_t lane_id)
{
    auto op0 = getOperand("bar_id");
    auto op1 = getOperand("bar_count");
}

