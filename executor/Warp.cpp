#include "inc/ThreadItem.h"
#include "inc/Warp.h"
#include "inc/Instruction.h"


void Warp::set_warp_active_mask(const active_mask_t &active, bool isatomic) {
  m_warp_active_mask = active;
  /*
  if (m_isatomic) {
    for (unsigned i = 0; i < m_config->warp_size; i++) {
      if (!m_warp_active_mask.test(i)) {
        m_per_scalar_thread[i].callback.function = NULL;
        m_per_scalar_thread[i].callback.instruction = NULL;
        m_per_scalar_thread[i].callback.thread = NULL;
      }
    }
  }
  */
}

void Warp::update(simt_mask_t &thread_done, addr_vector_t &next_pc,
                  address_type recvg_pc, OpType next_inst_op,
                  uint32_t next_inst_size, address_type next_inst_pc) {
    assert(m_stack.size() > 0);

    assert(next_pc.size() == m_warp_size);

    simt_mask_t top_active_mask = m_stack.back().m_active_mask;
    address_type top_recvg_pc = m_stack.back().m_recvg_pc;
    address_type top_pc = m_stack.back().m_pc;    // the pc of the instruction just executed
    stack_entry_type top_type = m_stack.back().m_type;
    assert(top_pc == next_inst_pc);
    assert(top_active_mask.any());

    const address_type null_pc = -1;
    bool warp_diverged = false;
    address_type new_recvg_pc = null_pc;
    uint32_t num_divergent_paths = 0;

    std::map<address_type, simt_mask_t> divergent_paths;
    while (top_active_mask.any()) {
        // extract a group of threads with the same next PC among the active threads
        // in the warp
        address_type tmp_next_pc = null_pc;
        simt_mask_t tmp_active_mask;
        for (int i = m_warp_size - 1; i >= 0; i--) {
            if (top_active_mask.test(i)) {    // is this thread active?
                if (thread_done.test(i)) {
                    top_active_mask.reset(i);    // remove completed thread from active mask
                } else if (tmp_next_pc == null_pc) {
                    tmp_next_pc = next_pc[i];
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                } else if (tmp_next_pc == next_pc[i]) {
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                }
            }
        }

        if (tmp_next_pc == null_pc) {
            assert(!top_active_mask.any());    // all threads done
            continue;
        }

        divergent_paths[tmp_next_pc] = tmp_active_mask;
        num_divergent_paths++;
    }

    address_type not_taken_pc = next_inst_pc + next_inst_size;
    assert(num_divergent_paths <= 2);
    for (uint32_t i = 0; i < num_divergent_paths; i++) {
        address_type tmp_next_pc = null_pc;
        simt_mask_t tmp_active_mask;
        tmp_active_mask.reset();
        if (divergent_paths.find(not_taken_pc) != divergent_paths.end()) {
            assert(i == 0);
            tmp_next_pc = not_taken_pc;
            tmp_active_mask = divergent_paths[tmp_next_pc];
            divergent_paths.erase(tmp_next_pc);
        } else {
            std::map<address_type, simt_mask_t>::iterator it =
                    divergent_paths.begin();
            tmp_next_pc = it->first;
            tmp_active_mask = divergent_paths[tmp_next_pc];
            divergent_paths.erase(tmp_next_pc);
        }

        // HANDLE THE SPECIAL CASES FIRST
        if (next_inst_op == CALL_OPS) {
            // Since call is not a divergent instruction, all threads should have
            // executed a call instruction
            assert(num_divergent_paths == 1);

            simt_stack_entry new_stack_entry;
            new_stack_entry.m_pc = tmp_next_pc;
            new_stack_entry.m_active_mask = tmp_active_mask;
            new_stack_entry.m_branch_div_cycle = m_branch_div_cycle;
            new_stack_entry.m_type = STACK_ENTRY_TYPE_CALL;
            m_stack.push_back(new_stack_entry);
            return;
        } else if (next_inst_op == RET_OPS && top_type == STACK_ENTRY_TYPE_CALL) {
            // pop the CALL Entry
            assert(num_divergent_paths == 1);
            m_stack.pop_back();

            assert(m_stack.size() > 0);
            m_stack.back().m_pc = tmp_next_pc;    // set the PC of the stack top entry
                                                                                    // to return PC from    the call stack;
            // Check if the New top of the stack is reconverging
            if (tmp_next_pc == m_stack.back().m_recvg_pc &&
                    m_stack.back().m_type != STACK_ENTRY_TYPE_CALL) {
                assert(m_stack.back().m_type == STACK_ENTRY_TYPE_NORMAL);
                m_stack.pop_back();
            }
            return;
        }

        // discard the new entry if its PC matches with reconvergence PC
        // that automatically reconverges the entry
        // If the top stack entry is CALL, dont reconverge.
        if (tmp_next_pc == top_recvg_pc && (top_type != STACK_ENTRY_TYPE_CALL))
            continue;

        // this new entry is not converging
        // if this entry does not include thread from the warp, divergence occurs
        if ((num_divergent_paths > 1) && !warp_diverged) {
            warp_diverged = true;
            // modify the existing top entry into a reconvergence entry in the pdom
            // stack
            new_recvg_pc = recvg_pc;
            if (new_recvg_pc != top_recvg_pc) {
                m_stack.back().m_pc = new_recvg_pc;
                m_stack.back().m_branch_div_cycle = m_branch_div_cycle;
                m_stack.push_back(simt_stack_entry());
            }
        }

        // discard the new entry if its PC matches with reconvergence PC
        if (warp_diverged && tmp_next_pc == new_recvg_pc) continue;

        // update the current top of pdom stack
        m_stack.back().m_pc = tmp_next_pc;
        m_stack.back().m_active_mask = tmp_active_mask;
        if (warp_diverged) {
            m_stack.back().m_calldepth = 0;
            m_stack.back().m_recvg_pc = new_recvg_pc;
        } else {
            m_stack.back().m_recvg_pc = top_recvg_pc;
        }

        m_stack.push_back(simt_stack_entry());
    }
    assert(m_stack.size() > 0);
    m_stack.pop_back();

    if (warp_diverged) {
        // m_gpu->gpgpu_ctx->stats->ptx_file_line_stats_add_warp_divergence(top_pc, 1);
    }
}
/*
void simt_stack::print(FILE *fout) const {
    for (unsigned k = 0; k < m_stack.size(); k++) {
        simt_stack_entry stack_entry = m_stack[k];
        if (k == 0) {
            fprintf(fout, "w%02d %1u ", m_warp_id, k);
        } else {
            fprintf(fout, "        %1u ", k);
        }
        for (unsigned j = 0; j < m_warp_size; j++)
            fprintf(fout, "%c", (stack_entry.m_active_mask.test(j) ? '1' : '0'));
        fprintf(fout, " pc: 0x%03x", stack_entry.m_pc);
        if (stack_entry.m_recvg_pc == (unsigned)-1) {
            fprintf(fout, " rp: ---- tp: %s cd: %2u ",
                            (stack_entry.m_type == STACK_ENTRY_TYPE_CALL ? "C" : "N"),
                            stack_entry.m_calldepth);
        } else {
            fprintf(fout, " rp: %4u tp: %s cd: %2u ", stack_entry.m_recvg_pc,
                            (stack_entry.m_type == STACK_ENTRY_TYPE_CALL ? "C" : "N"),
                            stack_entry.m_calldepth);
        }
        if (stack_entry.m_branch_div_cycle != 0) {
            fprintf(fout, " bd@%6u ", (unsigned)stack_entry.m_branch_div_cycle);
        } else {
            fprintf(fout, " ");
        }
        m_gpu->gpgpu_ctx->func_sim->ptx_print_insn(stack_entry.m_pc, fout);
        fprintf(fout, "\n");
    }
}
*/
/*
void Warp::ActivateAllThreadItems()
{
	for (auto it = m_items.begin(); it != m_items.end(); it++)
	{
		(*it)->setStatus(ThreadItem::ThreadItemStatusActive);
	}
}
*/

