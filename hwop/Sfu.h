#include <cmath>
#include <stdint>

template <typename ufloat_t = IEEE754<23, 8 , 0>>
class SFU {
public:
    explicit SFU() {};
    virtual ~SFU() {};

    struct table_param {
        int32_t index;
        int32_t delta;
        int32_t table_index;
        uint32_t input;
        bool use_x;
        uint32_t x;
        int32_t biased_exp;
        int32_t res_exp;
        int32_t quadrant;
        uint32_t res_sign;
        int32_t res_scale;
    }

    struct table_item {
        uint32_t C0;
        uint32_t C1;
        uint32_t C2;
        uint32_t C3;
    }

    ufloat_t::fixed fixed_data;
    ufloat_t::primitive float_data;

    struct table_width {
        int32_t C0;
        int32_t C1;
        int32_t C2;
        int32_t C3;
    }
    // virtual sfu_special(ufloat_t) = 0;
    // virtual sfu_result(uint64_t mac_res, table_param param) = 0;

    uint32_t calculate(ufloat_t input) {
        // return sfu_special();
        table_param param = table_param(input);
        table_item item = table_lookup(param);
        item = table_signext(item, param);
        uint64_t mac_res = MAC(item, param);
        ufloat_t res = sfu_result(mac_res, param);
    };

    uint32_t calexp(ufloat_t input);

    table_param table_param(ufloat_t input);

    table_item table_lookup(table_param p) {
        auto idx = p.table_idx * table_size_ + p.idx;
        return TABLE[idx];
    };

    uint64_t MAC(table_item item, table_param p) {
        uint64_t x = p.x;
        // X is unsign
        uint64_t C3xC2 = sign_ext(item.C3, table_sbits_[3]) * x +
                        (sign_ext(item.C2, table_sbits_[2]) << 19);
        uint64_t C1xC0 = sign_ext(item.C1, table_sbits_[1]) * x +
                        (sign_ext(item.C0, table_sbits_[0]) << 19);
        uint64_t Xsqrt = x * x;
        uint64_t result = bit_range(Xsqrt, 41, 16) * sign_ext(bit_range(C3xC2, 35, 14), 21) + (C1xC0 << 8);
        return result;
    };

    void get_rounding(uint64_t mant, int32_t msb, uint32_t &rounding) {
        assert(msb >= 0 && msb <= 63)
        rounding = (mant >> msb) & 0x1;
        /*
        R = (msb <= 0) ? 0 : (mant >> (msb - 1)) & 0x1;
        S = (msb <= 1) ? 0 : (mant & ((1ULL << (msb - 1)) - 1)) == 0 ? 0 : 1;
        */
    }

    ufloat_t sfu_sin_result(uint64_t mac_res, table_param param) {
        int32_t biased_exp = param.biased_exp;
        uint32_t mant = 0;
        uint32_t rounding = 0;

        if (mac_res & (uint64_t)0x1ULL << 53) {
            mant = ufloat_t::bit_range(mac_res, 52, 52-M+1);
            get_rounding(bit_range(mac_res, 52-M, 0), 52-M, rounding);
            /*
            ufloat_t v = ufloat_t::from_components(mac_res, p.biased_exp + ufloat_t::B, p.x);
            if (!(v >= 0.5 && v <= 1))
                biased_exp++;
                */
        } else {
            mant = ufloat_t::bit_range(mac_res, 51, 51-M+1);
            get_rounding(bit_range(mac_res, 51-M, 0), 51-M, rounding);
        }
        ufloat_t res = ufloat_t::from_components(param.res_sign, biased_exp + ufloat_t::B,
                                        bit_range(mant, ufloat_t::M - 1, 0));
        res.roundup_float(rounding);
        return res;
    }

    ufloat_t sfu_rcp_result(uint64_t mac_res, table_param param) {
        int32_t biased_exp = param.biased_exp;
        int32_t res_exp = param.res_exp;
        uint32_t rounding_bits = 0;
        uint32_t rounding_msb = 0;
        uint32_t rounding = 0;
        assert(bit_range(mac_res, 55, 54) == 0);
        assert(((mac_res & 1ULL << 53) && param.delta == 0 && param.index == 0) ||
                (mac_res & 1ULL << 53) == 0);

        if (mac_res & 1ULL << 53) res_exp++;
        int32_t src_float = ufloat_t::from_data(param.input).comp.exponent;

        // denormal check
        if (src_exp == 0xfd) {
            mant = ufloat_t::bit_range(mac_res, 53, 53 - M);
            rounding_msb = 53-M-1;
        } if (src_exp == 0xfe) {
            mant = ufloat_t::bit_range(mac_res, 53, 53 - M + 1);
            rounding_msb = 53-M;
        } else {
            mant = ufloat_t::bit_range(mac_res, 51, 53 - M + 1);
            rounding_msb = 51-M;
        }
        rounding_bits = ufloat_t::bit_range(mac_res, rounding_msb, 0);

        get_rounding(rounding_bits, rounding_msb, rounding);

        if (res_exp <= 0) res_exp = 0;
        if (res_exp >= 0xff) return std::numeric_limits<ufloat_t>::infinity(src_float.sign);
        auto dst_float = ufloat_t::from_components(src_float.sign, res_exp, mant);
        dst_float.roundup_float(rounding);
    }

    ufloat_t sfu_exp2_result(uint64_t mac_res, table_param param) {
        int32_t res_exp = param.res_exp;
        bool res_normal = (res_exp > 0);

        if (res_normal) {
            int32_t leadingonebit = count_leading_zeros(mac_res);
            assert(leadingonebit == 54);
            ufloat_t res = ufloat_t::from_mant_to_round(0, res_exp, mac_res, 53);
            return res;
        } else {
            mant = ufloat_t::bit_range(mac_res, 54, 31);
            // shift
            if (res_exp <= 0) {
                mant = (-res_exp) > 23 ? 0 : mant >> (-res_exp);
                res_exp = 0;
            }
            ufloat_t res = ufloat_t::from_mant_to_round(0, res_exp, mant, 23);
            return res;
        }
    }

    ufloat_t sfu_log2_result(uint64_t mac_res, table_param param) {
        int32_t res_exp = param.res_exp;
        bool res_normal = (res_exp > 0);
    }

    ufloat_t sfu_sqrt_result(uint64_t mac_res, table_param param) {
    }

    ufloat_t sfu_rsqrt_result(uint64_t mac_res, table_param param) {
    }

    ufloat_t sfu_tanh_result(uint64_t mac_res, table_param param) {
    }

    ufloat_t sfu_sigmod_result(uint64_t mac_res, table_param param) {
    }

    int32_t table_size_ = 4;
    int32_t table_bits_ = 4;

    table_item TABLE[100];
}

#if 0
template <typename ufloat_t = IEEE754<23, 8 , 0>>
class SIN : SFU<ufloat_t> {
public:
    virtual sfu_special(ufloat_t) = 0;

}
#endif

