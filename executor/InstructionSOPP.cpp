#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "common/utils.h"

#define opcode bytes.SOPP

void InstructionSOPP::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = opcode.op;
    bytes.word[1] = 0;
	m_size = 4;
    m_is_warp_op = true;

    if (opcode.op == OpcodeSOPP::S_BARRIER) {
        m_op_type = opu_op_t::BARRIER_OP;
    }
    if (opcode.op == OpcodeSOPP::S_WAITCNT) {
        m_op_type = opu_op_t::WAITCNT_OP;
    }

    if (opcode.op == OpcodeSOPP::S_EXIT) {
        m_is_warp_op = false;
    }
}


// End the program.
void InstructionSOPP::S_EXIT(ThreadItem *item)
{
	GetWarp->SetFinished(true);     // TODO s_exit make all active thread done
    item->m_thread_done = true;
	// GetBlock->incWavefrontsCompletedEmu();
}

// PC = PC + signext(SIMM16 * 4) + 4
void InstructionSOPP::S_BRANCH(ThreadItem *item)
{
	short simm16;
	int se_simm16;

	// Load the short constant operand and sign extend into an integer.
	simm16 = opcode.simm16;
	se_simm16 = simm16;

	// Relative jump
	GetWarp->IncWarpPC(se_simm16 * 4 + 4 - m_size);
}

// if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_SCC0(ThreadItem *item)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterScc))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}


// if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_SCC1(ThreadItem *item)
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
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);

	}
	else
	{
	}
}

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_VCCZ(ThreadItem *item)
{
	short simm16;
	int se_simm16;

	if (ReadSReg(RegisterVccz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}

// if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_VCCNZ(ThreadItem *item)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterVccz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}

// if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_EXECZ(ThreadItem *item)
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
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);

	}
	else
	{
	}
}


// if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop.
void InstructionSOPP::S_CBRANCH_EXECNZ(ThreadItem *item)
{
	short simm16;
	int se_simm16;

	if (!ReadSReg(RegisterExecz))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = opcode.simm16;
		se_simm16 = simm16;

		// Determine the program counter to branch to.
		GetWarp->IncWarpPC(
			se_simm16 * 4 + 4 - m_size);
	}
}

/* Suspend current wavefront at the barrier. If all wavefronts in work-group
 * reached the barrier, wake them up */
void InstructionSOPP::S_BARRIER(ThreadItem *item)
{
#if 0
	// Suspend current wavefront at the barrier
	GetWarp->SetBarrierInstruction(true);
	GetWarp->SetAtBarrier(true);
	GetBlock->IncWarpsAtBarrier();

/*	Emulator::isa_debug << misc::fmt("Group %d wavefront %d reached barrier "
		"(%d reached, %d left)\n",
		GetBlock->getId(), wavefront->getId(), 
		GetBlock->getWavefrontsAtBarrier(),
		GetBlock->getWavefrontsInWorkgroup() - 
		GetBlock->getWavefrontsAtBarrier());
        */


	// If all wavefronts in work-group reached the barrier, wake them up
	if (GetBlock->GetWarpsAtBarrier() == GetBlock->GetWarpsCount())
	{
		for (auto i = GetBlock->WarpsBegin(),
                e = GetBlock->WarpsEnd(); i != e; ++i)
			i->second->SetAtBarrier(false);

		GetBlock->SetWarpsAtBarrier(0);

		// Emulator::isa_debug << misc::fmt("Group %d completed barrier\n", GetBlock->getId());
	}
#endif
}

void InstructionSOPP::S_WAITCNT(ThreadItem *item)
{
	// Nothing to do in emulation
	GetWarp->SetMemoryWait(true);
}

void InstructionSOPP::S_PHI(ThreadItem *item)
{
	ISAUnimplemented(item);
}

