#pragma once
#include "coasm.h"
#include <core.h>
#include <htl/decoupled.h>
#include "common.h"
// #include "utils.h"
#include <map>

using namespace ch::core;
using namespace ch::htl;


__enum (alu_state, (add, sub ));

template <unsigned N>
class IALU {
public:
    __io (
        (ch_enq_io<alu_state>) op,
        __in (ch_uint<N>) lhs,
        __in (ch_uint<N>) rhs,
        __in (ch_uint1)   cin,
        (ch_valid_out<ch_uint<N>>) out,
        __out (ch_uint1) cout
    );

    void describe() {
        ch_reg<ch_uint<N>> data(0);
        ch_reg<ch_uint1> cout(0);
        ch_reg<ch_bool> p(false);

        io.op.ready = !p;

        __if (io.op.valid && io.op.ready) {
            p->next = true;
            __switch (io.op.data)
            __case (alu_state::add) {
                auto sum = ch_pad<1>(io.lhs) + io.rhs + io.cin;
                data->next = ch_slice<N>(sum);
                cout->next = sum[N];
            }
            __case (alu_state::sub) {
                auto sub = ch_pad<1>(io.lhs) - io.rhs - io.cin;
                data->next = ch_slice<N>(sub);
                cout->next = sub[N];
            };
        };

        __if (io.out.valid) {
            p->next = false;
        };

        io.out.data = data;
        io.out.valid = p;
        io.cout = cout;
    }
};

class HwOp {
public:
    HwOp();
    ~HwOp();

    ch_device<IALU<32>> *ialu32;
    ch_device<IALU<64>> *ialu64;
    ch_simulator *ialu32_sim;
    ch_simulator *ialu64_sim;

    // std::shared_ptr<ch_tracer> ialu32_sim;
    //std::shared_ptr<ch_simulator> ialu64_sim;

    uint32_t add(uint32_t lhs, uint32_t rhs, uint32_t &cout, uint32_t cin = 0);
};


