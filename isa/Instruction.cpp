
#include <cassert>
#include <iomanip>
#include <sstream>

#include "inc/Instruction.h"

std::shared_ptr<Instruction> make_instruction(uint64_t _opcode, FunUnit *funit)
{
    /* Initialize instruction */
    Instruction::Bytes opcode;
    opcode.dword = _opcode;

    std::shared_ptr<Instruction> inst;

    //    printf("INFO: make instruction for fmt enc: (%s)%x\n", _fmt_str, _enc);    \

    if (false) {
#define DEFINSTEND(_fmt)
#define DEFINST2(_name)
#define DEFINST(_name, _fmt_str, _opcode, _size, _flags)
#define DEFEND(_fmt)
#define DEFFMT(_fmt, _fmt_str, _enc)                                \
    } else if (opcode._fmt.enc == _enc) {                           \
        inst = Instruction##_fmt::make_instruction(_opcode);
#include <opcodes.def>
#undef DEFINST
#undef DEFFMT
#undef DEFEND
    } else {
        assert(false || "ERROR: unknow opcode format");
    }

    // inst->address = address;
    // inst->m_decoded = true;
    inst->funit_ = funit;
    return inst;
}


/// Constants for special registers
/*
const unsigned RegisterM0;
const unsigned RegisterVcc;
const unsigned RegisterVccz;
const unsigned RegisterExec;
const unsigned RegisterExecz;
const unsigned RegisterScc;
*/

#if 0
const misc::StringMap Instruction::format_map =
{
    { "<invalid>", FormatInvalid },
    { "sop2", FormatSOP2 },
    { "sopk", FormatSOPK },
    { "sop1", FormatSOP1 },
    { "sopc", FormatSOPC },
    { "sopp", FormatSOPP },
    { "smrd", FormatSMRD },
    { "vop2", FormatVOP2 },
    { "vop1", FormatVOP1 },
    { "vopc", FormatVOPC },
    { "vop3a", FormatVOP3a },
    { "vop3b", FormatVOP3b },
    { "vintrp", FormatVINTRP },
    { "ds", FormatDS },
    { "mubuf", FormatMUBUF },
    { "mtbuf", FormatMTBUF },
    { "mimg", FormatMIMG },
    { "exp", FormatEXP }
};

const misc::StringMap Instruction::sdst_map =
{
    {"reserved", 0},
    {"reserved", 1},
    {"vcc_lo", 2},
    {"vcc_hi", 3},
    {"tba_lo", 4},
    {"tba_hi", 5},
    {"tma_lo", 6},
    {"tma_hi", 7},
    {"ttmp0", 8},
    {"ttmp1", 9},
    {"ttmp2", 10},
    {"ttmp3", 11},
    {"ttmp4", 12},
    {"ttmp5", 13},
    {"ttmp6", 14},
    {"ttmp7", 15},
    {"ttmp8", 16},
    {"ttmp9", 17},
    {"ttmp10", 18},
    {"ttmp11", 19},
    {"m0", 20},
    {"reserved", 21},
    {"exec_lo", 22},
    {"exec_hi", 23}
};

const misc::StringMap Instruction::ssrc_map =
{
    {"0.5", 0},
    {"-0.5", 1},
    {"1.0", 2},
    {"-1.0", 3},
    {"2.0", 4},
    {"-2.0", 5},
    {"4.0", 6},
    {"-4.0", 7},
    {"reserved", 8},
    {"reserved", 9},
    {"reserved", 10},
    {"vccz", 11},
    {"execz", 12},
    {"scc", 13},
    {"reserved", 14},
    {"literal constant", 15}
};

