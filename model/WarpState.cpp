#include <assert.h>
#include "inc/WarpState.h"
#include "inc/Instruction.h"
#include "inc/BlockState.h"
#include "inc/ThreadItem.h"
#include "coasm_define.h"

extern int g_debug_exec;

void WarpState::init(uint32_t warp_id, BlockState* tb_state, ThreadItem** thd) {
    m_warp_id = warp_id;
    m_tb_state = tb_state;
    m_thread = thd;

    m_sreg = new uint32_t[m_sreg_num];
    m_vreg = new uint32_t[m_vreg_num*m_warp_size];
    m_status = new WarpStatus[m_warp_size];

	// Integer inline constants.
#if 0
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
#endif
}

void WarpState::initDump(std::string filename) {
    m_dump.open(filename, std::ios::in | std::ios::out | std::ios::trunc);
    if (!m_dump.is_open()) {
        assert(false || "fail to open file");
    }
    m_dump_enable = true;
}

uint32_t WarpState::getVreg(uint32_t vreg, uint32_t lane_id)
{
	assert(vreg >= 0);
	assert(vreg < m_vreg_num);
    assert(lane_id < m_warp_size);

	return m_vreg[vreg*m_warp_size + lane_id];
}

void WarpState::setVreg(uint32_t vreg, uint32_t value, uint32_t lane_id)
{
	assert(vreg >= 0);
	assert(vreg < m_vreg_num);
    assert(lane_id < m_warp_size);
	m_vreg[vreg*m_warp_size + lane_id] = value;
}

uint32_t WarpState::getDmemStride(uint32_t sreg)
{
    uint32_t value = getSreg(sreg);
    value = (value >> SREG_M0_STRIDE_BIT) & (0x1 << SREG_M0_STRIDE_WIDTH -1);
    return value;
}

uint32_t WarpState::getBitmaskSreg(uint32_t sreg, uint32_t lane_id)
{
	uint32_t mask = 1;
    assert(lane_id < m_warp_size);
	mask <<= lane_id;
    return (getSreg(sreg) & mask) >> lane_id;
}

void WarpState::setBitmaskSreg(uint32_t sreg, uint32_t value, uint32_t lane_id)
{
	uint32_t mask = 1;
	uint32_t bitfield;
	uint32_t new_field;
    assert(lane_id < m_warp_size);
	mask <<= lane_id;
	bitfield = getSreg(sreg);
	new_field = (value) ? bitfield | mask | mask: bitfield & ~mask;
	setSreg(sreg, new_field);
}

uint32_t WarpState::getReservedSreg(uint32_t sreg) {
	uint32_t value;
    assert(sreg < m_kernel_const_reg_num);
	value = m_sreg[sreg];
    return value;
}

// Sreg have a hole which is decoded as cons reg
uint32_t WarpState::getSreg(uint32_t sreg)
{
	uint32_t value;

    if (sreg < MAX_RESERVED_SREG_NUM) {
	    value = m_sreg[sreg];
        return value;
    } else if (sreg < (m_kernel_const_reg_num + MAX_RESERVED_SREG_NUM)) {
        value = getConst(sreg - MAX_RESERVED_SREG_NUM);
        return value;
    }
    sreg -= m_kernel_const_reg_num;

	assert(sreg >= 0);
	assert(sreg < 256);
#if 0
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
#endif
	value = m_sreg[sreg];
	return value;
}

// Sreg have a hole which is decoded as cons reg
void WarpState::setSreg(uint32_t sreg, uint32_t value)
{
    // skip const write
    if (sreg < MAX_RESERVED_SREG_NUM) {
        assert("sreg is < MAX_RESERVED_SREG_NUM");
    } else if(sreg > (m_kernel_const_reg_num + MAX_RESERVED_SREG_NUM)) {
        sreg -= m_kernel_const_reg_num;
    }

	assert(sreg >= 0);
	assert(sreg < 256);

	m_sreg[sreg] = value;
#if 0
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
#endif
}
/*
void WarpState::printSreg() {
    printf("sreg:");
    for (uint32_t i=0 ; i < 32; i++) {
        printf(" \%x", getSreg(i));
    }
    printf("\n");
}
*/

