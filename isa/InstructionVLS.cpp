#include "inc/Instruction.h"
#include "inc/InstructionCommon.h"
#include "common/DataTypes.h"

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
    if (info.op == OpcodeVLS::V_LOAD_U32X4) {
        data_reg_range = 4;
    } else if (info.op == OpcodeVLS::V_LOAD_U64) {
        data_reg_range = 2;
    }

    operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(OPCODE.vaddr, addr_reg_range, OPCODE.ssrc0_ == 1? Reg::Scalar : Reg::Vector));

    operands_[Operand::SRC1] = std::make_shared<Operand>(Operand::SRC0,
                (uint32_t)OPCODE.offset);

    if (info.op == OpcodeVLS::V_STORE_U8 ||
        info.op == OpcodeVLS::V_STORE_U16 ||
        info.op == OpcodeVLS::V_STORE_U32) {
        num_dst_operands = 0;
        num_src_operands = 3;
        operands_[Operand::SRC2] = std::make_shared<Operand>( Operand::SRC2,
                Reg(OPCODE.vdata, data_reg_range, Reg::Vector));
        asm_print_order_ = {Operand::SRC2, Operand::SRC0, Operand::SRC1};
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
    // RegisterX2 value_base;
    std::vector<Register> value = inst->operands_[Operand::SRC0]->getValueX(lane_id);

    // value_base.as_reg[0].as_uint = value[0].as_uint;
    //value_base.as_reg[1].as_uint = value[1].as_uint;

    uint64_t base_ptr = U64(value);


    Register value_offset = inst->operands_[Operand::SRC1]->getValue(lane_id);

	// Calculate effective address
	// uint64_t base = value_base.as_ptr.addr;
	uint64_t offset = value_offset.as_uint;
	uint64_t addr = base_ptr + offset;
	// assert(!(addr & 0x3));
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
void load(Instruction* inst, WarpState* w, uint32_t size, uint32_t lane_id) {
    uint64_t addr = calculateAddr(inst, w, lane_id);
    reg_t value;
	w->readVMEM(addr, size, &(value.u8));
    inst->operands_[Operand::DST]->setValue(value, lane_id);
}

void store(Instruction* inst, WarpState* w, uint32_t size, uint32_t lane_id) {
    uint64_t addr = calculateAddr(inst, w, lane_id);
    reg_t value;
    value.u32 = inst->operands_[Operand::SRC2]->getValue(lane_id).as_uint;
	w->writeVMEM(addr, size, &(value.u8));
}

void INST::V_LOAD_U8(WarpState *w, uint32_t lane_id)
{
    load(this, w, 1, lane_id);
}

void INST::V_LOAD_U16(WarpState *w, uint32_t lane_id)
{
    load(this, w, 2, lane_id);
}

void INST::V_LOAD_U32(WarpState *w, uint32_t lane_id)
{
    load(this, w, 4, lane_id);
}

void INST::V_LOAD_U64(WarpState *item, uint32_t lane_id)
{
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

void INST::V_LOAD_U32X4(WarpState *w, uint32_t lane_id)
{
	// Record access
	// w->SetScalarMemoryRead(true);

	// assert(OPCODE.imm);
    uint64_t addr = calculateAddr(this, w, lane_id);
	assert(!(addr & 0x3));

    std::vector<Register> value(4);
	for (int i = 0; i < 4; i++)
	{
		// Read value from global memory
		w->readVMEM(addr + i * 4, 4, (char *)&value[i]);
		// Store the data in the destination register
        operands_[Operand::DST]->setValueX(value, lane_id);
	}

	// Record last memory access for the detailed simulator.
	w->m_global_memory_access_address = addr;
	w->m_global_memory_access_size = 4 * 2;
}

void INST::V_STORE_U8(WarpState *w, uint32_t lane_id)
{
    store(this, w, 1, lane_id);
}
void INST::V_STORE_U16(WarpState *w, uint32_t lane_id)
{
    store(this, w, 2, lane_id);
}
void INST::V_STORE_U32(WarpState *w, uint32_t lane_id)
{
    store(this, w, 4, lane_id);
}

void INST::V_STORE_U64(WarpState *w, uint32_t lane_id)
{
    uint64_t addr = calculateAddr(this, w, lane_id);
	assert(!(addr & 0x3));

    std::vector<Register> value(2);
	for (int i = 0; i < 2; i++)
	{
        value = operands_[Operand::SRC2]->getValueX(lane_id);
	    w->writeVMEM(addr + i * 4, 4, (char*)&value[i]);
	}
}

