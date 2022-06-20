#pragma once
// #include "inc/Compute.h"
#include "inc/ExecTypes.h"
#include "inc/ExecContext.h"
//#include "inc/ThreadBlock.h"
#include "inc/WarpState.h"
#include "inc/OperandUtil.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <cctype>
#include <algorithm>
#include <core.h>
#include <htl/decoupled.h>
#include "common.h"

class FunUnit;

#define DEFINSTEND(_fmt)
#define DEFINST2(_name)

// define V_AND_F32 = opcode enum
#define DEFFMT(_fmt, _fmt_str, _enc)                            \
	enum Opcode##_fmt {
#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)        \
        _name = _opcode ,
#define DEFEND(_fmt)                                            \
		_fmt##OpcodeCount                                       \
	};
#include <opcodes.def>
#undef DEFFMT
#undef DEFINST
#undef DEFEND


enum RDMode {
    RD_MODE_NE = 0,
    RD_MODE_PINF = 1,
    RD_MODE_NINF = 2,
    RD_MODE_ZERO = 3,
    RD_MODE_RTA = 4,
    RD_MODE_INVALID = 0xff,
    RD_NOT_SUPPORT = 0x100,
};


class Reg {
public:
    enum RegType {
        Scalar = 0,
        Vector = 1,
        Data = 2,
        TCC = 3
    };
    Reg(int32_t reg_idx, int32_t range = 1, RegType rt = Vector, int32_t lane_stride = 0, int32_t lie = false )
        : reg_idx_(reg_idx)
        , range_(range)
        , reg_type_(rt)
        , lane_stride_(lane_stride)
        , lie_(lie)
    {}
    bool operator==(const Reg& rhs) {
        rhs.reg_idx_ == reg_idx_ && rhs.range_ == range_;
    }
    int32_t reg_idx_;
    int32_t range_;
    int32_t lane_stride_;
    bool    lie_;
    RegType reg_type_;
};


class Operand {
public:
    enum OperandName {
        SRC0 = 0,
        SRC1 = 1,
        SRC2 = 2,
        SRC3 = 3,
        DST = 4,
        IMPLICIT_DST0 = 5,
        IMPLICIT_DST1 = 6,
        OPERAND_NAME_MAX
    };

    static OperandName GetSrcOperand(uint32_t idx) {
        switch(idx) {
            case 0: return SRC0; break;
            case 1: return SRC1; break;
            case 2: return SRC2; break;
            case 3: return SRC3; break;
            default: assert(false || "unknow src operand idx");
                     break;
        }
    }

    static OperandName GetDstOperand(uint32_t idx) {
        switch(idx) {
            case 0: return DST; break;
            case 1: return IMPLICIT_DST0; break;
            case 2: return IMPLICIT_DST1; break;
            default: assert(false || "unknow dst operand idx");
                     break;
        }
    }

    static std::string GetOperandNameStr(OperandName name) {
        switch(name) {
            case SRC0: return "SRC0"; break;
            case SRC1: return "SRC1"; break;
            case SRC2: return "SRC2"; break;
            case SRC3: return "SRC3"; break;
            case DST: return "DST"; break;
            case IMPLICIT_DST0: return "IMPLICIT_DST0"; break;
            case IMPLICIT_DST1: return "IMPLICIT_DST1"; break;
            default: assert(false || "unknow operand idx");
                     break;
        }
    }

    explicit Operand(enum OperandName name, Reg reg)
        : name_(name)
        , reg_(reg) {
            is_valid_ = true;
            range_ = reg.range_;
            if (reg_.reg_type_ == Reg::Scalar || reg_.reg_type_ == Reg::TCC) {
                value_.resize(range_);
            } else if (reg_.reg_type_ == Reg::Vector ||
                        reg_.reg_type_ == Reg::Data
                    ) {
                vector_value_.resize(range_);
                is_vector_ready_.resize(MAX_WARPSIZE);
                for (uint32_t i = 0; i < range_; i++) {
                    vector_value_[i].resize(MAX_WARPSIZE);
                }
            } else {
                assert(false || "new Operand ");
            }
        }
    explicit Operand(enum OperandName name, uint32_t imm) {
        name_ = name;
        is_imm_ = true;
        imm_.as_uint = imm;
        is_valid_ = true;
    }
    void setRegType(Reg::RegType rt) {
        reg_.reg_type_ = rt;
    }
    OperandName name_;
    bool is_valid_ {false};
    bool is_imm_ {false};
    bool is_scalar_ready_ {false};  // ready to execute warp operand
    std::vector<bool> is_vector_ready_;  // ready to execute thread operand
    Reg reg_ {0};
    Register imm_;
    std::vector<Register> value_;
    std::vector<std::vector<Register>> vector_value_;
    uint32_t range_;

