#pragma once
#ifndef GEC_CURVE_MIXIN_JACOBAIN_HPP
#define GEC_CURVE_MIXIN_JACOBAIN_HPP

#include <gec/utils/context_check.hpp>
#include <gec/utils/crtp.hpp>

namespace gec {

namespace curve {

/** @brief mixin that enables elliptic curve arithmetic with Jacobian coordinate
 */
template <typename Core, typename FIELD_T, const FIELD_T &A, const FIELD_T &B>
class Jacobain : protected CRTP<Core, Jacobain<Core, FIELD_T, A, B>> {
    friend CRTP<Core, Jacobain<Core, FIELD_T, A, B>>;

  public:
    /** @brief add distinct point with some precomputed value
     *
     * ctx.get<0>() == a == x1 z2^2
     * ctx.get<1>() == b == x2 z1^2
     * ctx.get<2>() == c == y1 z2^3
     * ctx.get<3>() == d == y2 z1^3
     */
    template <typename F_CTX>
    __host__ __device__ static void
    add_distinct_inner(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                       const Core &GEC_RSTRCT c, F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 4);

        FIELD_T &t1 = ctx.template get<0>();
        FIELD_T &t2 = ctx.template get<1>();
        FIELD_T &t3 = ctx.template get<2>();
        FIELD_T &t4 = ctx.template get<3>();

        FIELD_T::sub(t2, t1);           // e = b - a
        FIELD_T::sub(t4, t3);           // f = d - c
        FIELD_T::mul(a.z(), t2, t2);    // e^2
        FIELD_T::mul(a.y(), t1, a.z()); // a e^2
        FIELD_T::mul(t1, a.z(), t2);    // e^3
        FIELD_T::mul(a.z(), t3, t1);    // c e^3
        FIELD_T::add(t3, a.y(), a.y()); // 2 a e^2
        FIELD_T::mul(a.x(), t4, t4);    // f^2
        FIELD_T::sub(a.x(), t3);        // f^2 - 2 a e^2
        FIELD_T::sub(a.x(), t1);        // x = f^2 - 2 a e^2 - e^3
        FIELD_T::sub(t1, a.y(), a.x()); // a e^2 - x
        FIELD_T::mul(a.y(), t4, t1);    // f (a e^2 - x)
        FIELD_T::sub(a.y(), a.z());     // y = f (a e^2 - x) - c e^3
        FIELD_T::mul(t1, b.z(), c.z()); // z1 z2
        FIELD_T::mul(a.z(), t1, t2);    // z = z1 z2 e
    }

    __host__ __device__ GEC_INLINE bool is_inf() const {
        return this->core().z().is_zero();
    }
    __host__ __device__ GEC_INLINE void set_inf() {
        this->core().x().set_zero();
        this->core().y().set_zero();
        this->core().z().set_zero();
    }

    template <typename F_CTX>
    __host__ __device__ static bool on_curve(const Core &GEC_RSTRCT a,
                                             F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 4);

        FIELD_T &l = ctx.template get<0>();
        FIELD_T &r = ctx.template get<1>();
        FIELD_T &t1 = ctx.template get<2>();
        FIELD_T &t2 = ctx.template get<3>();

        FIELD_T::mul(t1, a.z(), a.z()); // z^2
        FIELD_T::mul(t2, t1, t1);       // z^4
        FIELD_T::mul(r, t1, t2);        // z^6
        FIELD_T::mul(l, a.x(), t2);     // x z^4
        FIELD_T::mul(t2, A, l);         // a x z^4
        FIELD_T::mul(t1, B, r);         // b z^6
        FIELD_T::mul(l, a.x(), a.x());  // x^2
        FIELD_T::mul(r, l, a.x());      // x^3
        FIELD_T::add(r, t2);            // x^3 + a x z^4
        FIELD_T::add(r, t1);            // right = x^3 + a x z^4 + b z^6
        FIELD_T::mul(l, a.y(), a.y());  // left = y^2
        return l == r;
    }

    template <typename F_CTX>
    __host__ __device__ static void to_affine(Core &GEC_RSTRCT a,
                                              F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 2);

        if (a.is_inf() || a.z().is_mul_id())
            return;

        FIELD_T &t1 = ctx.template get<0>();
        FIELD_T &t2 = ctx.template get<1>();

