#pragma once
// #include "inc/Compute.h"
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
    Reg(int32_t reg_idx, int32_t range = 1, RegType rt = Vector )
        : reg_idx_(reg_idx)
        , range_(range)
        , reg_type_(rt)
    {}
    bool operator==(const Reg& rhs) {
        rhs.reg_idx_ == reg_idx_ && rhs.range_ == range_;
    }
    int32_t reg_idx_;
    int32_t range_;
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
        IMPLICIT_DST = 5,
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
            case 1: return IMPLICIT_DST; break;
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
            case IMPLICIT_DST: return "IMPLICIT_DST"; break;
            default: assert(false || "unknow operand idx");
                     break;
        }
    }

    explicit Operand(enum OperandName name, Reg reg)
        : name_(name)
        , reg_(reg) {
            is_valid_ = true;
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
    std::vector<bool> is_vector_ready_ {false};  // ready to execute thread operand
    Reg reg_ {0, 0};
    Register imm_;
    std::vector<Register> value_;
    uint32_t range_;

    bool isDst() {
        if (name_ > DST) { return true; } else return false;
    }
    bool isValid() {return is_valid_;}
    bool isImm() {return is_imm_;}
    uint32_t getImm() {return imm_.as_uint;}

    void dumpReg(std::stringstream& ss, WarpState *w) {
        if (isImm()) {
            ss << getImm();
        } else if (reg_.reg_type_ == Reg::Scalar) {
            for (uint32_t i=0; i < reg_.range_; i++) {
                ss << "s" << reg_.reg_idx_ + i << "=";
                w->dumpSreg(ss, reg_.reg_idx_);
            }
        } else if (reg_.reg_type_ == Reg::Vector) {
            Register value;
            for (uint32_t i=0; i < reg_.range_; i++) {
                ss << "v" << reg_.reg_idx_ + i << "=";
                w->dumpVreg(ss, reg_.reg_idx_ + i);
            }
        }
    }

    void readReg(WarpState *w, uint32_t lane_id = 0) {
        if (is_imm_) return;
        range_ = reg_.range_;
        value_.resize(range_);
        for (uint32_t i=0; i < range_; i++) {
            if (reg_.reg_type_ == Reg::Scalar) {
                is_scalar_ready_ = true;
                value_[i].as_uint = w->getSreg(reg_.reg_idx_ + i);
            } else if (reg_.reg_type_ == Reg::Vector) {
                is_vector_ready_[lane_id] = true;
                value_[i].as_uint = w->getVreg(reg_.reg_idx_ + i, lane_id);
            } else {
                assert("TODO");
            }
        }
    }
    void writeReg(WarpState *w, uint32_t lane_id = 0) {
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
            case 5: return Operand::IMPLICIT_DST; break;
            default: assert(false || "unknow operand idx");
                     break;
        }
    }

	int getOp() { return info.op; }
	// Opcode getOpcode() { return info ? info->opcode : OpcodeInvalid; }
	Format getFormat() { return info.fmt; }
	const char *getName() { return info.name; }
	Bytes *getBytes() { return &bytes; }
	uint32_t GetSize() { return m_size; }
	op_type_t GetOpType() { return m_op_type; }

    virtual void Decode(uint64_t _opcode) = 0;
    virtual void print() = 0;
    virtual void dumpExecBegin(WarpState *w) {
        std::stringstream ss;
        for (uint32_t i= 0; i < num_src_operands; i++) {
            Operand::OperandName name = Operand::GetSrcOperand(i);
            operands_[name]->dumpReg(ss, w);
            ss << "\n";
        }
        w->out() << ss.str() << "\n";
    };
    virtual void dumpExecEnd(WarpState *w) {
        std::stringstream ss;
        for (uint32_t i= 0; i < num_src_operands; i++) {
            Operand::OperandName name = Operand::GetDstOperand(i);
            operands_[name]->dumpReg(ss, w);
            ss << "\n";
        }
        w->out() << ss.str() << "\n";
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

  // uint32_t getOperandAddr(OperandName operand_name) {
  //    return operand_addr[operand_name];
  //};



  addr_t pc;  // program counter address of instruction
  op_type_t m_op_type;       // opcode (uarch visible)

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
    FuncPtr InstFuncTable[255];                                     \
    std::map<uint32_t, std::string> opcode_str;                     \
                                                                    \
    void addOp(uint32_t opcode, std::string op_str) {               \
       transform(op_str.begin(),op_str.end(), op_str.begin(), ::tolower); \
       opcode_str.insert(std::make_pair(opcode, op_str));           \
    }                                                               \
                                                                    \
    Instruction##_fmt(uint64_t opcode) {                            \
        print##_fmt(bytes._fmt);                                    \
        is_##_fmt = true;                                           \
    }                                                               \
    virtual void Decode(uint64_t opcode);                           \
    virtual void print();                                           \
    virtual void dumpExecBegin(WarpState *w);                      \
    virtual void dumpExecEnd(WarpState *w);                        \
                                                                    \
    virtual void Execute(WarpState *item, uint32_t lane_id = 0) override {               \
        (this->*(InstFuncTable[info.op]))(item, lane_id);                    \
    }                                                               \
                                                                    \
    static std::shared_ptr<Instruction##_fmt> make_instruction(uint64_t opcode) { \
        static bool initialized {false};                                    \
        static std::shared_ptr<Instruction##_fmt> inst = std::make_shared<Instruction##_fmt>(opcode);        \
        if (!initialized) {

#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)            \
	        (inst->InstFuncTable)[_opcode] = &Instruction##_fmt_str::_name; \
            inst->addOp(_opcode, #_name);                           \
            // printf("INFO: make_instruction for (%s)%x\n", #_name, _opcode);
            //
#define DEFINSTEND(_fmt_str)                                        \
            initialized = true;                                     \
        }                                                           \
        return inst;                                                \
    }

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


std::shared_ptr<Instruction> make_instruction(uint64_t _opcode);


class WarpInst {
public:
  Instruction *inst;
#if 0
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
#endif
};


