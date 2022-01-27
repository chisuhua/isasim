#pragma once
#include "inc/Compute.h"
#include "inc/ExecContext.h"
#include "inc/ThreadItem.h"
#include "inc/OperandUtil.h"

#define DEFINSTEND(_fmt)
#define DEFINST2(_name)

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
  virtual void _name (ThreadItem*);
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

	int getOp() { return info.op; }
	// Opcode getOpcode() { return info ? info->opcode : OpcodeInvalid; }
	Format getFormat() { return info.fmt; }
	const char *getName() { return info.name; }
	Bytes *getBytes() { return &bytes; }
	uint32_t GetSize() { return m_size; }
	OpType GetOpType() { return m_op_type; }
	bool is_exit() {
        // TODO
        return false;
    }

    virtual void Decode(uint64_t _opcode) = 0;
    virtual void Execute(ThreadItem *item) = 0;
    virtual ~Instruction() = default;

 public:
  Instruction() {
    m_decoded = false;
    pc = (address_type)-1;
    reconvergence_pc = (address_type)-1;
    bar_type = NOT_BAR;
    red_type = NOT_RED;
    bar_id = (unsigned)-1;
    bar_count = (unsigned)-1;
    // op = NO_OP;
    // oprnd_type = UN_OP;
    // sp_op = OTHER_OP;
    // op_pipe = UNKOWN_OP;
    // mem_op = NOT_TEX;
    num_operands = 0;
    num_regs = 0;
    // memset(out, 0, sizeof(unsigned));
    // memset(in, 0, sizeof(unsigned));
    is_vectorin = 0;
    is_vectorout = 0;
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

  unsigned get_num_operands() const { return num_operands; }
  unsigned get_num_regs() const { return num_regs; }
  void set_num_regs(unsigned num) { num_regs = num; }
  void set_num_operands(unsigned num) { num_operands = num; }
  void set_bar_id(unsigned id) { bar_id = id; }
  void set_bar_count(unsigned count) { bar_count = count; }

  address_type pc;  // program counter address of instruction
  OpType m_op_type;       // opcode (uarch visible)

  barrier_type bar_type;
  reduction_type red_type;
  unsigned bar_id;
  unsigned bar_count;

  types_of_operands oprnd_type;  // code (uarch visible) identify if the
                                 // operation is an interger or a floating point
  special_ops
      sp_op;  // code (uarch visible) identify if int_alu, fp_alu, int_mul ....
  // operation_pipeline op_pipe;  // code (uarch visible) identify the pipeline of
                               // the operation (SP, SFU or MEM)
  // mem_operation mem_op;        // code (uarch visible) identify memory type
  // _memory_op_t memory_op;      // memory_op used by ptxplus
  unsigned num_operands;
  unsigned num_regs;  // count vector operand as one register operand

  address_type reconvergence_pc;  // -1 => not a branch, -2 => use function
                                  // return address

  unsigned out[8];
  unsigned outcount;
  unsigned in[24];
  unsigned incount;
  unsigned char is_vectorin;
  unsigned char is_vectorout;
  int pred;  // predicate register number
  int ar1, ar2;
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

  // typedef void (Instruction::*FuncPtr)(ThreadItem *ti);

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
    typedef void (Instruction##_fmt::*FuncPtr)(ThreadItem *ti);  \
    FuncPtr InstFuncTable[255];                                     \
    std::map<uint32_t, std::string> opcode_str;                     \
                                                                    \
    Instruction##_fmt(uint64_t opcode) {                            \
        print##_fmt(bytes._fmt);                                    \
        is_##_fmt = true;                                           \
    }                                                               \
    virtual void Decode(uint64_t opcode);                           \
                                                                    \
    virtual void Execute(ThreadItem *item) override {               \
        printf("INFO: Execute %s(%x)\n", opcode_str[info.op].c_str(), info.op);    \
        (this->*(InstFuncTable[info.op]))(item);                    \
    }                                                               \
                                                                    \
    static shared_ptr<Instruction##_fmt> make_instruction(uint64_t opcode) { \
        static bool initialized {false};                                    \
        static shared_ptr<Instruction##_fmt> inst = make_shared<Instruction##_fmt>(opcode);        \
        if (!initialized) {

#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)            \
	        (inst->InstFuncTable)[_opcode] = &Instruction##_fmt_str::_name; \
            inst->opcode_str.insert(std::make_pair(_opcode, #_name));  \
            // printf("INFO: make_instruction for (%s)%x\n", #_name, _opcode);
            //
#define DEFINSTEND(_fmt_str)                                        \
            initialized = true;                                     \
        }                                                           \
        return inst;                                                \
    }

#define DEFINST2(_name)                                             \
    virtual void _name (ThreadItem*);

#define DEFEND(_fmt)                                                \
};

#include <opcodes.def>
#undef DEFFMT
#undef DEFINST
#undef DEFINSTEND
#undef DEFINST2
#undef DEFEND


shared_ptr<Instruction> make_instruction(uint64_t _opcode, unsigned int address);