        FIELD_T::inv(a.z(), ctx);       // z^-1
        FIELD_T::mul(t1, a.z(), a.z()); // z^-2
        FIELD_T::mul(t2, a.x(), t1);    // x z^-2
        a.x() = t2;                     //
        FIELD_T::mul(t2, t1, a.z());    // z^-3
        FIELD_T::mul(t1, a.y(), t2);    // y z^-3
        a.y() = t1;                     //
        // we don't assign z = 1 here, so `to_affine` and `from_affine` should
        // be paired
    }

    __host__ __device__ static void from_affine(Core &GEC_RSTRCT a) {
        a.z().set_mul_id();
    }

    template <typename F_CTX>
    __host__ __device__ static bool eq(const Core &GEC_RSTRCT a,
                                       const Core &GEC_RSTRCT b,
                                       F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 4);

        bool a_inf = a.is_inf();
        bool b_inf = b.is_inf();
        if (a_inf && b_inf) { // both infinity
            return true;
        } else if (a_inf || b_inf) { // only one infinity
            return false;
        } else if (a.z() == b.z()) { // z1 == z2
            return a.x() == b.x() && a.y() == b.y();
        } else { // z1 != z2
            FIELD_T &ta = ctx.template get<0>();
            FIELD_T &tb = ctx.template get<1>();
            FIELD_T &tc = ctx.template get<2>();
            FIELD_T &td = ctx.template get<3>();

            FIELD_T::mul(tc, a.z(), a.z()); // z1^2
            FIELD_T::mul(td, b.z(), b.z()); // z2^2
            FIELD_T::mul(ta, a.x(), td);    // x1 z2^2
            FIELD_T::mul(tb, b.x(), tc);    // x2 z1^2
            // check x1 z2^2 == x2 z1^2
            if (ta != tb) {
                return false;
            }
            FIELD_T::mul(ta, tc, a.z()); // z1^3
            FIELD_T::mul(tb, td, b.z()); // z2^3
            FIELD_T::mul(tc, a.y(), tb); // y1 z2^3
            FIELD_T::mul(td, b.y(), ta); // y2 z1^3
            // check y1 z2^3 == y2 z1^3
            if (tc != td) {
                return false;
            }
            return true;
        }
    }

    template <typename F_CTX>
    __host__ __device__ static void
    add_distinct(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b,
                 const Core &GEC_RSTRCT c, F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 5);

        FIELD_T &ta = ctx.template get<0>();
        FIELD_T &tb = ctx.template get<1>();
        FIELD_T &tc = ctx.template get<2>();
        FIELD_T &td = ctx.template get<3>();
        FIELD_T &t = ctx.template get<4>();

        FIELD_T::mul(tc, c.z(), c.z()); // z2^2
        FIELD_T::mul(t, tc, c.z());     // z2^3
        FIELD_T::mul(ta, tc, b.x());    // a = x1 z2^2
        FIELD_T::mul(tc, t, b.y());     // c = y1 z2^3

        FIELD_T::mul(td, b.z(), b.z()); // z1^2
        FIELD_T::mul(t, td, b.z());     // z1^3
        FIELD_T::mul(tb, td, c.x());    // b = x2 z1^2
        FIELD_T::mul(td, t, c.y());     // d = y2 z1^3

        add_distinct_inner(a, b, c, ctx);
    }

    template <typename F_CTX>
    __host__ __device__ static void add_self(Core &GEC_RSTRCT a,
                                             const Core &GEC_RSTRCT b,
                                             F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 2);

        FIELD_T &t4 = ctx.template get<0>();
        FIELD_T &t5 = ctx.template get<1>();

        FIELD_T::mul(t5, b.z(), b.z());       // z1^2
        FIELD_T::mul(t4, t5, t5);             // z1^4
        FIELD_T::mul(t5, A, t4);              // A z1^4
        FIELD_T::mul(t4, b.x(), b.x());       // x1^2
        FIELD_T::add(t5, t4);                 // x1^2 + A z1^4
        FIELD_T::add(t5, t4);                 // 2 x1^2 + A z1^4
        FIELD_T::add(t5, t4);                 // b = 3 x1^2 + A z1^4
        FIELD_T::mul(a.z(), b.y(), b.y());    // y1^2
        FIELD_T::mul(t4, b.x(), a.z());       // x1 y1^2
        FIELD_T::template mul_pow2<2>(t4);    // a = 4 x1 y1^2
        FIELD_T::add(a.y(), t4, t4);          // 2 a
        FIELD_T::mul(a.x(), t5, t5);          // b^2
        FIELD_T::sub(a.x(), a.y());           // x = b^2 - 2 a
        FIELD_T::sub(t4, a.x());              // a - x
        FIELD_T::mul(a.y(), t5, t4);          // b (a - x)
        FIELD_T::mul(t4, a.z(), a.z());       // y1^4
        FIELD_T::template mul_pow2<3>(t4);    // 8 y1^4
        FIELD_T::sub(a.y(), t4);              // y = b(a - x) -8 y1^4
        FIELD_T::mul(a.z(), b.y(), b.z());    // y1 z1
        FIELD_T::template mul_pow2<1>(a.z()); // z = 2 y1 z1
    }

    template <typename F_CTX>
    __host__ __device__ static void
    add(Core &GEC_RSTRCT a, const Core &GEC_RSTRCT b, const Core &GEC_RSTRCT c,
        F_CTX &GEC_RSTRCT ctx) {
        GEC_CTX_CAP(F_CTX, 5);

        if (b.is_inf()) {
            a = c;
        } else if (c.is_inf()) {
            a = b;
        } else {
            FIELD_T &ta = ctx.template get<0>();
            FIELD_T &tb = ctx.template get<1>();
            FIELD_T &tc = ctx.template get<2>();
            FIELD_T &td = ctx.template get<3>();
            FIELD_T &t = ctx.template get<4>();

            FIELD_T::mul(tc, c.z(), c.z()); // z2^2
            FIELD_T::mul(t, tc, c.z());     // z2^3
            FIELD_T::mul(ta, tc, b.x());    // a = x1 z2^2
            FIELD_T::mul(tc, t, b.y());     // c = y1 z2^3

            FIELD_T::mul(td, b.z(), b.z()); // z1^2
            FIELD_T::mul(t, td, b.z());     // z1^3
            FIELD_T::mul(tb, td, c.x());    // b = x2 z1^2
            FIELD_T::mul(td, t, c.y());     // d = y2 z1^3

            if (ta == tb && tc == td) {
                add_self(a, b, ctx);
            } else {
                add_distinct_inner(a, b, c, ctx);
            }
        }
    }

    __host__ __device__ GEC_INLINE static void neg(Core &GEC_RSTRCT a,
                                                   const Core &GEC_RSTRCT b) {
        a.x() = b.y();
        FIELD_T::neg(a.y(), b.y());
        a.z() = b.z();
    }
};

} // namespace curve

} // namespace gec

#endif // !GEC_CURVE_MIXIN_JACOBAIN_HPP