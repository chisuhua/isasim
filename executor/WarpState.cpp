#include <assert.h>
#include "inc/WarpState.h"
#include "inc/Instruction.h"
#include "coasm_define.h"


void WarpState::init() {
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

uint32_t WarpState::getVreg(uint32_t vreg, uint32_t lane_id)
{
	assert(vreg >= 0);
	assert(vreg < m_vreg_num);
    assert(lane_id < m_warp_size);

    if (vreg > (KERNEL_CONST_REG_BASE_KERNEL_VIEW - m_kernel_const_reg_num)) {
        return getConst(KERNEL_CONST_REG_BASE + (KERNEL_CONST_REG_BASE_KERNEL_VIEW - vreg));
    }
	return m_vreg[vreg*m_warp_size + lane_id];
}

void WarpState::setVreg(uint32_t vreg, uint32_t value, uint32_t lane_id)
{
	assert(vreg >= 0);
	assert(vreg < m_vreg_num);
    assert(lane_id < m_warp_size);
	m_vreg[vreg*m_warp_size + lane_id] = value;
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
	new_field = (value) ? bitfield | mask: bitfield & ~mask;
	setSreg(sreg, new_field);
}

uint32_t WarpState::getSreg(uint32_t sreg) const
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

void WarpState::setSreg(uint32_t sreg, uint32_t value)
{
	assert(sreg >= 0);
	assert(sreg != 104);
	assert(sreg != 105);
	assert(sreg != 125);
	assert((sreg < 209) || (sreg > 239));
	assert((sreg < 248) || (sreg > 250));
	assert(sreg != 254);
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

void WarpState::printSreg() {
    printf("sreg:");
    for (uint32_t i=0 ; i < 32; i++) {
        printf(" \%x", getSreg(i));
    }
    printf("\n");
}

void WarpState::printSreg(uint32_t sreg) {
    printf("sreg\%d:\%x\n", getSreg(sreg));
}

void WarpState::printVreg() {
    for (uint32_t i=0 ; i < 32; i++) {
        printVreg(i);
    }
}

void WarpState::printVreg(uint32_t vreg) {
    printf("vreg\%d:", vreg);
    for (uint32_t i=0 ; i < 32; i++) {
        printf(" \%x", getVreg(vreg, i));
    }
    printf("\n");
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

uint64_t WarpState::setupAddrSpace(uint64_t addr, mem_space_t &space) {
#if 0
    switch (space) {
        case mem_space_t::param_local_space:
        case mem_space_t::param_kernel_space:
            addr += m_param_addr;
            break;
        case mem_space_t::local_space:
            addr += m_local_mem_stack_pointer;
            break;
        default:
            break;
    }
#endif
    space = mem_space_t::global_space;
    return addr;
};

// FIXME block const will setup in shared memory, remove below ugly code
MemoryPointer WarpState::getVBaseAddr(uint32_t vreg, uint32_t lane_id) {
	MemoryPointer mem_ptr;
    // FIXME for temp
    // vreg0 is reserved as sreg base
    if (vreg == KERNEL_PARAM_BASE) {
        mem_ptr.addr = getConst(0);
    } else if (vreg == LOCAL_MEM_PTR) {
        getSregMemPtr(0, mem_ptr);
    } else if (vreg > (KERNEL_CONST_REG_BASE_KERNEL_VIEW - m_kernel_const_reg_num)) {
        mem_ptr.addr = getConst(KERNEL_CONST_REG_BASE + (KERNEL_CONST_REG_BASE_KERNEL_VIEW - vreg));
    } else {
        getVregMemPtr(vreg, mem_ptr, lane_id);
    }
    return mem_ptr;
}

MemoryPointer WarpState::getDBaseAddr(uint32_t vreg, uint32_t lane_id) {
}

MemoryPointer WarpState::getSBaseAddr(uint32_t sreg) {
	MemoryPointer mem_ptr;
    getSregMemPtr(sreg, mem_ptr);
    return mem_ptr;
}

uint64_t WarpState::calculateAddr(uint32_t vreg) {
}


void WarpState::writeDMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readDMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

void WarpState::writeSMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readSMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

void WarpState::writeVMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_write(addr, length, value, space);
}

void WarpState::readVMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space) {
    addr = setupAddrSpace(addr, space);
    m_mem_read(addr, length, value, space);
}

void WarpState::executeInst(std::shared_ptr<Instruction> inst) {
    inst->Execute(this);
}

void WarpState::executeInst(uint64_t opcode) {
    auto inst = make_instruction(opcode);
    inst->Decode(opcode);
    inst->Execute(this);
}

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
    RegisterX2 regx2;
    regx2.as_long = stack_pointer;
    setSreg(0, regx2.reg[0].as_uint);
    setSreg(1, regx2.reg[1].as_uint);
}

// Initialize a buffer resource descriptor
void WarpState::ReadBufferResource(
	int sreg,
	BufferDescriptor &buf_desc)
{
    uint32_t value[4];
    MemoryPointer mem_ptr;
    getSregMemPtr(sreg, mem_ptr);
    m_mem_read(mem_ptr.addr, 4 * sizeof(uint32_t), (void*)&value[0], mem_space_t::undefined);
	// Buffer resource descriptor is stored in 4 succesive scalar registers
	((uint32_t *) &buf_desc)[0] = value[0];
	((uint32_t *) &buf_desc)[1] = value[1];
	((uint32_t *) &buf_desc)[2] = value[2];
	((uint32_t *) &buf_desc)[3] = value[3];
}