void Warp::AddThreadItem(std::shared_ptr<ThreadItem> item)
{
	this->m_items.push_back(item);
}

void Warp::reset() {
	// Integer inline constants.
	for(int i = 128; i < 193; i++)
		sreg[i].as_int = i - 128;
	for(int i = 193; i < 209; i++)
		sreg[i].as_int = -(i - 192);

	// Inline floats.
	sreg[240].as_float = 0.5;
	sreg[241].as_float = -0.5;
	sreg[242].as_float = 1.0;
	sreg[243].as_float = -1.0;
	sreg[244].as_float = 2.0;
	sreg[245].as_float = -2.0;
	sreg[246].as_float = 4.0;
	sreg[247].as_float = -4.0;

    m_stack.clear();
};

uint32_t Warp::getSregUint(int sreg) const
{
	uint32_t value;

	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	if (sreg == RegisterVccz) {
		if (this->sreg[RegisterVcc].as_uint == 0 &&
			this->sreg[RegisterVcc+1].as_uint == 0)
			value = 1;
		else
			value = 0;
	} if (sreg == RegisterExecz) {
		if (this->sreg[RegisterExec].as_uint == 0 &&
			this->sreg[RegisterExec+1].as_uint == 0)
			value = 1;
		else
			value = 0;
	} else {
		value = this->sreg[sreg].as_uint;
	}

	return value;
}

void Warp::setSregUint(int sreg, unsigned int value)
{
	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
	assert(sreg < 256);

	this->sreg[sreg].as_uint = value;

	// Update VCCZ and EXECZ if necessary.
	if (sreg == RegisterVcc || sreg == RegisterVcc + 1) {
		this->sreg[RegisterVccz].as_uint =
			!this->sreg[RegisterVcc].as_uint &
			!this->sreg[RegisterVcc + 1].as_uint;
	}
	if (sreg == RegisterExec || sreg == RegisterExec + 1)
	{
		this->sreg[RegisterExecz].as_uint =
			!this->sreg[RegisterExec].as_uint &
			!this->sreg[RegisterExec + 1].as_uint;
	}

}

