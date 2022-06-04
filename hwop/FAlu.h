#pragma once
#include "coasm.h"
#include <core.h>
#include <htl/decoupled.h>
#include <htl/float.h>
#include <logic.h>
#include "common.h"
// #include "utils.h"
#include <map>

using namespace ch::core;
using namespace ch::htl;


__enum (falu_state, (mul, add, sub));

#define F32_MUL(x, y, en) ufmul<FP32::M_, FP32::E_, FP32::TF_, 1>(x, y, en);
#define F32_ADD(x, y, en) ufadd<FP32::M_, FP32::E_, FP32::TF_, 1>(x, y, en);
#define F32_SUB(x, y, en) ufsub<FP32::M_, FP32::E_, FP32::TF_, 1>(x, y, en);


template <unsigned N>
class FALU {
public:
    __io (
        (ch_enq_io<falu_state>) op,
        __in (ch_uint<N>) lhs,
        __in (ch_uint<N>) rhs,
        (ch_valid_out<ch_uint<N>>) out
    );

    void describe() {
        ch_reg<ch_uint<N>> data(0);
        ch_reg<ch_bool> p(false);
        ch_reg<ch_uint<2>> count(0);

        // io.op.ready = !p;
        io.op.ready = count != 2;

        __if (io.op.valid && io.op.ready) {
            // p->next = true;
            count->next = count + 1;
            __switch (io.op.data)
            __case (falu_state::mul) {
                auto res = F32_MUL(io.lhs, io.rhs, io.op.data == falu_state::mul);
                data->next = res;
            }
            __case (falu_state::add) {
                auto res = F32_ADD(io.lhs, io.rhs, io.op.data == falu_state::add);
                data->next = res;
            }
            __case (falu_state::sub) {
                __if (io.op.data == falu_state::sub) {
                auto res = F32_SUB(io.lhs, io.rhs, io.op.data == falu_state::sub);
                data->next = res;
                };
            }
            ;
        };

        __if (io.out.valid) {
            // p->next = false;
            count->next = 0;
        };

        io.out.data = data;
        // io.out.valid = p;
        io.out.valid = count == 2;
        /*
        ch_println("data={}, out.data={}, out.valid={}, op.valid={}, op.ready={}, p={}",
                    data, io.out.data, io.out.valid, io.op.valid, io.op.ready, p);
                    */
    }
};
