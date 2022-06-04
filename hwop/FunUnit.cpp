#include "inc/FunUnit.h"
#include "IAlu.h"
#include "FAlu.h"

FunUnit::FunUnit() {
    ialu32 = new ch_device<IALU<32>>;
    ialu64 = new ch_device<IALU<64>>;
    falu32 = new ch_device<FALU<32>>;
    tensorcore = new ch_device<TensorCore<32>>;

    ialu32_sim = new ch_tracer(*ialu32);
    ialu64_sim = new ch_tracer(*ialu64);
    falu32_sim = new ch_tracer(*falu32);
    tensorcore_sim = new ch_tracer(*tensorcore);
    //
    // ialu32_sim = std::make_shared<ch_tracer>(ialu32);
    //ialu64_sim = std::make_shared<ch_simulator>(ialu64);
}

void FunUnit::dumpVCD(std::string &file_name) {
    falu32_sim->toText(file_name + "_falu32.log");
    falu32_sim->toVCD(file_name + "_falu32.vcd");

    ialu32_sim->toText(file_name + "_ialu32.log");
    ialu32_sim->toVCD(file_name + "_ialu32.vcd");
    //falu32_sim->toVerilog(file_name + "_falu32_tb.v", file_name + "_falu32.v", true);
}

FunUnit::~FunUnit() {
    delete ialu32_sim;
    delete falu32_sim;
    delete ialu64_sim;
    delete tensorcore_sim;
    delete ialu32;
    delete falu32;
    delete ialu64;
    delete tensorcore;
}

uint32_t FunUnit::add(uint32_t lhs, uint32_t rhs, uint32_t &cout, uint32_t cin) {
    ialu32->io.cin = cin;
    ialu32->io.lhs = lhs;
    ialu32->io.rhs = rhs;
    ialu32->io.op.valid = 1;
    ialu32->io.op.data = ialu_state::add;
    ialu32_sim->run([&](ch_tick t)->bool {
        return !ialu32->io.out.valid;
    }, 2);
    auto sum = static_cast<uint32_t>(ialu32->io.out.data);
	cout =  static_cast<uint32_t>(ialu32->io.cout);
    return sum;
}

uint32_t FunUnit::add_f32(uint32_t lhs, uint32_t rhs) {
    falu32->io.lhs = lhs;
    falu32->io.rhs = rhs;
    falu32->io.op.valid = 1;
    falu32->io.op.data = falu_state::add;
    falu32_sim->run([&](ch_tick t)->bool {
        return !falu32->io.out.valid;
    }, 2);
    auto sum = static_cast<uint32_t>(falu32->io.out.data);
    falu32->io.op.valid = 0;
    return sum;
}
