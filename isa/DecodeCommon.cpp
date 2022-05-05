#include "inc/Instruction.h"

void dsrc0_Decode(Instruction* inst, uint32_t imm_, uint32_t dsrc0_, uint32_t src0, uint32_t reg_range, Operand::OperandName operand_name ) {
    if (imm_ == 0x1) {
        inst->operands_[operand_name] = std::make_shared<Operand>(operand_name,
                uint32_t((dsrc0_ << COMMON_ENC_max_src0_32e_width) + src0));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_l) {
	    int stride = 4;
        inst->operands_[operand_name] = std::make_shared<Operand>(operand_name,
                Reg(src0, reg_range, Reg::Data, stride));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_d) {
	    int stride = 0;
        inst->operands_[operand_name] = std::make_shared<Operand>(operand_name,
                Reg(src0, reg_range, Reg::Data, stride));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_s) {
        inst->operands_[operand_name] = std::make_shared<Operand>(operand_name,
                Reg(src0, reg_range, Reg::Scalar));
    } else {
        inst->operands_[operand_name] = std::make_shared<Operand>(operand_name,
                Reg(src0, reg_range, Reg::Vector));
    }
}
