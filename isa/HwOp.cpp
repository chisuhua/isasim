#include "inc/HwOp.h"

HwOp::HwOp() {
    ialu32 = new ch_device<IALU<32>>;
    ialu64 = new ch_device<IALU<64>>;

    ialu32_sim = new ch_simulator(*ialu32);
    ialu64_sim = new ch_simulator(*ialu64);
    //
    // ialu32_sim = std::make_shared<ch_tracer>(ialu32);
    //ialu64_sim = std::make_shared<ch_simulator>(ialu64);
}

HwOp::~HwOp() {
    delete ialu32_sim;
    delete ialu64_sim;
    delete ialu32;
    delete ialu64;
}

uint32_t HwOp::add(uint32_t lhs, uint32_t rhs, uint32_t &cout, uint32_t cin) {
    ialu32->io.cin = cin;
    ialu32->io.lhs = lhs;
    ialu32->io.rhs = rhs;
    ialu32->io.op.valid = 1;
    ialu32->io.op.data = alu_state::add;
    ialu32_sim->run([&](ch_tick t)->bool {
        return !ialu32->io.out.valid;
    }, 2);
    auto sum = static_cast<uint32_t>(ialu32->io.out.data);
	cout =  static_cast<uint32_t>(ialu32->io.cout);
    return sum;
}

