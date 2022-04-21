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

    if (OPCODE.op == OpcodeSOPP::S_BARRIER) {
        m_op_type = opu_op_t::BARRIER_OP;
    }
    if (OPCODE.op == OpcodeSOPP::S_WAITCNT) {
        m_op_type = opu_op_t::WAITCNT_OP;
    }

    if (OPCODE.op == OpcodeSOPP::S_EXIT) {
        m_is_warp_op = false;
    }
}

void INST::print() {
    printf("Instruction: %s(%x)\n", opcode_str[info.op].c_str(), info.op);
}

void INST::dumpExecBegin(WarpState *w) {
}

void INST::dumpExecEnd(WarpState *w) {
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
	short simm16;
	int se_simm16;

	// Load the short constant operand and sign extend into an integer.
	simm16 = OPCODE.simm16;
	se_simm16 = simm16;

	// Relative jump
	item->incWarpPC(se_simm16 * 4 + 4 - m_size);
}

// if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_SCC0(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterScc))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}


// if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_SCC1(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	Register scc;

	scc.as_uint = ReadSReg(RegisterScc);

	if (scc.as_uint)
	{
		assert(ReadSReg(RegisterScc) == 1);

		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);

	}
	else
	{
	}
}

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_VCCZ(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	if (ReadSReg(RegisterVccz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_VCCNZ(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterVccz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}

// if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_EXECZ(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	// Register exec;
	Register execz;

	// exec.as_uint = ReadSReg(RegisterExec);
	execz.as_uint = ReadSReg(RegisterExecz);

	if (execz.as_uint)
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);

	}
	else
	{
	}
}


// if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void INST::S_CBRANCH_EXECNZ(WarpState *item, uint32_t lane_id)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterExecz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = OPCODE.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		item->incWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
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

