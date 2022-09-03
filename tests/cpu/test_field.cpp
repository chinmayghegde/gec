#include <common.hpp>
#include <field.hpp>

#include <configured_catch.hpp>

using namespace gec;
using namespace bigint;

TEST_CASE("add group neg", "[add_group][field]") {
    using F = Field160;
    F e;
    F::neg(e, F());
    REQUIRE(e.is_zero());

    F::neg(e, F(0x1u));
    REQUIRE(F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu,
              0x5e3b0968u) == e);

    F::neg(e,
           F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu, 0x5e3b0968u));
    REQUIRE(F(0x1u) == e);

    F::neg(e,
           F(0x5bbc8155u, 0xec6dcb13u, 0xfaebe765u, 0x2e0bf7b6u, 0x2f1d84b4u));
    REQUIRE(F(0x5bbc8155u, 0xec6dcb13u, 0xfaebe765u, 0x2e0bf7b6u,
              0x2f1d84b5u) == e);
}

TEST_CASE("add group add", "[add_group][field]") {
    using F = Field160;
    F e;

    F::add(e, F(), F());
    REQUIRE(e.is_zero());

    F::add(e, F(1), F(2));
    REQUIRE(F(3) == e);

    F::add(e, F(0x2),
           F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu, 0x5e3b0966u));
    REQUIRE(F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu,
              0x5e3b0968u) == e);

    F::add(e, F(0x2),
           F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu, 0x5e3b0968u));
    REQUIRE(F(0x1) == e);

    F::add(e,
           F(0x0d1f4b5bu, 0x8005d7aau, 0x4fed62acu, 0x03831479u, 0x83ccd32du),
           F(0x1cfaec75u, 0x7faf7c19u, 0xd3121b9eu, 0xded3ca3bu, 0x952e1b38u));
    REQUIRE(F(0x2a1a37d0u, 0xffb553c4u, 0x22ff7e4au, 0xe256deb5u,
              0x18faee65u) == e);

    F::add(e,
           F(0x8f566078u, 0xb1d6a8dfu, 0xd5af7fadu, 0xaa89f612u, 0x240a6b52u),
           F(0x4a617461u, 0x4c8165c6u, 0xf378a372u, 0x8d6cccb6u, 0xd07f7850u));
    REQUIRE(F(0x223ed22eu, 0x257c787eu, 0xd3505455u, 0xdbded35cu,
              0x964eda39u) == e);
}

TEST_CASE("add group sub", "[add_group][field]") {
    using F = Field160;
    F e;

    F::sub(e, F(), F());
    REQUIRE(e.is_zero());

    F::sub(e, F(0xf0), F(0x2));
    REQUIRE(F(0xee) == e);

    F::sub(e,
           F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu, 0x5e3b0968u),
           F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu, 0x5e3b0966u));
    REQUIRE(F(0x2) == e);

    F::sub(e, F(0x1), F(0x2));
    REQUIRE(F(0xb77902abu, 0xd8db9627u, 0xf5d7cecau, 0x5c17ef6cu,
              0x5e3b0968u) == e);

    F::sub(e,
           F(0x2a1a37d0u, 0xffb553c4u, 0x22ff7e4au, 0xe256deb5u, 0x18faee65u),
           F(0x1cfaec75u, 0x7faf7c19u, 0xd3121b9eu, 0xded3ca3bu, 0x952e1b38u));
    REQUIRE(F(0x0d1f4b5bu, 0x8005d7aau, 0x4fed62acu, 0x03831479u,
              0x83ccd32du) == e);

    F::sub(e,
           F(0x223ed22eu, 0x257c787eu, 0xd3505455u, 0xdbded35cu, 0x964eda39u),
           F(0x4a617461u, 0x4c8165c6u, 0xf378a372u, 0x8d6cccb6u, 0xd07f7850u));
    REQUIRE(F(0x8f566078u, 0xb1d6a8dfu, 0xd5af7fadu, 0xaa89f612u,
              0x240a6b52u) == e);
}

TEST_CASE("mul_pow2", "[add_group][field]") {
    using F = Field160;

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());

    F a, a2, a4, a8, res;
    do {
        for (size_t i = 0; i < LN_160; ++i) {
            a.array()[i] = dis_u32(gen);
        }
    } while (a >= F::mod());
    F::add(a2, a, a);
    F::add(a4, a2, a2);
    F::add(a8, a4, a4);
    CAPTURE(a, a2, a4);

    res = a;
    F::add_self(res);
    REQUIRE(a2 == res);

    res = a;
    F::mul_pow2<1>(res);
    REQUIRE(a2 == res);

    res = a;
    F::mul_pow2<2>(res);
    REQUIRE(a4 == res);

    res = a;
    F::mul_pow2<3>(res);
    REQUIRE(a8 == res);
}