void WarpState::dumpSreg(std::stringstream &ss, uint32_t sreg) {
    ss << std::hex << "0x" << std::setw(8) << std::setfill('0') << getSreg(sreg);
}
/*
void WarpState::printVreg() {
    for (uint32_t i=0 ; i < 32; i++) {
        printVreg(i);
    }
}
*/

void WarpState::dumpVreg(std::stringstream &ss, uint32_t vreg, uint32_t data_size) {
    for (uint32_t w=0; w < m_warp_size; w++) {
        if (w == 16) {
            ss << "         ";
        }
        if (g_debug_exec > 3 || ((w + 1) %16  < 5)) {
            ss << "0x" << std::hex << std::setw(data_size) << std::setfill('0') << getVreg(vreg, w);
        }
        if ((w + 1) % 16 == 0) {
            ss << "\n";
        } else {
            ss << " ";
        }
    }
    // ss << "\n";
}

void WarpState::dumpDmem(std::stringstream &ss, uint32_t dreg, uint32_t lane_stride, uint32_t data_size) {
    if (!m_dump_enable) return;
    for (uint32_t w=0; w < m_warp_size; w++) {
        if (w == 16) {
            ss << "         ";
        }
        uint32_t value;
        // FIXME adjust dreg per lane_id
        if (g_debug_exec > 3 || ((w + 1) %16  < 5)) {
            getDmem(dreg + lane_stride * w, 4, (char*)&value);
            ss << "0x" << std::hex << std::setw(data_size) << std::setfill('0') << value;
        }
        if ((w + 1) % 16 == 0) {
            ss << "\n";
        } else {
            ss << " ";
        }
    }
    // ss << "\n";
}

void WarpState::dumpAddr(std::stringstream &ss, std::vector<uint64_t> &addr, uint32_t tmsk) {
    if (!m_dump_enable) return;
    ss << std::hex;
    for (uint32_t w=0; w < m_warp_size; w++) {
        if (w == 16) {
            ss << "         ";
        }
        ss << "0x" << std::setw(12) << std::setfill('0') << addr[w];
        if ((w + 1) % 16 == 0) {
            ss << "\n";
        } else {
            ss << " ";
        }
    }
}

void WarpState::setDmem(uint32_t addr, uint32_t length, char* value) {
    m_dsm_write(addr, length, (void*)value);
}

void WarpState::getDmem(uint32_t addr, uint32_t length, char* value) {
    m_dsm_read(addr, length, (void*)value);
}

uint32_t WarpState::getConst(uint32_t addr) {
    return m_const_buffer[addr];
}

uint64_t WarpState::setupAddrSpace(uint64_t addr, isasim::mem_space_t::SpaceType &space) {
#if 0
    switch (space) {
        case isasim::mem_space_t::param_local_space:
        case isasim::mem_space_t::param_kernel_space:
            addr += m_param_addr;
            break;
        case isasim::mem_space_t::local_space:
            addr += m_local_mem_stack_pointer;
            break;
        default:
            break;
    }
#endif
    space = isasim::mem_space_t::global_space;
    return addr;
};
#if 0
// FIXME block const will setup in shared memory, remove below ugly code
MemoryPointer WarpState::getVBaseAddr(uint32_t vreg, uint32_t lane_id) {
	MemoryPointer mem_ptr;
    // FIXME for temp
    // vreg0 is reserved as sreg base
    if (vreg == KERNEL_PARAM_BASE) {
        mem_ptr.addr = getConst(0);
    } else if (vreg == LOCAL_MEM_PTR) {
        getSregMemPtr(0, mem_ptr);
    } else if (vreg < m_kernel_const_reg_num) {
        mem_ptr.addr = getConst(vreg);
    } else {
        getVregMemPtr(vreg, mem_ptr, lane_id);
    }
    return mem_ptr;
}
#endif

MemoryPointer WarpState::getDBaseAddr(uint32_t vreg, uint32_t lane_id) {
}

MemoryPointer WarpState::getSBaseAddr(uint32_t sreg) {
	MemoryPointer mem_ptr;
    getSregMemPtr(sreg, mem_ptr);
    return mem_ptr;
}

