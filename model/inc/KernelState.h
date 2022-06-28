#pragma once
#include <stdint.h>
#include <map>
#include "inc/ExecTypes.h"
#include "inc/ExecContext.h"

// the const buffer is implemented in icache
// it use same scalar encode, with higer 128 encoding space
// but in isasim, it create as buffer per kernel
class KernelState {
    enum class Status {
        READY,
        RUNNING,
        EXIT
    };
public:
	KernelState(uint32_t const_num)
        : m_const_num(const_num)
    {
        m_const_buffer = new uint32_t[const_num];
        init();
    }

    void init() {
        m_status = Status::READY;
    }

    uint32_t *getConstBuffer() {
        return m_const_buffer;
    }

    uint64_t getProgAddr() {
        return m_prog_addr;
    }

    void setProgAddr( uint64_t prog_addr) {
        m_prog_addr = prog_addr;
    }

    uint64_t getParamAddr() {
        return m_param_addr;
    }

    void setParamAddr(uint64_t param_addr) {
        m_param_addr = param_addr;
        // *(uint64_t*)m_const_buffer = param_addr;
    }

    void setConstReg(uint32_t addr, uint32_t value) {
        // the first 2 is param_addr and local_mem_addr
        *((uint32_t*)m_const_buffer + addr) = value;
    }

    void setConstReg(uint32_t addr, uint64_t value) {
        // the first 2 is param_addr and local_mem_addr
        *((uint64_t*)m_const_buffer + addr) = value;
    }

    uint64_t getParamSize() {
        return m_param_size;
    }

    void setParamSize(uint64_t param_size) {
        m_param_size = param_size;
    }

    uint64_t getLocalAddr() {
        return m_local_addr;
    }

    void setLocalAddr(uint64_t local_addr) {
        m_local_addr = local_addr;
        *(uint64_t*)(m_const_buffer + 1) = local_addr;
    }

    uint64_t getLocalSize() {
        return m_local_addr;
    }

    void setLocalSize(uint64_t local_size) {
        m_local_size = local_size;
    }

    uint64_t getSharedSize() {
        return m_shared_size;
    }

    void setSharedSize(uint64_t shared_size) {
        m_shared_size = shared_size;
    }

    uint32_t getVRegUsed() {
        return m_vreg_used;
    }

    void setVRegUsed(uint32_t reg) {
        m_vreg_used = reg;
    }

    uint32_t getSRegUsed() {
        return m_sreg_used;
    }

    void setSRegUsed(uint32_t reg) {
        m_sreg_used = reg;
    }

private:
    Status m_status;
    uint32_t m_const_num;
    uint64_t m_prog_addr;
    uint64_t m_param_addr;
    uint64_t m_param_size;
    uint64_t m_local_addr;
    uint64_t m_local_size;
    uint64_t m_shared_addr;
    uint64_t m_shared_size;
    uint32_t m_vreg_used;
    uint32_t m_sreg_used;
    uint32_t *m_const_buffer;
    std::map<dim3, uint32_t[MAX_BARRIERS_PER_CTA]> reduction_storage;
};
