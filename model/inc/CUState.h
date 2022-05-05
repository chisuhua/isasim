#pragma once
#include <stdint.h>
#include <map>
#include <vector>
#include "inc/ExecContext.h"

class CUState {
public:
    CUState() {
    }

    void initStorage(uint32_t cta_uid) {
        auto itr = storage_.find(cta_uid);
        if (itr != storage_.end()) {
            for (int i = 0; i < MAX_BARRIERS_PER_CTA; i++) {
                storage_[cta_uid][i] = 0;
            }
        } else {
            storage_.insert(std::make_pair(cta_uid, std::vector<uint32_t>(MAX_BARRIERS_PER_CTA, 0)));
        }
    }

    void andReduction(unsigned cta_uid, unsigned barid, bool value) {
        storage_[cta_uid][barid] &= value;
    }
    void orReduction(unsigned cta_uid, unsigned barid, bool value) {
        storage_[cta_uid][barid] |= value;
    }
    void popcReduction(unsigned cta_uid, unsigned barid, bool value) {
        storage_[cta_uid][barid] += value;
    }
    unsigned getReductionValue(unsigned cta_uid, unsigned barid) {
        return storage_[cta_uid][barid];
    }
    std::map<uint32_t, std::vector<uint32_t>> storage_;
};