using SmallArray = ArrayBE<LIMB_T, 3>;
GEC_DEF(SmallMod, static const SmallArray, 0x0, 0xb, 0x7);

TEST_CASE("random sampling", "[add_group][field][random]") {
    using F1 = Field160;
    using F2 = Field160_2;
    using G = GEC_BASE_ADD_GROUP(SmallArray, SmallMod);

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    auto rng = make_gec_rng(std::mt19937(seed));

#define test(Int)                                                              \
    do {                                                                       \
        Int x, y, z;                                                           \
        for (int k = 0; k < 10000; ++k) {                                      \
            Int::sample(x, rng);                                               \
            REQUIRE(x < Int::mod());                                           \
                                                                               \
            Int::sample_non_zero(x, rng);                                      \
            REQUIRE(!x.is_zero());                                             \
            REQUIRE(x < Int::mod());                                           \
                                                                               \
            Int::sample(y, x, rng);                                            \
            REQUIRE(y < x);                                                    \
                                                                               \
            Int::sample(z, y, x, rng);                                         \
            REQUIRE(z < x);                                                    \
            REQUIRE(y <= z);                                                   \
                                                                               \
            Int::sample_inclusive(z, x, rng);                                  \
            REQUIRE(z <= x);                                                   \
                                                                               \
            Int::sample_inclusive(z, y, x, rng);                               \
            REQUIRE(z <= x);                                                   \
            REQUIRE(y <= z);                                                   \
        }                                                                      \
    } while (false)

    test(F1);
    test(F2);
    test(G);

#undef test
}

TEST_CASE("mul_pow2 bench", "[add_group][bench]") {
    using F = Field160;

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());

    F a;
    do {
        for (size_t i = 0; i < LN_160; ++i) {
            a.array()[i] = dis_u32(gen);
        }
    } while (a >= F::mod());

    BENCHMARK("add to 2a") {
        F res = a;
        F::add(res, a, a);
        return res;
    };

    BENCHMARK("add to 4a") {
        F res, tmp;
        F::add(tmp, a, a);
        F::add(res, tmp, tmp);
        return res;
    };

    BENCHMARK("add to 8a") {
        F res, tmp;
        F::add(res, a, a);
        F::add(tmp, res, res);
        F::add(res, tmp, tmp);
        return res;
    };

    BENCHMARK("add to 2^32 a") {
        F res, tmp;
        F::add(tmp, a, a);
        for (int k = 0; k < 31; ++k) {
            F::add(res, tmp, tmp);
            F::add(tmp, res, res);
        }
        F::add(res, tmp, tmp);
        return res;
    };

    BENCHMARK("mul 2") {
        F res = a;
        F::mul_pow2<1>(res);
        return res;
    };

    BENCHMARK("mul 4") {
        F res = a;
        F::mul_pow2<2>(res);
        return res;
    };

    BENCHMARK("mul 8") {
        F res = a;
        F::mul_pow2<3>(res);
        return res;
    };

    BENCHMARK("mul 2^32") {
        F res = a;
        F::mul_pow2<32>(res);
        return res;
    };
}

