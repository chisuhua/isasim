#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"

#define OPCODE bytes.VLS
#define INST InstructionVLS

void INST::Decode(uint64_t _opcode) {
    bytes.dword = _opcode;
    info.op = OPCODE.op;
	if (OPCODE.ext.e0_.ext_enc == 0x7) {
		m_size = 8;
	} else {
        bytes.word[1] = 0;
	    m_size = 4;
    }
    is_VLS = true;
    num_dst_operands = 1;
    num_src_operands = 2;
    uint32_t addr_reg_range = 2;
    uint32_t data_reg_range = 1;
    if (info.op == OpcodeVLS::V_LOAD_DWORDX4) {
        data_reg_range = 4;
    } else if (info.op == OpcodeVLS::V_LOAD_DWORDX2) {
        data_reg_range = 2;
    }

    operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.vaddr, addr_reg_range, OPCODE.ssrc0_ == 1? Reg::Scalar : Reg::Vector));

    operands_[Operand::SRC1] = std::make_shared<Operand>(Operand::SRC0,
                (uint32_t)OPCODE.offset);

    if (info.op == OpcodeVLS::V_STORE_DWORD) {
        num_dst_operands = 0;
        num_src_operands = 3;
        operands_[Operand::SRC2] = std::make_shared<Operand>( Operand::SRC2,
                Reg(OPCODE.vdata, data_reg_range, Reg::Vector));
    } else {
        operands_[Operand::DST] = std::make_shared<Operand>( Operand::DST,
                Reg(OPCODE.vdata, data_reg_range, Reg::Vector));
    }
}

void INST::print() {
    Instruction::print();
    printVLS(OPCODE);
}

void INST::OperandCollect(WarpState *w) {
    Instruction::OperandCollect(w);
}

void INST::WriteBack(WarpState *w) {
    Instruction::WriteBack(w);
}


uint64_t calculateAddr(Instruction* inst, WarpState* item, uint32_t lane_id) {
    RegisterX2 value_base;
    std::vector<Register> value = inst->operands_[Operand::SRC0]->getValueX(lane_id);

    value_base.as_reg[0].as_uint = value[0].as_uint;
    value_base.as_reg[1].as_uint = value[1].as_uint;

    Register value_offset = inst->operands_[Operand::SRC1]->getValue(lane_id);

	// Calculate effective address
	uint64_t base = value_base.as_ptr.addr;
	uint64_t offset = value_offset.as_uint;
	uint64_t addr = base + offset;
    return addr;
}

#if 0
void INST::V_LOAD_WORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);

	int vaddr = OPCODE.vaddr; //  << 2;

	MemoryPointer memory_pointer = item->getVBaseAddr(vaddr, lane_id);

	// Calculate effective address
	uint64_t m_base = memory_pointer.addr;

	uint64_t m_offset = OPCODE.offset * 4;
	uint64_t m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	Register value;
	{
		// Read value from global memory
        value.as_uint = 0;
		ReadVMEM(m_addr, 2, (char *)&value);
		WriteVReg(OPCODE.vdata, value.as_uint, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = m_addr;
	item->m_global_memory_access_size = 4 * 2;
}
#endif


void INST::V_LOAD_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t addr = calculateAddr(this, item, lane_id);

    std::vector<Register> value(1);
	for (int i = 0; i < 1; i++)
	{
		// Read value from global memory
		ReadVMEM(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
        operands_[Operand::DST]->setValueX(value, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_LOAD_DWORDX2(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t addr = calculateAddr(this, item, lane_id);
    assert(!(addr & 0x3));

    std::vector<Register> value(2);
	for (int i = 0; i < 2; i++)
	{
		// Read value from global memory
		ReadVMEM(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
        operands_[Operand::DST]->setValueX(value, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_LOAD_DWORDX4(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t addr = calculateAddr(this, item, lane_id);
	assert(!(addr & 0x3));

    std::vector<Register> value(4);
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		ReadVMEM(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
        operands_[Operand::DST]->setValueX(value, lane_id);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = addr;
	item->m_global_memory_access_size = 4 * 2;
}

void INST::V_STORE_DWORD(WarpState *item, uint32_t lane_id)
{
	// Record access
	// item->SetScalarMemoryRead(true);

    uint64_t addr = calculateAddr(this, item, lane_id);

	assert(!(addr & 0x3));

    std::vector<Register> value(1);
	for (int i = 0; i < 1; i++)
	{
        // FIXME don DST useage in store
		// value[i].as_uint = ReadVReg(OPCODE.vdata + i, lane_id);
        value = operands_[Operand::SRC2]->getValueX(lane_id);
		// Store the data in the destination register
		WriteVMEM(addr + i * 4, 4, (char *)&value[i]);
	}

	// Record last memory access for the detailed simulator.
	item->m_global_memory_access_address = addr;
	item->m_global_memory_access_size = 4 * 2;
}
