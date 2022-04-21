#pragma once
#include <functional>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include "inc/ExecTypes.h"
// #include "abstract_hardware_model.h"
//
class Instruction;
using mem_access_ftype = void(uint64_t, size_t, void*, mem_space_t);
using dsm_access_ftype = void(uint64_t, size_t, void*);

class WarpState {
    enum class WarpStatus {
        READY,
        RUNNING,
        EXIT
    };
public:
	WarpState(uint32_t sreg_num,
              uint32_t vreg_num,
              uint32_t warp_size,
              uint32_t kernel_const_reg_num,
              std::function<dsm_access_ftype> dsm_read,
              std::function<dsm_access_ftype> dsm_write,
              std::function<mem_access_ftype> mem_read,
              std::function<mem_access_ftype> mem_write
            )
    {
        m_sreg_num = sreg_num;
        m_vreg_num = vreg_num;
        m_kernel_const_reg_num = kernel_const_reg_num;
        m_warp_size = warp_size;
        m_sreg = new uint32_t[sreg_num];
        m_vreg = new uint32_t[vreg_num*warp_size];
        m_status = new WarpStatus[warp_size];
        m_dsm_read = dsm_read;
        m_dsm_write = dsm_write;
        m_mem_read = mem_read;
        m_mem_write = mem_write;
        init();
    }
    ~WarpState() {
        delete m_sreg;
        delete m_vreg;
        delete m_status;
        if (m_dump.is_open()) {
            m_dump.close();
        }
    }

    void init();

    void initDump(std::string filename);

    uint32_t getSreg(uint32_t reg_id) ;
    void setSreg(uint32_t reg_id, uint32_t value);

    uint32_t getVreg(uint32_t reg_id, uint32_t lane_id);
    void setVreg(uint32_t reg_id, uint32_t value, uint32_t lane_id);

	void setBitmaskSreg(uint32_t sreg, uint32_t value, uint32_t lane_id);
	uint32_t getBitmaskSreg(uint32_t sreg, uint32_t lane_id);

    void setDmem(uint32_t addr, uint32_t length, char* value);
    void getDmem(uint32_t addr, uint32_t length, char* value);

    uint32_t getConst(uint32_t addr);



    void writeSMEM(uint64_t addr, uint32_t length, void *value, mem_space_t space = mem_space_t::undefined);
    void readSMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space = mem_space_t::undefined);
    void writeDMEM(uint64_t addr, uint32_t length, void *value, mem_space_t space = mem_space_t::undefined);
    void readDMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space = mem_space_t::undefined);
    void writeVMEM(uint64_t addr, uint32_t length, void *value, mem_space_t space = mem_space_t::undefined);
    void readVMEM(uint64_t addr, uint32_t length, void* value, mem_space_t space = mem_space_t::undefined);

    // void printSreg();
    // void printVreg() ;
    void dumpAddr(std::stringstream &ss, std::vector<uint64_t> &addr, uint32_t tmsk) ;
    void dumpSreg(std::stringstream &ss, uint32_t sreg);
    void dumpVreg(std::stringstream &ss, uint32_t vreg, uint32_t reg_num = 1, uint32_t data_size = 8);

    std::ofstream& out() {return m_dump;}
    void flush() { m_dump.flush();}

    uint64_t setupAddrSpace(uint64_t, mem_space_t &space);
    //MemoryPointer getVBaseAddr(uint32_t vreg, uint32_t lane_id); // call by VMEM
    MemoryPointer getDBaseAddr(uint32_t vreg, uint32_t lane_id); // call by DMEM
    MemoryPointer getSBaseAddr(uint32_t vreg); // call by SMEM
    uint64_t calculateAddr(uint32_t vreg);

    void executeInst(std::shared_ptr<Instruction> inst);
    void executeInst(uint64_t opcode);

    uint64_t getWarpPC() {
        return m_PC;
    }

    void setWarpPC(uint64_t pc) {
        m_PC = pc;
    }

    void incWarpPC(int increment) {
        m_PC += increment;
    }

    bool isLaneExit(uint32_t laneid) {
        if (m_status[laneid] == WarpStatus::EXIT)
            return true;
        else
            return false;
    }

    void setFinished(uint32_t lane_id) {
        m_status[lane_id] = WarpStatus::EXIT;
    }

    void getSregMemPtr(uint32_t sreg, MemoryPointer &mem_ptr);
    void getVregMemPtr(uint32_t vreg, MemoryPointer &mem_ptr, uint32_t lane_id);
	void ReadBufferResource(int sreg, BufferDescriptor &buffer_desc);

    /// Flag set during instruction emulation.
    void SetVectorMemoryGlobalCoherency(bool vector_mem_global_coherency)
    {
	    this->vector_memory_global_coherency = vector_mem_global_coherency;
    }

    void SetScalarMemoryRead(bool scalar_memory_read)
    {
	    this->scalar_memory_read = scalar_memory_read;
    }

    void SetMemoryWait(bool mem_wait) { this->memory_wait = mem_wait; }

    void setConstBuffer(uint32_t *const_buffer);
    void setStackPointer(uint64_t stack_pointer);
/*
    void setActiveMask(active_mask_t &active_mask) {
        m_active_mask = active_mask;
    };
*/
    std::function<dsm_access_ftype> m_dsm_read;
    std::function<dsm_access_ftype> m_dsm_write;
    std::function<mem_access_ftype> m_mem_read;
    std::function<mem_access_ftype> m_mem_write;

    uint64_t m_local_mem_stack_pointer;
    uint64_t m_param_addr;

    uint64_t m_global_memory_access_address;
    uint64_t m_global_memory_access_size;

    uint32_t m_smem_read_counter;

    bool vector_memory_global_coherency = false;
    bool scalar_memory_read = false;    // scalar memory read
    bool vector_memory_read = false;   // vecor memory write
    bool vector_memory_write = false;   // vecor memory write
    bool vector_memory_atomic = false;  // vector meory read
    bool ds_read = false;       // wait instruction
    bool ds_wait = false;       // wait instruction

    bool memory_wait = false;   // wai instruction

    WarpStatus *m_status;
    // active_mask_t m_active_mask;
    std::ofstream m_dump;

private:
    uint32_t m_sreg_num;
    uint32_t m_vreg_num;
    uint32_t m_kernel_const_reg_num;
    uint32_t m_warp_size;
    uint32_t *m_sreg;
    uint32_t *m_vreg;
    uint32_t *m_const_buffer;
    uint64_t m_PC;
};
