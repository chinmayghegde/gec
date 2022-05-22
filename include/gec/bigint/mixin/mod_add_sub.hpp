#pragma once
#ifndef GEC_BIGINT_MIXIN_MOD_ADD_SUB_HPP
#define GEC_BIGINT_MIXIN_MOD_ADD_SUB_HPP

#include <gec/utils/arithmetic.hpp>
#include <gec/utils/crtp.hpp>
#include <gec/utils/sequence.hpp>

namespace gec {

namespace bigint {

template <class Core, size_t K, typename LIMB_T, size_t LIMB_N,
          const LIMB_T *MOD>
struct MulPow2Helper {
    __host__ __device__ GEC_INLINE static void call(Core &GEC_RSTRCT a) {
        constexpr size_t Idx = LIMB_N - 1;
        constexpr LIMB_T Mask = LIMB_T(1)
                                << (std::numeric_limits<LIMB_T>::digits - 1);

        bool carry = bool(a.array()[Idx] & Mask);
        utils::seq_shift_left<LIMB_N, 1>(a.array());
        if (carry || utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
                         utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
        MulPow2Helper<Core, K - 1, LIMB_T, LIMB_N, MOD>::call(a);
    }
};

template <class Core, typename LIMB_T, size_t LIMB_N, const LIMB_T *MOD>
struct MulPow2Helper<Core, 0, LIMB_T, LIMB_N, MOD> {
    __host__ __device__ GEC_INLINE static void call(Core &GEC_RSTRCT) {}
};

/** @brief mixin that enables addition and substrcation operation
 *
 * require `Core::is_zero`, `Core::set_zero` methods
 */
template <class Core, typename LIMB_T, size_t LIMB_N, const LIMB_T *MOD>
class ModAddSub : protected CRTP<Core, ModAddSub<Core, LIMB_T, LIMB_N, MOD>> {
    friend CRTP<Core, ModAddSub<Core, LIMB_T, LIMB_N, MOD>>;

  public:
    __host__ __device__ GEC_INLINE static const Core &mod() {
        return *reinterpret_cast<const Core *>(MOD);
    }

    /** @brief a = b + c (mod MOD)
     */
    static void add(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                    const Core &GEC_RSTRCT c) {
        bool carry = utils::seq_add<LIMB_N>(a.array(), b.array(), c.array());
        if (carry || utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
                         utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a + c (mod MOD)
     */
    static void add(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        bool carry = utils::seq_add<LIMB_N>(a.array(), b.array());
        if (carry || utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
                         utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = - b (mod MOD)
     */
    static void neg(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        if (b.is_zero()) {
            a.set_zero();
        } else {
            utils::seq_sub<LIMB_N>(a.array(), MOD, b.array());
        }
    }

    /** @brief a = b - c (mod MOD)
     */
    static void sub(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                    const Core &GEC_RSTRCT c) {
        bool borrow = utils::seq_sub<LIMB_N>(a.array(), b.array(), c.array());
        if (borrow) {
            utils::seq_add<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a - b (mod MOD)
     */
    static void sub(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        bool borrow = utils::seq_sub<LIMB_N>(a.array(), b.array());
        if (borrow) {
            utils::seq_add<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a * 2^K (mod MOD)
     */
    template <size_t K>
    __host__ __device__ static void mul_pow2(Core &GEC_RSTRCT a) {
        MulPow2Helper<Core, K, LIMB_T, LIMB_N, MOD>::call(a);
    }

    /** @brief a = 2 * a (mod MOD)
     */
    __host__ __device__ static void add_self(Core &GEC_RSTRCT a) {
        MulPow2Helper<Core, 1, LIMB_T, LIMB_N, MOD>::call(a);
    }
};

template <class Core, size_t K, typename LIMB_T, size_t LIMB_N,
          const LIMB_T *MOD>
struct CarryFreeMulPow2Helper {
    __host__ __device__ GEC_INLINE static void call(Core &GEC_RSTRCT a) {
        utils::seq_shift_left<LIMB_N, K>(a.array());
        if (utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
            utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
        CarryFreeMulPow2Helper<Core, K - 1, LIMB_T, LIMB_N, MOD>::call(a);
    }
};

template <class Core, typename LIMB_T, size_t LIMB_N, const LIMB_T *MOD>
struct CarryFreeMulPow2Helper<Core, 0, LIMB_T, LIMB_N, MOD> {
    __host__ __device__ GEC_INLINE static void call(Core &GEC_RSTRCT) {}
};

/** @brief Mixin that enables addition and substrcation operation without
 * checking for carry bit
 *
 * Note this mixin does not check overflow during calculation.
 *
 * If `Core` can hold twice as `MOD`, than replacing `ModAddSubMixin` with this
 * mixin might have a performance boost. Otherwise, the mixin could lead to
 * incorrect result.
 *
 * require `Core::is_zero`, `Core::set_zero` methods
 */
template <class Core, typename LIMB_T, size_t LIMB_N, const LIMB_T *MOD>
class ModAddSubMixinCarryFree
    : protected CRTP<Core, ModAddSubMixinCarryFree<Core, LIMB_T, LIMB_N, MOD>> {
    friend CRTP<Core, ModAddSubMixinCarryFree<Core, LIMB_T, LIMB_N, MOD>>;

  public:
    __host__ __device__ GEC_INLINE static const Core &mod() {
        return *reinterpret_cast<const Core *>(MOD);
    }

    /** @brief a = b + c (mod MOD)
     */
    static void add(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                    const Core &GEC_RSTRCT c) {
        utils::seq_add<LIMB_N>(a.array(), b.array(), c.array());
        if (utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
            utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a + c (mod MOD)
     */
    static void add(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        utils::seq_add<LIMB_N>(a.array(), b.array());
        if (utils::VtSeqCmp<LIMB_N, LIMB_T>::call(a.array(), MOD) !=
            utils::CmpEnum::Lt) {
            utils::seq_sub<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = - b (mod MOD)
     */
    static void neg(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        if (b.is_zero()) {
            a.set_zero();
        } else {
            utils::seq_sub<LIMB_N>(a.array(), MOD, b.array());
        }
    }

    /** @brief a = b - c (mod MOD)
     */
    static void sub(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                    const Core &GEC_RSTRCT c) {
        bool borrow = utils::seq_sub<LIMB_N>(a.array(), b.array(), c.array());
        if (borrow) {
            utils::seq_add<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a - c (mod MOD)
     */
    static void sub(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b) {
        bool borrow = utils::seq_sub<LIMB_N>(a.array(), b.array());
        if (borrow) {
            utils::seq_add<LIMB_N>(a.array(), MOD);
        }
    }

    /** @brief a = a * 2^K (mod MOD)
     */
    template <size_t K>
    __host__ __device__ static void mul_pow2(Core &GEC_RSTRCT a) {
        CarryFreeMulPow2Helper<Core, K, LIMB_T, LIMB_N, MOD>::call(a);
    }

    /** @brief a = 2 * a (mod MOD)
     */
    __host__ __device__ static void add_self(Core &GEC_RSTRCT a) {
        CarryFreeMulPow2Helper<Core, 1, LIMB_T, LIMB_N, MOD>::call(a);
    }
};

} // namespace bigint

} // namespace gec

#endif // !GEC_BIGINT_MIXIN_MOD_ADD_SUB_HPP