    bool isDst() {
        if (name_ > DST) { return true; } else return false;
    }
    bool isValid() {return is_valid_;}
    bool isImm() {return is_imm_;}
    int32_t getImm() {return imm_.as_int;}

    isasim::reg_t getRegValue(int32_t lane_id = -1) {
        if (is_imm_) {
            return isasim::reg_t(imm_.as_uint);
        }
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC
                ) {
            return isasim::reg_t(value_[0].as_uint);
        } else if (reg_.reg_type_ == Reg::Vector ||
                    reg_.reg_type_ == Reg::Data
                ) {
            assert(lane_id != -1);
            return isasim::reg_t(vector_value_[0][lane_id].as_uint);
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    int32_t getRegIdx(uint32_t reg_num) {
        if (is_imm_) {
            return -1;
        }
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC
                ) {
            if (reg_num > reg_.range_) return -1;
            return (reg_.reg_idx_ + reg_num);
        } else if (reg_.reg_type_ == Reg::Vector ||
                    reg_.reg_type_ == Reg::Data
                ) {
            if (reg_num > reg_.range_) return -1;
            return (reg_.reg_idx_ + reg_num);
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    Register getValue(int32_t lane_id = -1) {
        if (is_imm_) {
            return imm_;
        }
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC
                ) {
            return value_[0];
        } else if (reg_.reg_type_ == Reg::Vector ||
                    reg_.reg_type_ == Reg::Data
                ) {
            assert(lane_id != -1);
            return vector_value_[0][lane_id];
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    std::vector<Register> getValueX(int32_t lane_id = -1) {
        std::vector<Register> value;
        if (is_imm_) {
            value.push_back(imm_);
            return value;
        }
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC ) {
            return value_;
        } else if (reg_.reg_type_ == Reg::Vector ||
                  reg_.reg_type_ == Reg::Data) {
            assert(lane_id != -1);
            for (uint32_t i=0; i < reg_.range_; i++) {
                value.push_back(vector_value_[i][lane_id]);
            }
            return value;
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    void setValue(isasim::reg_t value, int32_t lane_id = -1) {
        assert(is_imm_ == false);
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC ) {
            is_scalar_ready_ = true;
            for (uint32_t i=0; i < reg_.range_; i++) {
                value_[0].as_uint = value.reg[i];
            }
        } else if (reg_.reg_type_ == Reg::Vector ||
            reg_.reg_type_ == Reg::Data) {
            assert(lane_id != -1);
            for (uint32_t i=0; i < reg_.range_; i++) {
                is_vector_ready_[lane_id] = true;
                vector_value_[i][lane_id].as_uint = value.reg[i];
            }
        } else {
            assert(false || "TODO Operand getValue");
        }
    }
    void setValue(Register value, int32_t lane_id = -1) {
        assert(is_imm_ == false);
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC ) {
            is_scalar_ready_ = true;
            value_[0].as_uint = value.as_uint;
        } else if (reg_.reg_type_ == Reg::Vector ||
            reg_.reg_type_ == Reg::Data) {
            assert(lane_id != -1);
            is_vector_ready_[lane_id] = true;
            vector_value_[0][lane_id].as_uint = value.as_uint;
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    void setBitmask(Register value, int32_t lane_id = -1) {
        assert(is_imm_ == false);
        uint32_t mask = 1;
        Register bitfield;
        assert(reg_.reg_type_ == Reg::Scalar || reg_.reg_type_ == Reg::TCC);

        if (lane_id < 32) {
            mask <<= lane_id;
            bitfield = value_[0];
            value_[0].as_uint = (value.as_uint) ? bitfield.as_uint | mask : bitfield.as_uint & ~mask;
	    } else {
            assert(range_ > 1);
		    mask <<= (lane_id - 32);
		    bitfield = value_[1];
		    value_[1].as_uint = (value.as_uint) ? bitfield.as_uint | mask: bitfield.as_uint & ~mask;
        }
    }

    void setValueX(std::vector<Register> value, int32_t lane_id = -1) {
        assert(is_imm_ == false);
        assert(value.size() <= reg_.range_);
        if (reg_.reg_type_ == Reg::Scalar ||
            reg_.reg_type_ == Reg::TCC ) {
            is_scalar_ready_ = true;
            for (uint32_t i=0; i < reg_.range_; i++) {
                value_[i].as_uint = value[i].as_uint;
            }
        } else if (reg_.reg_type_ == Reg::Vector ||
            reg_.reg_type_ == Reg::Data) {
            assert(lane_id != -1);
            for (uint32_t i=0; i < reg_.range_; i++) {
                is_vector_ready_[lane_id] = true;
                vector_value_[i][lane_id].as_uint = value[i].as_uint;
            }
        } else {
            assert(false || "TODO Operand getValue");
        }
    }

    std::string getOperandStr() {
        std::stringstream ss;
        uint32_t idx = reg_.reg_idx_;
        if (isImm()) {
            ss << getImm();
            return ss.str();
        } else if (reg_.reg_type_ == Reg::Scalar) {
            ss << "s";
        } else if (reg_.reg_type_ == Reg::TCC) {
            ss << "tcc";
            idx = idx - RegisterTcc;
        } else if (reg_.reg_type_ == Reg::Vector) {
            ss << "v";
        } else if (reg_.reg_type_ == Reg::Data) {
            if (reg_.lie_) {
                ss << "l";
            } else {
                ss << "d";
            }
        } else {
            assert(false || "TODO getOperandStr");
        }
        if (reg_.range_ == 1) {
            ss << idx;
        } else {
            ss << "[" << idx <<  ":" << idx + reg_.range_ - 1 << "]";
        }
        return ss.str();
    }

    void dumpReg(std::stringstream& ss, WarpState *w) {
        std::string space = "";
        if (isImm()) {
            ss << "Imm = " << std::hex << "0x" << getImm() << "\n";
        } else if (reg_.reg_type_ == Reg::Scalar ||
                reg_.reg_type_ == Reg::TCC) {
            for (uint32_t i=0; i < reg_.range_; i++) {
                if (i > 0) ss << ", ";
                ss << "s" << std::dec << reg_.reg_idx_ + i << " = ";
                w->dumpSreg(ss, reg_.reg_idx_);
            }
            ss << "\n";
        } else if (reg_.reg_type_ == Reg::Vector) {
            Register value;
            for (uint32_t i=0; i < reg_.range_; i++) {
                ss << space << "v" << std::dec << reg_.reg_idx_ + i << " = ";
                w->dumpVreg(ss, reg_.reg_idx_ + i);
                space = "     ";
            }
        } else if (reg_.reg_type_ == Reg::Data) {
            Register value;
            uint32_t warp_addr_stride = reg_.lane_stride_ * w->getWarpSize();
            for (uint32_t i=0; i < reg_.range_; i++) {
                ss << space << ((reg_.lie_)? "l" : "d" )<< reg_.reg_idx_ + i << "-" << reg_.lane_stride_ << " = ";
                w->dumpDmem(ss, reg_.reg_idx_ + i * warp_addr_stride, reg_.lane_stride_);
                space = "     ";
            }
        }

    }

    void readReg(WarpState *w) {
        if (is_imm_) return;
        range_ = reg_.range_;
        for (uint32_t i=0; i < range_; i++) {
            if (reg_.reg_type_ == Reg::Scalar || reg_.reg_type_ == Reg::TCC) {
                is_scalar_ready_ = true;
                Register value;
                value.as_uint = w->getSreg(reg_.reg_idx_ + i);
                value_[i] = value;
            } else if (reg_.reg_type_ == Reg::Vector) {
                std::vector<Register> vector_value (w->getWarpSize());
                for (unsigned j = 0; j < w->getWarpSize(); j++) {
                    if (w->getActiveMask().test(j)) {
                        is_vector_ready_[j] = true;
                        vector_value[j].as_uint = w->getVreg(reg_.reg_idx_ + i, j);
                    }
                }
                vector_value_[i] = vector_value;
            } else if (reg_.reg_type_ == Reg::Data) {
                std::vector<Register> vector_value (w->getWarpSize());
                uint32_t warp_stride = reg_.lane_stride_ * w->getWarpSize();
                for (unsigned j = 0; j < w->getWarpSize(); j++) {
                    if (w->getActiveMask().test(j)) {
                        is_vector_ready_[j] = true;
                        uint32_t value;
                        w->getDmem(reg_.reg_idx_ + i * warp_stride + j * reg_.lane_stride_, 4 , (char*)&value);
                        vector_value[j].as_uint = value;
                    }
                }
                vector_value_[i] = vector_value;
            } else {
                assert("TODO");
            }
        }
    }
    void writeReg(WarpState *w) {
        range_ = reg_.range_;
        for (uint32_t i=0; i < range_; i++) {
            if (reg_.reg_type_ == Reg::Scalar ||
                reg_.reg_type_ == Reg::TCC) {
                is_scalar_ready_ = true;
                w->setSreg(reg_.reg_idx_ + i, value_[i].as_uint);
            } else if (reg_.reg_type_ == Reg::Vector) {
                for (unsigned j = 0; j < w->getWarpSize(); j++) {
                    if (w->getActiveMask().test(j)) {
                        assert(is_vector_ready_[j]);
                        w->setVreg(reg_.reg_idx_ + i, vector_value_[i][j].as_uint, j);
                    }
                }
            } else if (reg_.reg_type_ == Reg::Data) {
                std::vector<Register> vector_value (w->getWarpSize());
                uint32_t warp_stride = reg_.lane_stride_ * w->getWarpSize();
                for (unsigned j = 0; j < w->getWarpSize(); j++) {
                    if (w->getActiveMask().test(j)) {
                        assert(is_vector_ready_[j]);
                        uint32_t value;
                        value = vector_value_[i][j].as_uint;
                        w->setDmem(reg_.reg_idx_ + i * warp_stride + j * reg_.lane_stride_, 4 , (char*)&value);
                    }
                }
            } else {
                assert("TODO");
            }
        }
    }
};

class Instruction {
  public:
	enum Format
	{
		OpcodeInvalid = 0,
#define DEFFMT(_fmt, _fmt_str, _enc) Format##_fmt,
#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)
#define DEFEND(_fmt)
#include <opcodes.def>
#undef DEFFMT
#undef DEFINST
#undef DEFEND
		// Max
		FormatCount
	};


	/// Special register enumeration
	enum SpecialReg
	{
		SpecialRegInvalid = 0,
		SpecialRegVcc,
		SpecialRegScc,
		SpecialRegExec,
		SpecialRegTma,
		SpecialRegM0
	};

	/// Buffer data format
	enum BufDataFormat
	{
		BufDataFormatInvalid = 0,
		BufDataFormat8 = 1,
		BufDataFormat16 = 2,
		BufDataFormat8_8 = 3,
		BufDataFormat32 = 4,
		BufDataFormat16_16 = 5,
		BufDataFormat10_11_11 = 6,
		BufDataFormat11_10_10 = 7,
		BufDataFormat10_10_10_2 = 8,
		BufDataFormat2_10_10_10 = 9,
		BufDataFormat8_8_8_8 = 10,
		BufDataFormat32_32 = 11,
		BufDataFormat16_16_16_16 = 12,
		BufDataFormat32_32_32 = 13,
		BufDataFormat32_32_32_32 = 14,
		BufDataFormatReserved = 15
	};

	/// Buffer number format
	enum BufNumFormat
	{
		BufNumFormatUnorm = 0,
		BufNumFormatSnorm = 1,
		BufNumFormatUnscaled = 2,
		BufNumFormatSscaled = 3,
		BufNumFormatUint = 4,
		BufNumFormatSint = 5,
		BufNumFormatSnormNz = 6,
		BufNumFormatFloat = 7,
		BufNumFormatReserved = 8,
		BufNumFormatSrgb = 9,
		BufNumFormatUbnorm = 10,
		BufNumFormatUbnormNz = 11,
		BufNumFormatUbint = 12,
		BufNumFormatUbscaled = 13
	};
		/// Instruction flags
	enum Flag
	{
		FlagNone = 0x0000,
		FlagOp8 = 0x0001,  // Opcode represents 8 comparison instructions
		FlagOp16 = 0x0002  // Opcode represents 16 comparison instructions
	};

#include <opcodes_fmt.def>

	/// Instruction bytes
	union Bytes
	{
		uint8_t  byte[8];
		uint32_t word[2];
		uint64_t dword;

#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)
#define DEFFMT(_fmt, _fmt_str, _enc) Bytes##_fmt _fmt;
#define DEFEND(_fmt)
#include <opcodes.def>
#undef DEFINST
#undef DEFFMT
#undef DEFEND
	};

 public:
#if 0
#define DEFFMT(_fmt, _fmt_str, _enc)
#define DEFEND(_fmt)
#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)      \
  virtual void _name (WarpState*);
//      printf("Instruction is not unimplemented\n");
//  }
#include <opcodes.def>
#undef DEFFMT
#undef DEFINST
#undef DEFEND
#endif
	/// Entry in the instruction information table
	struct Info
	{
		/// Word formats
		Format fmt;

		/// Operation code
		// Opcode opcode;

		/// Instruction name
		const char *name;

		/// Format string
		const char *fmt_str;

		/// Opcode bits
		int op;

		/// Bitmap of flags
		Flag flags;

		/// Size of the micro-code format in bytes, not counting a
		/// possible additional literal added by a particular instance.
		int size;
	};

    public:
	// Instruction identifier with all information
	Info info;

	// Instruction bytes
	Bytes bytes;

	// Instruction size in bytes, including the literal constant
	// if present.
	uint32_t m_size;

	// Instruction virtual address, stored when decoding
	int address;

    inline Operand::OperandName GetOperandName(int32_t i) {
        switch(i) {
            case 0: return Operand::SRC0; break;
            case 1: return Operand::SRC1; break;
            case 2: return Operand::SRC2; break;
            case 3: return Operand::SRC3; break;
            case 4: return Operand::DST; break;
            case 5: return Operand::IMPLICIT_DST0; break;
            case 6: return Operand::IMPLICIT_DST1; break;
            default: assert(false || "unknow operand idx");
                     break;
        }
    }

    std::vector<Operand::OperandName> asm_print_order_;

	int getOp() { return info.op; }
	// Opcode getOpcode() { return info ? info->opcode : OpcodeInvalid; }
	Format getFormat() { return info.fmt; }
	const char *getName() { return info.name; }
	Bytes *getBytes() { return &bytes; }
	uint32_t GetSize() { return m_size; }
	op_type_t GetOpType() { return getOpType(info.op); }

    // std::map<uint32_t, std::string> getOpStr();
    // std::map<uint32_t, std::string> getOpFunc();

    virtual std::string getOpStr(int op) = 0;
    virtual opu_op_t getOpType(int op) = 0;

    virtual void Decode(uint64_t _opcode) = 0;
    virtual void print() {
        printf("decode: %s(%lx), size=%d, op_type=%d\n", getOpStr(info.op).c_str(), info.op, m_size, getOpType(info.op));
    }
    virtual void Issue(WarpState *w) {
        if (w->isDumpEnable()) {
            std::stringstream ss;
            ss << "[PC=" << pc << "]: " << getOpStr(info.op).c_str();
            if (not asm_print_order_.empty()) {
                for (auto &n : asm_print_order_) {
                    ss << "  " << operands_[n]->getOperandStr();
                }
            } else {
                for (uint32_t i= 0; i < num_dst_operands; i++) {
                    Operand::OperandName name = Operand::GetDstOperand(i);
                    ss << "  " << operands_[name]->getOperandStr();
                }
                for (uint32_t i= 0; i < num_src_operands; i++) {
                    Operand::OperandName name = Operand::GetSrcOperand(i);
                    ss << ", ";
                    ss << operands_[name]->getOperandStr();
                }
            }
            ss << "\n  STATE: tmask =" << w->getActiveMask();
            w->out() << ss.str(); //  << "\n";
        }
        /*
        for (uint32_t i= 0; i < num_src_operands; i++) {
            Operand::OperandName name = Operand::GetSrcOperand(i);
            // update Data Register 's stride infomation
            if (operands_[name]->reg_.reg_type_ == Reg::Data) {
	            int stride = w->getDmemStride(RegisterM0);
                operands_[name]->reg_.lane_stride_ = stride;
            }
        }
        */
    }
    virtual void OperandCollect(WarpState *w) {
        if (w->isDumpEnable()) {
            std::stringstream ss;
            std::string ident;
            ss << "\n  OC:";
            for (uint32_t i= 0; i < num_src_operands; i++) {
                ss << ident;
                ident = "     ";
                Operand::OperandName name = Operand::GetSrcOperand(i);
                operands_[name]->dumpReg(ss, w);
            }
            for (uint32_t i= 0; i < num_dst_operands; i++) {
                ss << ident;
                ident = "     ";
                Operand::OperandName name = Operand::GetDstOperand(i);
                operands_[name]->dumpReg(ss, w);
            }
            ss << "\n";
            w->out() << ss.str(); //  << "\n";
        }
        for (uint32_t i= 0; i < num_src_operands; i++) {
            Operand::OperandName name = Operand::GetSrcOperand(i);
            operands_[name]->readReg(w);
        }
        // target TCC need to read to for later bitmask operation
        for (uint32_t i= 0; i < num_dst_operands; i++) {
            Operand::OperandName name = Operand::GetDstOperand(i);
            if (operands_[name]->reg_.reg_type_ == Reg::TCC) {
                operands_[name]->readReg(w);
            }
        }
    };
    virtual void WriteBack(WarpState *w) {
        for (uint32_t i= 0; i < num_dst_operands; i++) {
            Operand::OperandName name = Operand::GetDstOperand(i);
            operands_[name]->writeReg(w);
        }
        if (w->isDumpEnable()) {
            std::stringstream ss;
            ss << "  WB:";
            std::string ident;
            for (uint32_t i= 0; i < num_dst_operands; i++) {
                ss << ident;
                ident = "     ";
                Operand::OperandName name = Operand::GetDstOperand(i);
                operands_[name]->dumpReg(ss, w);
            }
            w->out() << ss.str();
            if (num_dst_operands == 0) w->out() << "\n";
        }
    };
    virtual void Execute(WarpState *item, uint32_t lane_id = 0) = 0;
    virtual ~Instruction() = default;

 public:
  Instruction() {
    m_decoded = false;
    pc = (addr_t)-1;
    reconvergence_pc = (addr_t)-1;
    bar_type = NOT_BAR;
    red_type = NOT_RED;
    bar_id = (unsigned)-1;
    bar_count = (unsigned)-1;
    // op = NO_OP;
    // oprnd_type = UN_OP;
    // sp_op = OTHER_OP;
    // op_pipe = UNKOWN_OP;
    // mem_op = NOT_TEX;
    num_src_operands = 0;
    num_dst_operands = 0;
    num_regs = 0;
    operands_.resize(Operand::OPERAND_NAME_MAX);
    // operand_addr.resize(OPERAND_NAME_MAX);
    // operand_addr = {0, 0, 0, 0, 0, 0};
    // memset(out, 0, sizeof(unsigned));
    // memset(in, 0, sizeof(unsigned));
    // is_vectorin = 0;
    // is_vectorout = 0;
    // space = memory_space_t();
    // cache_op = CACHE_UNDEFINED;
    latency = 1;
    initiation_interval = 1;
    /*
    for (unsigned i = 0; i < MAX_REG_OPERANDS; i++) {
      arch_reg.src[i] = -1;
      arch_reg.dst[i] = -1;
    }*/
    m_size = 0;

	Clear();
  }

  void Clear();
  bool valid() const { return m_decoded; }
  virtual void print_insn(FILE *fp) const {
    fprintf(fp, " [inst @ pc=0x%04lx] ", pc);
  }
  void print_insn() const {
    print_insn(stdout);
    fflush(stdout);
  }
#if 0
  bool is_load() const {
      /*
    return (op == LOAD_OP || op == TENSOR_CORE_LOAD_OP ||
            memory_op == memory_load);
            */
  }
  bool is_store() const {
      /*
    return (op == STORE_OP || op == TENSOR_CORE_STORE_OP ||
            memory_op == memory_store);
            */
  }
#endif

  unsigned get_num_src_operands() const { return num_src_operands; }
  unsigned get_num_dst_operands() const { return num_dst_operands; }
  unsigned get_num_regs() const { return num_regs; }
  void set_num_regs(unsigned num) { num_regs = num; }
  void set_num_src_operands(unsigned num) { num_src_operands = num; }
  void set_num_dst_operands(unsigned num) { num_dst_operands = num; }
  void set_bar_id(unsigned id) { bar_id = id; }
  void set_bar_count(unsigned count) { bar_count = count; }

  bool hasOperand(Operand::OperandName operand_name) const {
      switch (operand_name) {
          case Operand::SRC0: return num_src_operands >= 1;
          case Operand::SRC1: return num_src_operands >= 2;
          case Operand::SRC2: return num_src_operands >= 3;
          case Operand::SRC3: return num_src_operands >= 4;
          case Operand::DST: return num_dst_operands >= 1;
          default:
            return false;
      }
  }

  void makeOperand(Operand::OperandName name, Reg &&reg, std::string &&alias_name = "") {
    operands_[name] = std::make_shared<Operand>(name, reg);
    if (alias_name != "") {
        operands_alias_[alias_name] = operands_[name];
    }
  };

  void makeOperand(Operand::OperandName name, uint32_t imm, std::string &&alias_name = "") {
    operands_[name] = std::make_shared<Operand>(name, imm);
    if (alias_name != "") {
        operands_alias_[alias_name] = operands_[name];
    }
  };

  std::shared_ptr<Operand> getOperand(Operand::OperandName name) {
      return operands_[name];
  }

  std::shared_ptr<Operand> getOperand(std::string&& name) {
      return operands_alias_[name];
  }
  // uint32_t getOperandAddr(OperandName operand_name) {
  //    return operand_addr[operand_name];
  //};



  addr_t pc;  // program counter address of instruction
  op_type_t op_type_;       // opcode (uarch visible)
  FunUnit    *funit_;       // cash wrapper

  barrier_type_t bar_type;
  reduction_type_t red_type;
  unsigned bar_id;
  unsigned bar_count;

  RDMode rd_mode;

  // types_of_operands_t oprnd_type;  // code (uarch visible) identify if the
                                 // operation is an interger or a floating point
  // special_ops_t sp_op;  // code (uarch visible) identify if int_alu, fp_alu, int_mul ....
  // operation_pipeline op_pipe;  // code (uarch visible) identify the pipeline of
                               // the operation (SP, SFU or MEM)
  // mem_operation mem_op;        // code (uarch visible) identify memory type
  // _memory_op_t memory_op;      // memory_op used by ptxplus
  unsigned num_src_operands;
  unsigned num_dst_operands;
  unsigned num_regs;  // count vector operand as one register operand
  std::vector<std::shared_ptr<Operand>> operands_;
  std::map<std::string, std::shared_ptr<Operand>> operands_alias_;

  addr_t reconvergence_pc;  // -1 => not a branch, -2 => use function
                                  // return address
/*
  unsigned out[8];
  unsigned outcount;
  unsigned in[24];
  unsigned incount;
  unsigned char is_vectorin;
  unsigned char is_vectorout;
*/
  int pred;  // predicate register number
  // int ar1, ar2;
  // register number for bank conflict evaluation
  /*
  struct {
    int dst[MAX_REG_OPERANDS];
    int src[MAX_REG_OPERANDS];
  } arch_reg;
  */
  // int arch_reg[MAX_REG_OPERANDS]; // register number for bank conflict
  // evaluation
  unsigned latency;  // operation latency
  unsigned initiation_interval;

  unsigned data_size;  // what is the size of the word being operated on?
  // memory_space_t space;
  // cache_operator_type cache_op;

  // warp level action
  void do_atomic(bool forceDo = false);
  void do_atomic(const active_mask_t &access_mask, bool forceDo = false);
  bool isatomic() const { return m_isatomic; }

  bool is_warp_op() const { return m_is_warp_op; }

  bool m_isatomic = false;
  bool m_is_warp_op = false;
  bool m_exit;

  // typedef void (Instruction::*FuncPtr)(WarpState *ti);

#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)
#define DEFFMT(_fmt, _fmt_str, _enc) bool is_##_fmt = false;
#define DEFEND(_fmt)
#include <opcodes.def>
#undef DEFINST
#undef DEFFMT
#undef DEFEND


 protected:
  bool m_decoded;

};


#define DEFFMT(_fmt, _fmt_str, _enc)                             \
class Instruction##_fmt : public Instruction {                     \
  public:                                                           \
    typedef void (Instruction##_fmt::*FuncPtr)(WarpState *ti, uint32_t lane_id);  \
    static FuncPtr OpFunc(uint64_t op, FuncPtr ptr = nullptr) {              \
        static FuncPtr _opfunc_table[255];                          \
        if (ptr != nullptr) { _opfunc_table[op] = ptr; }        \
        return _opfunc_table[op];                               \
    }                                                               \
    static std::string& OpStr(uint64_t op, std::string name = "") {     \
        static std::map<uint32_t, std::string> _opstr;                      \
        if (name != "") {                                                   \
            transform(name.begin(), name.end(), name.begin(), ::tolower);   \
            _opstr.insert(std::make_pair(op, name));                    \
        }                                                                   \
        return _opstr[op];                                              \
    }                                                                       \
    virtual std::string getOpStr(int op) {                                  \
        return OpStr(op);                                                   \
    }                                                                       \
    static opu_op_t OpType(uint64_t op, opu_op_t optype = opu_op_t::NO_OP) {      \
        static std::map<uint32_t, opu_op_t> _optype;                     \
        if (optype != opu_op_t::NO_OP) { _optype.insert(std::make_pair(op, optype));} \
        return _optype[op];                                              \
    }                                                                       \
    virtual opu_op_t getOpType(int op) {                                  \
        return OpType(op);                                                   \
    }                                                                       \
    Instruction##_fmt(uint64_t opcode) {                            \
        print##_fmt(bytes._fmt);                                    \
        is_##_fmt = true;                                           \
    }                                                               \
    virtual void Decode(uint64_t opcode);                           \
    virtual void print();                                           \
    virtual void OperandCollect(WarpState *w);                      \
    virtual void WriteBack(WarpState *w);                           \
                                                                    \
    virtual void Execute(WarpState *w, uint32_t lane_id = 0) override {               \
        (this->*OpFunc(info.op))(w, lane_id);                              \
    }                                                               \
                                                                    \
    static std::shared_ptr<Instruction##_fmt> make_instruction(uint64_t opcode) { \
        static bool initialized {false};                                    \
        if (!initialized) {

#define DEFINST(_name, _fmt_str, _op, _optype, _flags)          \
            Instruction##_fmt_str::OpFunc(_op, &Instruction##_fmt_str::_name);    \
            Instruction##_fmt_str::OpStr(_op, #_name);                            \
            Instruction##_fmt_str::OpType(_op, _optype);                          \
            //printf("INFO: create_op for (%s)%x, op_type:%d\n", #_name, _op, _optype);
#define DEFINSTEND(_fmt_str)                                        \
            initialized = true;                                     \
        }                                                           \
        std::shared_ptr<Instruction##_fmt_str> inst = std::make_shared<Instruction##_fmt_str>(opcode); \
        return inst;                                                \
    }

// (this->*(InstFuncTable[info.op]))(item, lane_id);

#define DEFINST2(_name)                                             \
    virtual void _name (WarpState*, uint32_t lane_id = 0);

#define DEFEND(_fmt)                                                \
};

#include <opcodes.def>
#undef DEFFMT
#undef DEFINST
#undef DEFINSTEND
#undef DEFINST2
#undef DEFEND

//	        (inst->InstFuncTable)[_opcode] = &Instruction##_fmt_str::_name; 

std::shared_ptr<Instruction> make_instruction(uint64_t _opcode, FunUnit*);

#if 0
class WarpInst {
public:
  Instruction *inst;
  addr_t get_reconvergence_pc { // -1 => not a branch, -2 => use function return address
      return inst->reconvergence_pc;
  }
  op_type_t get_op() {
      return inst->op;
  };
  uint32_t get_isize() {
      return inst->isize;
  };
  addr_t get_pc() {
      return inst->pc;
  };
};
#endif

void dsrc0_Decode(Instruction* inst, uint32_t imm_, uint32_t dsrc0_, uint32_t src0,
        uint32_t size, uint32_t ext_imm, uint32_t src0_ext, uint32_t reg_range, Operand::OperandName);