const misc::StringMap Instruction::buf_data_format_map =
{
    {"invalid", BufDataFormatInvalid },
    {"BUF_DATA_FORMAT_8", BufDataFormat8 },
    {"BUF_DATA_FORMAT_16", BufDataFormat16 },
    {"BUF_DATA_FORMAT_8_8", BufDataFormat8_8 },
    {"BUF_DATA_FORMAT_32", BufDataFormat32 },
    {"BUF_DATA_FORMAT_16_16", BufDataFormat16_16 },
    {"BUF_DATA_FORMAT_10_11_11", BufDataFormat10_11_11 },
    {"BUF_DATA_FORMAT_11_10_10", BufDataFormat11_10_10 },
    {"BUF_DATA_FORMAT_10_10_10_2", BufDataFormat10_10_10_2 },
    {"BUF_DATA_FORMAT_2_10_10_10", BufDataFormat2_10_10_10 },
    {"BUF_DATA_FORMAT_8_8_8_8", BufDataFormat8_8_8_8 },
    {"BUF_DATA_FORMAT_32_32", BufDataFormat32_32 },
    {"BUF_DATA_FORMAT_16_16_16_16", BufDataFormat16_16_16_16 },
    {"BUF_DATA_FORMAT_32_32_32", BufDataFormat32_32_32 },
    {"BUF_DATA_FORMAT_32_32_32_32", BufDataFormat32_32_32_32 },
    {"reserved", BufDataFormatReserved }
};

const misc::StringMap Instruction::buf_num_format_map =
{
    {"BUF_NUM_FORMAT_UNORM", BufNumFormatUnorm },
    {"BUF_NUM_FORMAT_SNORM", BufNumFormatSnorm },
    {"BUF_NUM_FORMAT_UNSCALED", BufNumFormatUnscaled },
    {"BUF_NUM_FORMAT_SSCALED", BufNumFormatSscaled },
    {"BUF_NUM_FORMAT_UINT", BufNumFormatUint },
    {"BUF_NUM_FORMAT_SINT", BufNumFormatSint },
    {"BUF_NUM_FORMAT_SNORM_NZ", BufNumFormatSnormNz },
    {"BUF_NUM_FORMAT_FLOAT", BufNumFormatFloat },
    {"reserved", BufNumFormatReserved },
    {"BUF_NUM_FORMAT_SRGB", BufNumFormatSrgb },
    {"BUF_NUM_FORMAT_UBNORM", BufNumFormatUbnorm },
    {"BUF_NUM_FORMAT_UBNORM_NZ", BufNumFormatUbnormNz },
    {"BUF_NUM_FORMAT_UBINT", BufNumFormatUbint },
    {"BUF_NUM_FORMAT_UBSCALED", BufNumFormatUbscaled }
};

const misc::StringMap Instruction::op16_map =
{
    {"f", 0},
    {"lt", 1},
    {"eq", 2},
    {"le", 3},
    {"gt", 4},
    {"lg", 5},
    {"ge", 6},
    {"o", 7},
    {"u", 8},
    {"nge", 9},
    {"nlg", 10},
    {"ngt", 11},
    {"nle", 12},
    {"neq", 13},
    {"nlt", 14},
    {"tru", 15}
};

const misc::StringMap Instruction::op8_map =
{
    {"f", 0},
    {"lt", 1},
    {"eq", 2},
    {"le", 3},
    {"gt", 4},
    {"lg", 5},
    {"ge", 6},
    {"tru", 7}
};

const misc::StringMap Instruction::special_reg_map =
{
    { "vcc", SpecialRegVcc },
    { "scc", SpecialRegScc },
    { "exec", SpecialRegExec },
    { "tma", SpecialRegTma },
    { "m0", SpecialRegM0 }    
};


#endif


void Instruction::Clear()
{
    m_decoded = false;
    bytes.dword = 0;
    m_size = 0;
    // address = 0;
}

#if 0
static std::shared_ptr<Instruction> Instruction::make_instruction(uint64_t opcode, unsigned int address)
{
    /* Initialize instruction */
    Bytes bytes
    bytes.dword = opcode;

    if (false) {
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags)
#define DEFEND(_name)
#define DEFFMT(_name, _fmt_str, _enc)                                       \
    } else if (bytes._name.enc == _enc) {                                   \
        auto inst = Instruction##_name::make_instruction(opcode);
#include <opcodes.def>
#undef DEFINST
#undef DEFFMT
#undef DEFEND
    } else {
        printf("ERROR: unknow opcode format")
    }

    inst->address = address;

    return inst;
    m_decoded = true;
}
#endif