TEST_CASE("montgomery mul", "[ring][field]") {
    using F = Field160;

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());

    F a, b;

    F::to_montgomery(a, F());
    REQUIRE(F(0) == a);

    F::from_montgomery(b, a);
    REQUIRE(F(0) == b);

    F::to_montgomery(a, F(0xffffffffu));
    REQUIRE(F(0xad37b410u, 0x255c6eb2u, 0x7601a883u, 0x659883e8u,
              0x070707fcu) == a);

    F::from_montgomery(b, a);
    REQUIRE(F(0xffffffffu) == b);

    F c, d, e;
    do {
        for (size_t i = 0; i < LN_160; ++i) {
            c.array()[i] = dis_u32(gen);
        }
    } while (c >= F::mod());

    d = c;
    F::to_montgomery(e, d);
    F::from_montgomery(d, e);
    REQUIRE(c == d);

    LIMB_T l, h, x, y;
    F mon_x, mon_y, mon_xy, xy;

    x = 0xd8b2f21eu;
    y = 0xabf7c642u;
    utils::uint_mul_lh(l, h, x, y);
    F::to_montgomery(mon_x, F(x));
    F::to_montgomery(mon_y, F(y));
    F::mul(mon_xy, mon_x, mon_y);
    F::from_montgomery(xy, mon_xy);
    REQUIRE(l == xy.array()[0]);
    REQUIRE(h == xy.array()[1]);

    x = dis_u32(gen);
    y = dis_u32(gen);
    utils::uint_mul_lh(l, h, x, y);
    F::to_montgomery(mon_x, F(x));
    F::to_montgomery(mon_y, F(y));
    F::mul(mon_xy, mon_x, mon_y);
    F::from_montgomery(xy, mon_xy);
    REQUIRE(l == xy.array()[0]);
    REQUIRE(h == xy.array()[1]);

    x = dis_u32(gen);
    y = dis_u32(gen);
    utils::uint_mul_lh(l, h, x, y);
    F::to_montgomery(mon_x, F(x));
    F::to_montgomery(mon_y, F(y));
    F::mul(mon_xy, mon_x, mon_y);
    F::from_montgomery(xy, mon_xy);
    REQUIRE(l == xy.array()[0]);
    REQUIRE(h == xy.array()[1]);

    mon_x = F(0xa5481e14u, 0x293b3c7du, 0xb85ecae1u, 0x83d79492u, 0xcd652763u);
    mon_y = F(0x93d20f51u, 0x898541bbu, 0x74aa1184u, 0xbccb10b2u, 0x47f79c2cu);
    F::mul(mon_xy, mon_x, mon_y);
    REQUIRE(F(0x4886fd54u, 0x272469d8u, 0x0a283135u, 0xa3e81093u,
              0xa1c4f697u) == mon_xy);
}

#ifdef GEC_ENABLE_AVX2

TEST_CASE("avx2 montgomery", "[ring][avx2]") {
    using gec::utils::CmpEnum;
    using gec::utils::VtSeqCmp;
    using Int = GEC_BASE_ADD_GROUP(Array256, MOD_256);

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    auto rng = make_gec_rng(std::mt19937(seed));

    Array256 x_arr(0x1f82f372u, 0x62639538u, 0xca640ff9u, 0xed12396au,
                   0x9c4d50dau, 0xff21e339u, 0xfbfa64d8u, 0x75b40000u);
    Array256 y_arr(0xed469d79u, 0xaba8d6fau, 0x6724432cu, 0x7221f040u,
                   0x6416351du, 0x923ec2cau, 0x72bc1127u, 0xf1e018aau);
    Int &x_int = static_cast<Int &>(x_arr);
    Int &y_int = static_cast<Int &>(y_arr);
    Array256 mon_x_arr, mon_y_arr, mon_xy_arr, xy_arr;

    for (int k = 0; k < 10000; ++k) {
        Int::sample(x_int, rng);
        Int::sample(y_int, rng);
        CAPTURE(Int::mod(), x_int, y_int);

        {
            using F =
                GEC_BASE_FIELD(Array256, MOD_256, MOD_P_256, RR_256, OneR_256);
            const auto &x = static_cast<F &>(x_arr);
            const auto &y = static_cast<F &>(y_arr);
            auto &mon_x = static_cast<F &>(mon_x_arr);
            auto &mon_y = static_cast<F &>(mon_y_arr);
            auto &mon_xy = static_cast<F &>(mon_xy_arr);
            auto &xy = static_cast<F &>(xy_arr);

            F::to_montgomery(mon_x, x);
            F::to_montgomery(mon_y, y);
            F::mul(mon_xy, mon_x, mon_y);
            F::from_montgomery(xy, mon_xy);
        }
        CAPTURE(mon_x_arr, mon_y_arr);
        CAPTURE(mon_xy_arr, xy_arr);

        {
            using F = GEC_BASE_AVX2FIELD(Array256, MOD_256, MOD_P_256, RR_256,
                                         OneR_256);
            const auto &x = static_cast<F &>(x_arr);
            const auto &y = static_cast<F &>(y_arr);
            const auto &expected_mon_x = static_cast<F &>(mon_x_arr);
            const auto &expected_mon_y = static_cast<F &>(mon_y_arr);
            const auto &expected_mon_xy = static_cast<F &>(mon_xy_arr);
            const auto &expected_xy = static_cast<F &>(xy_arr);

            F mon_x, mon_y, mon_xy, xy;
            F::to_montgomery(mon_x, x);
            CAPTURE(mon_x);
            REQUIRE(expected_mon_x == mon_x);
            F::to_montgomery(mon_y, y);
            CAPTURE(mon_y);
            REQUIRE(expected_mon_y == mon_y);
            F::mul(mon_xy, mon_x, mon_y);
            CAPTURE(mon_xy);
            REQUIRE(expected_mon_xy == mon_xy);
            F::from_montgomery(xy, mon_xy);
            CAPTURE(xy);
            REQUIRE(expected_xy == xy);
        }
    }
}

