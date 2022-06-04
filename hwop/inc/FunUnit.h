#pragma once
#include <core.h>
#include <htl/decoupled.h>
#include <logic.h>
#include "IAlu.h"
#include "FAlu.h"
#include "TensorCore.h"

class FunUnit {
public:
    FunUnit();
    ~FunUnit();

    void dumpVCD(std::string &file_name);
    ch_device<IALU<32>> *ialu32;
    ch_device<FALU<32>> *falu32;
    ch_device<TensorCore<32>> *tensorcore;
    ch_device<IALU<64>> *ialu64;
    ch_tracer *ialu32_sim;
    ch_tracer *falu32_sim;
    ch_tracer *ialu64_sim;
    ch_tracer *tensorcore_sim;

    // std::shared_ptr<ch_tracer> ialu32_sim;
    //std::shared_ptr<ch_simulator> ialu64_sim;

    uint32_t add(uint32_t lhs, uint32_t rhs, uint32_t &cout, uint32_t cin = 0);
    uint32_t add_f32(uint32_t lhs, uint32_t rhs) ;
};


