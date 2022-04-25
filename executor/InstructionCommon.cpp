#include "inc/Instruction.h"

void dsrc0_Decode(Instruction* inst, uint32_t imm_, uint32_t dsrc0_, uint32_t src0, uint32_t reg_range ) {
    if (imm_ == 0x3) {
        inst->operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                uint32_t(dsrc0_ << COMMON_ENC_max_src0_32e_width + src0));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_l) {
	    int stride = 4;
        inst->operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(src0, reg_range, Reg::Data, stride));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_d) {
	    int stride = 0;
        inst->operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(src0, reg_range, Reg::Data, stride));
    } else if (dsrc0_ == COMMON_ENC_dsrc0_s) {
        inst->operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(src0, reg_range, Reg::Scalar));
    } else {
        inst->operands_[Operand::SRC0] = std::make_shared<Operand>(Operand::SRC0,
                Reg(src0, reg_range, Reg::Vector));
    }
}