TEST_CASE("256 montgomery bench", "[ring][avx2][bench]") {
    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    auto rng = make_gec_rng(std::mt19937(seed));
    using Int = GEC_BASE_ADD_GROUP(Array256, MOD_256);
    using SerialF =
        GEC_BASE_FIELD(Array256, MOD_256, MOD_P_256, RR_256, OneR_256);
    using AVX2F =
        GEC_BASE_AVX2FIELD(Array256, MOD_256, MOD_P_256, RR_256, OneR_256);

    Array256 x_arr, y_arr, mon_x_arr, mon_y_arr;
    Int &x_int = static_cast<Int &>(x_arr);
    Int &y_int = static_cast<Int &>(y_arr);

    Int::sample(x_int, rng);
    Int::sample(y_int, rng);

    {
        using F = SerialF;
        const auto &x = static_cast<const F &>(x_arr);
        const auto &y = static_cast<const F &>(y_arr);
        auto &mon_x = static_cast<F &>(mon_x_arr);
        auto &mon_y = static_cast<F &>(mon_y_arr);

        F::to_montgomery(mon_x, x);
        F::to_montgomery(mon_y, y);
    }

    {
        using F = SerialF;
        const auto &x = static_cast<const F &>(x_arr);
        const auto &mon_x = static_cast<const F &>(mon_x_arr);
        const auto &mon_y = static_cast<const F &>(mon_y_arr);

        BENCHMARK("into montgomery form") {
            F res;
            F::to_montgomery(res, x);
            return res;
        };

        BENCHMARK("from montgomery form") {
            F res;
            F::from_montgomery(res, mon_x);
            return res;
        };

        BENCHMARK("montgomery mul") {
            F mon_xy;
            F::mul(mon_xy, mon_x, mon_y);
            return mon_xy;
        };
    }

    {
        using F = AVX2F;
        const auto &x = static_cast<const F &>(x_arr);
        const auto &mon_x = static_cast<const F &>(mon_x_arr);
        const auto &mon_y = static_cast<const F &>(mon_y_arr);

        BENCHMARK("avx2 into montgomery form") {
            F res;
            F::to_montgomery(res, x);
            return res;
        };

        BENCHMARK("avx2 from montgomery form") {
            F res;
            F::from_montgomery(res, mon_x);
            return res;
        };

        BENCHMARK("avx2 montgomery mul") {
            F mon_xy;
            F::mul(mon_xy, mon_x, mon_y);
            return mon_xy;
        };
    }
}

#endif // GEC_ENABLE_AVX2

TEST_CASE("montgomery inv", "[field]") {
    using F = Field160;

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());

    F a, mon_a, inv_a, mon_prod, prod;
    for (int k = 0; k < 10000; ++k) {
        do {
            for (size_t k = 0; k < LN_160; ++k) {
                a.array()[k] = dis_u32(gen);
            }
        } while (a >= F::mod());
        // a = Field(0x31a50ad6u, 0x93f524b7u, 0xa6ea2efeu, 0xed31237au,
        //           0x2d2731f7u);
        F::to_montgomery(mon_a, a);
        F::inv(inv_a, mon_a);
        F::mul(mon_prod, mon_a, inv_a);
        F::from_montgomery(prod, mon_prod);
        CAPTURE(prod);
        REQUIRE(prod.is_one());
    }
}

TEST_CASE("montgomery exp", "[field]") {
    using F = Field160;

    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());

    F mod_m, a, mon_a, mon_exp_a, exp_a;
    F::sub(mod_m, F::mod(), 1);

    for (int k = 0; k < 10000; ++k) {
        do {
            for (size_t k = 0; k < LN_160; ++k) {
                a.array()[k] = dis_u32(gen);
            }
        } while (a >= F::mod() && !a.is_zero());
        // a = Field(0x31a50ad6u, 0x93f524b7u, 0xa6ea2efeu, 0xed31237au,
        //           0x2d2731f7u);
        F::to_montgomery(mon_a, a);

        F::pow(mon_exp_a, mon_a, 1u);
        REQUIRE(mon_exp_a == mon_a);

        F::pow(mon_exp_a, mon_a, 0u);
        F::from_montgomery(exp_a, mon_exp_a);
        REQUIRE(exp_a.is_one());

        F::pow(mon_exp_a, mon_a, F::mod());
        REQUIRE(mon_exp_a == mon_a); // Fermat's Little Theorem

        F::pow(mon_exp_a, mon_a, mod_m);
        F::from_montgomery(exp_a, mon_exp_a);
        REQUIRE(exp_a.is_one()); // Fermat's Little Theorem
    }
}

