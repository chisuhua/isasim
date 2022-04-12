#pragma once

#include <vector>
#include <bitset>
#include <deque>
#include "inc/ExecContext.h"
// #include "../../libcuda/abstract_hardware_model.h"

namespace libcuda {
class gpgpu_context;
}

typedef unsigned long long addr_t;

  enum stack_entry_type { STACK_ENTRY_TYPE_NORMAL = 0, STACK_ENTRY_TYPE_CALL };
    struct SimtStack_entry {
        addr_t m_PC;
        uint32_t m_calldepth;
        simt_mask_t m_active_mask;
        addr_t m_recvg_pc;
        uint64_t m_branch_div_cycle;
        stack_entry_type m_type;
        SimtStack_entry()
            : m_PC(-1),
            m_calldepth(0),
            m_active_mask(),
            m_recvg_pc(-1),
            m_branch_div_cycle(0),
            m_type(STACK_ENTRY_TYPE_NORMAL){};
    };


class Warp {
 public:

  Warp(unsigned wid, unsigned warpSize, libcuda::gpgpu_context *ctx);

  void reset();
  void launch(addr_t start_pc, const simt_mask_t &active_mask);
  void update(simt_mask_t &thread_done, std::vector<addr_t> &next_pc,
              addr_t recvg_pc, op_type_t next_inst_op,
              unsigned next_inst_size, addr_t next_inst_pc);

  const simt_mask_t &get_simt_active_mask() const;
  void set_simt_active_mask(const active_mask_t &active, bool isatomic);
  void update_simt_active_mask(bool isatomic_op) {
    set_simt_active_mask(get_simt_active_mask(), isatomic_op);
  };


  void get_pdom_stack_top_info(unsigned *pc, unsigned *rpc) const;
  unsigned get_rp() const;
  void print(FILE *fp) const;
  void resume(char * fname) ;
  void print_checkpoint (FILE *fout) const;

  unsigned getWarpId() {return m_warp_id; }

 protected:
  unsigned m_warp_id;
  unsigned m_warp_size;



  std::deque<SimtStack_entry> m_stack;

  libcuda::gpgpu_context *m_gpgpu_ctx;

public:
  // void setSReg(int sreg, unsigned value);
  // unsigned getSregUint(int sreg_id) const;
  // void setSregUint(int id, unsigned int value);

  addr_t GetWarpPC() { return m_PC;}
  void SetWarpPC(unsigned pc) { m_PC = pc; }
  void IncWarpPC(int increment) { m_PC += increment; }

  void SetFinished(bool finished) { this->finished = finished; }

  // FIXME m_PC should converged for SimtStack_entry
  unsigned m_PC;
  // Register sreg[256];

  bool at_barrier = false;    // barrier instruction
  bool finished = false;      // warp is done
  bool barrier_instruction = false;

  class WarpState *m_warp_state;

};


