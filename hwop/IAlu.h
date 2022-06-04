#pragma once
#include "coasm.h"
#include <core.h>
#include <htl/decoupled.h>
#include <logic.h>
#include "common.h"
// #include "utils.h"
#include <map>

using namespace ch::core;
using namespace ch::htl;


__enum (ialu_state, (mulhi, mullo, add, sub, and_, or_, xor_, not_, shr, shl, rotr, rotl));

template <unsigned N>
class IALU {
public:
    __io (
        (ch_enq_io<ialu_state>) op,
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
            __case (ialu_state::mulhi) {
                auto sum = ch_pad<N>(io.lhs) * io.rhs;
                data->next = ch_slice<N>(sum);
            }
            __case (ialu_state::mullo) {
                auto sum = ch_pad<N>(io.lhs) * io.rhs;
                data->next = ch_aslice<N>(sum, 1);
            }
            __case (ialu_state::add) {
                auto sum = ch_pad<1>(io.lhs) + io.rhs + io.cin;
                data->next = ch_slice<N>(sum);
                cout->next = sum[N];
            }
            __case (ialu_state::sub) {
                auto sub = ch_pad<1>(io.lhs) - io.rhs - io.cin;
                data->next = ch_slice<N>(sub);
                cout->next = sub[N];
            }
            __case (ialu_state::and_) {
                auto r = io.lhs & io.rhs;
                data->next = r;
            }
            __case (ialu_state::or_) {
                auto r = io.lhs | io.rhs;
                data->next = r;
            }
            __case (ialu_state::xor_) {
                auto r = io.lhs ^ io.rhs;
                data->next = r;
            }
            __case (ialu_state::not_) {
                auto r = ch_inv(io.lhs);  // use ~ ok
                data->next = r;
            }
            __case (ialu_state::shr) {
                auto r = ch_shr(io.lhs, io.rhs);
                data->next = r;
            }
            __case (ialu_state::shl) {
                auto r = ch_shl(io.lhs, io.rhs);
                data->next = r;
            }
            /*
            __case (ialu_state::rotr) {
                auto rhs = ch_slice<5>(io.rhs).as_uint();
                auto r = ch_rotr(io.lhs, static_cast<uint32_t>(rhs));
                data->next = r;
            }
            __case (ialu_state::rotl) {
                int32_t rhs = io.rhs.as_int();
                auto r = ch_rotl(io.lhs, rhs);
                data->next = r;
            };
            */
            ;
        };

        __if (io.out.valid) {
            p->next = false;
        };

        io.out.data = data;
        io.out.valid = p;
        io.cout = cout;

    }
};