TEST_CASE("montgomery mul bench", "[ring][field][bench]") {
    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());
    std::uniform_int_distribution<LIMB2_T> dis_u64(
        std::numeric_limits<LIMB2_T>::min(),
        std::numeric_limits<LIMB2_T>::max());
    Field160_2 x0, y0, mon_x0, mon_y0;
    do {
        x0.array()[0] = dis_u64(gen);
        x0.array()[1] = dis_u64(gen);
        x0.array()[2] = dis_u32(gen);
    } while (x0 >= Field160_2::mod());
    do {
        y0.array()[0] = dis_u64(gen);
        y0.array()[1] = dis_u64(gen);
        y0.array()[2] = dis_u32(gen);
    } while (y0 >= Field160_2::mod());
    Field160_2::to_montgomery(mon_x0, x0);
    Field160_2::to_montgomery(mon_y0, y0);

    {
        using F = Field160;
        const F &x = *reinterpret_cast<const F *>(x0.array());
        const F &mon_x = *reinterpret_cast<const F *>(mon_x0.array());
        const F &mon_y = *reinterpret_cast<const F *>(mon_y0.array());

        BENCHMARK("32-bits into montgomery form") {
            F res;
            F::to_montgomery(res, x);
            return res;
        };

        BENCHMARK("32-bits from montgomery form") {
            F res;
            F::from_montgomery(res, mon_x);
            return res;
        };

        BENCHMARK("32-bits montgomery mul") {
            F xy;
            F::mul(xy, mon_x, mon_y);
            return xy;
        };
    }

    {
        using F = Field160_2;
        const F &x = reinterpret_cast<const F &>(x0);
        const F &mon_x = reinterpret_cast<const F &>(mon_x0);
        const F &mon_y = reinterpret_cast<const F &>(mon_y0);

        BENCHMARK("64-bits into montgomery form") {
            F res;
            F::to_montgomery(res, x);
            return res;
        };

        BENCHMARK("64-bits from montgomery form") {
            F res;
            F::from_montgomery(res, mon_x);
            return res;
        };

        BENCHMARK("64-bits montgomery mul") {
            F xy;
            F::mul(xy, mon_x, mon_y);
            return xy;
        };
    }
}

TEST_CASE("montgomery inv bench", "[field][bench]") {
    std::random_device rd;
    auto seed = rd();
    INFO("seed: " << seed);
    std::mt19937 gen(seed);

    std::uniform_int_distribution<LIMB_T> dis_u32(
        std::numeric_limits<LIMB_T>::min(), std::numeric_limits<LIMB_T>::max());
    std::uniform_int_distribution<LIMB2_T> dis_u64(
        std::numeric_limits<LIMB2_T>::min(),
        std::numeric_limits<LIMB2_T>::max());
    Field160_2 x0, mon_x0;
    do {
        x0.array()[0] = dis_u64(gen);
        x0.array()[1] = dis_u64(gen);
        x0.array()[2] = dis_u32(gen);
    } while (x0 >= Field160_2::mod());
    Field160_2::mul(mon_x0, x0, Field160_2::r_sqr());

    {
        using F = Field160;
        const F &mon_x = *reinterpret_cast<const F *>(mon_x0.array());
        BENCHMARK("32-bits montgomery inv") {
            F inv_x;
            F::inv(inv_x, mon_x);
            return inv_x;
        };
    }

    {
        using F = Field160_2;
        const F &mon_x = reinterpret_cast<const F &>(mon_x0);
        BENCHMARK("64-bits montgomery inv") {
            F inv_x;
            F::inv(inv_x, mon_x);
            return inv_x;
        };
    }
}

template <typename F>
static void test_mod_sqrt(std::random_device::result_type seed) {
    CAPTURE(seed);

    auto rng = make_gec_rng(std::mt19937(seed));
    F x, xx, sqrt, sqr;
    for (int k = 0; k < 1000; ++k) {
        F::sample(x, rng);
        F::mul(xx, x, x);
        CAPTURE(x, xx);
        REQUIRE(F::mod_sqrt(sqrt, xx, rng));
        CAPTURE(sqrt);
        F::mul(sqr, sqrt, sqrt);
        REQUIRE(xx == sqr);
    }
}
TEST_CASE("montgomery mod_sqrt", "[field][quadratic_residue]") {
    std::random_device rd;
    test_mod_sqrt<Field160>(rd());
    test_mod_sqrt<Field160_2>(rd());
}

TEST_CASE("bigint hash", "[field][hash]") {
    using F = Field160;
    F::Hasher h;

    F Zero(0), One(1);

    REQUIRE(h(Zero) != h(One));
}