uint64_t WarpState::calculateAddr(uint32_t vreg) {
}


void WarpState::writeDMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readDMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

void WarpState::writeSMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readSMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

void WarpState::writeVMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readVMEM(uint64_t addr, uint32_t length, void* value, isasim::mem_space_t::SpaceType space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

#if 0
void WarpState::executeInst(std::shared_ptr<Instruction> inst) {
    inst->Execute(this);
}

void WarpState::executeInst(uint64_t opcode) {
    auto inst = make_instruction(opcode);
    inst->Decode(opcode);
    inst->Execute(this);
}
#endif

void WarpState::getSregMemPtr(uint32_t sreg, MemoryPointer &mem_ptr) {
	((uint32_t *)&mem_ptr)[0] = getSreg(sreg);
	((uint32_t *)&mem_ptr)[1] = getSreg(sreg + 1);
}

void WarpState::getVregMemPtr(uint32_t vreg, MemoryPointer &mem_ptr, uint32_t lane_id) {
	((uint32_t *)&mem_ptr)[0] = getVreg(vreg, lane_id);
	((uint32_t *)&mem_ptr)[1] = getVreg(vreg + 1, lane_id);
}


// use sreg[0,1] as param_base, sreg[2,3] as stack_pointer
void WarpState::setConstBuffer(uint32_t *const_buffer) {
    m_const_buffer = const_buffer;
}

// use sreg[0,1] as param_base, sreg[2,3] as stack_pointer
void WarpState::setStackPointer(uint64_t stack_pointer) {
    m_local_mem_stack_pointer = stack_pointer;
    RegisterX2 value;
    value.as_long = stack_pointer;
    setSreg(0, value.as_reg[0].as_uint);
    setSreg(1, value.as_reg[1].as_uint);
}

// Initialize a buffer resource descriptor
void WarpState::ReadBufferResource(
	int sreg,
	BufferDescriptor &buf_desc)
{
    uint32_t value[4];
    MemoryPointer mem_ptr;
    getSregMemPtr(sreg, mem_ptr);
    m_mem_read(mem_ptr.addr, 4 * sizeof(uint32_t), (void*)&value[0], isasim::mem_space_t::undefined);
	// Buffer resource descriptor is stored in 4 succesive scalar registers
	((uint32_t *) &buf_desc)[0] = value[0];
	((uint32_t *) &buf_desc)[1] = value[1];
	((uint32_t *) &buf_desc)[2] = value[2];
	((uint32_t *) &buf_desc)[3] = value[3];
}

void WarpState::arriveBar(uint32_t slot, uint32_t warp_count) {
        m_tb_state->arriveBar(slot, m_warp_id, warp_count);
}

void WarpState::setFinished(uint32_t lane_id) {
    m_status[lane_id] = WarpStatus::EXIT;
    m_tb_state->removeWarp(m_warp_id);
}

uint64_t WarpState::getWarpPC(uint32_t lane_id) {
    return getThread(lane_id)->get_pc();
}

//  FIXME modify to use real warp PC
void WarpState::setWarpPC(uint64_t pc) {
    for (uint32_t lane = 0; lane < m_warp_size; lane++) {
        if (m_active_mask.test(lane)) {
            getThread(lane)->set_pc(pc);
        }
    }
}

void WarpState::incWarpPC(int increment, uint32_t lane_id) {
    uint32_t pc = getWarpPC(lane_id);
    for (uint32_t lane = 0; lane < m_warp_size; lane++) {
        if (m_active_mask.test(lane)) {
            getThread(lane)->set_npc(pc + increment);
        }
    }
}

uint64_t WarpState::getThreadPC(uint32_t lane_id) {
    return getThread(lane_id)->get_pc();
}

void WarpState::setThreadPC(uint64_t pc, uint32_t lane_id) {
    getThread(lane_id)->set_pc(pc);
}

void WarpState::incThreadPC(int increment, uint32_t lane_id) {
    uint32_t pc = getWarpPC(lane_id);
    getThread(lane_id)->set_npc(pc + increment);
}


