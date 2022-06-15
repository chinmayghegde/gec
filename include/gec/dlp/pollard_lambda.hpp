#pragma once
#ifndef GEC_DLP_POLLARD_LAMBDA_HPP
#define GEC_DLP_POLLARD_LAMBDA_HPP

#include <gec/bigint/mixin/random.hpp>
#include <gec/utils/basic.hpp>
#include <gec/utils/misc.hpp>

#include <random>

#ifdef GEC_ENABLE_PTHREADS
#include <pthread.h>
#include <unordered_map>
#endif // GEC_ENABLE_PTHREADS
namespace gec {

namespace dlp {

/** @brief pollard lambda algorithm for ECDLP
 *
 * `a` must be strictly less than `b`, otherwise the behaviour is undefined.
 */
template <typename S, typename P, typename Rng, typename Ctx>
__host__ void pollard_lambda(S &GEC_RSTRCT x, S *GEC_RSTRCT sl,
                             P *GEC_RSTRCT pl, const S &GEC_RSTRCT bound,
                             const S &GEC_RSTRCT a, const S &GEC_RSTRCT b,
                             const P &GEC_RSTRCT g, const P &GEC_RSTRCT h,
                             GecRng<Rng> &rng, Ctx &GEC_RSTRCT ctx) {
    using F = typename P::Field;
    auto &ctx_view = ctx.template view_as<P, P, P, S, S, S>();
    auto &p1 = ctx_view.template get<0>();
    auto &p2 = ctx_view.template get<1>();
    auto &temp = ctx_view.template get<2>();
    auto &d = ctx_view.template get<3>();
    auto &idx = ctx_view.template get<4>();
    ctx_view.template get<5>().set_one();
    const auto &one = ctx_view.template get<5>();
    auto &rest_ctx = ctx_view.rest();

    while (true) {
        P *u = &p1, *v = &p2, *tmp = &temp;

        S::sub(x, b, a);
        // with `a` less than `b`, `m` would not underflow
        size_t m = x.most_significant_bit() - 1;
        for (size_t i = 0; i < m; ++i) {
            sl[i].array()[0] = typename S::LimbT(i);
        }
        for (size_t i = 0; i < m; ++i) {
            size_t ri = m - 1 - i;
            utils::swap(sl[ri].array()[0], sl[rng.sample(ri)].array()[0]);
        }
        for (size_t i = 0; i < m; ++i) {
            typename F::LimbT e = sl[i].array()[0];
            sl[i].set_pow2(e);
            P::mul(pl[i], sl[i], g, rest_ctx);
        }

        S::sample_inclusive(x, a, b, rng, rest_ctx);
        P::mul(*u, x, g, rest_ctx);
        for (idx.set_zero(); idx < bound; S::add(idx, one)) {
            size_t i = u->x().array()[0] % m;
            S::add(x, sl[i]);
            P::add(*tmp, *u, pl[i], rest_ctx);
            utils::swap(u, tmp);
        }

        d.set_zero();
        *v = h;
        for (idx.set_zero(); idx < bound; S::add(idx, one)) {
            if (P::eq(*u, *v)) {
                S::sub(x, d);
                return;
            }
            size_t idx = v->x().array()[0] % m;
            S::add(d, sl[idx]);
            P::add(*tmp, *v, pl[idx], rest_ctx);
            utils::swap(v, tmp);
        }
    }
}

#ifdef GEC_ENABLE_PTHREADS

namespace _pollard_lambda_ {

template <typename S, typename P>
struct SharedData {
    std::unordered_map<P, S, typename P::Hasher> traps;
    std::vector<S> sl;
    std::vector<P> pl;
    pthread_mutex_t x_lock;
    pthread_mutex_t traps_lock;
    pthread_barrier_t barrier;
    S &x;
    const S &a;
    const S &b;
    const P &g;
    const P &h;
    const S &bound;

    size_t worker_n;
    bool shutdown;

    SharedData(size_t worker_n, S &GEC_RSTRCT x, const S &GEC_RSTRCT a,
               const S &GEC_RSTRCT b, const P &GEC_RSTRCT g,
               const P &GEC_RSTRCT h, const S &GEC_RSTRCT bound, size_t m)
        : traps(worker_n), sl(m), pl(m), x_lock(PTHREAD_MUTEX_INITIALIZER),
          traps_lock(PTHREAD_MUTEX_INITIALIZER), barrier(), x(x), a(a), b(b),
          g(g), h(h), bound(bound), worker_n(worker_n), shutdown(false) {
        pthread_barrier_init(&barrier, nullptr, (unsigned int)(worker_n));
    }
};

template <typename S, typename P>
struct WorkerData {
    SharedData<S, P> &shared_data;
    size_t seed;
    size_t id;
};

template <typename S, typename P, typename Rng>
void *worker(void *data_ptr) {
    using Data = WorkerData<S, P>;
    using LimbT = typename P::Field::LimbT;

    Data &worker_data = *static_cast<Data *>(data_ptr);
    SharedData<S, P> &data = worker_data.shared_data;
    const size_t m = data.sl.size();
    P p1, p2;
    P *u = &p1, *tmp = &p2;
    S x, j, one(1);
    typename P::template Context<> ctx;
    auto rng = make_gec_rng(std::mt19937((unsigned int)(worker_data.seed)));

    while (true) {
        // calculate jump table
        if (worker_data.id == 0) {
            for (size_t i = 0; i < m; ++i) {
                data.sl[i].array()[0] = typename S::LimbT(i);
            }
            for (size_t i = 0; i < m; ++i) {
                size_t ri = m - 1 - i;
                utils::swap(data.sl[ri].array()[0],
                            data.sl[rng.sample(ri)].array()[0]);
            }
            for (size_t i = 0; i < m; ++i) {
                // TODO: maybe using multithread to generate the jump table?
                LimbT e = data.sl[i].array()[0];
                data.sl[i].set_pow2(e);
                P::mul(data.pl[i], data.sl[i], data.g, ctx);
            }
#ifdef GEC_DEBUG
            printf("[worker %03zu]: jump table generated\n", worker_data.id);
#endif // GEC_DEBUG
        }

        pthread_barrier_wait(&data.barrier);

        // setting traps
        S::sample_inclusive(x, data.a, data.b, rng, ctx);
        P::mul(*u, x, data.g, ctx);
        for (j.set_zero(); j < data.bound; S::add(j, one)) {
            size_t i = u->x().array()[0] % m;
            S::add(x, data.sl[i]);
            P::add(*tmp, *u, data.pl[i], ctx);
            utils::swap(u, tmp);
#ifdef GEC_DEBUG
            if (!(utils::LowerKMask<LimbT, 20>::value & j.array()[0])) {
                printf("[worker %03zu]: calculating trap, step ",
                       worker_data.id);
                j.println();
            }
#endif // GEC_DEBUG
        }
        pthread_mutex_lock(&data.traps_lock);
        data.traps.insert(std::make_pair(*u, x));
        pthread_mutex_unlock(&data.traps_lock);

#ifdef GEC_DEBUG
        printf("[worker %03zu]: trap set\n", worker_data.id);
#endif // GEC_DEBUG
        pthread_barrier_wait(&data.barrier);

        // start searching
        S::sample_inclusive(x, data.a, data.b, rng, ctx);
        P::mul(*tmp, x, data.g, ctx);
        P::add(*u, data.h, *tmp, ctx);
        for (j.set_zero(); j < data.bound; S::add(j, one)) {
            if (data.shutdown) {
                break;
            }
            auto it = data.traps.find(*u);
            if (it != data.traps.end() && it->second != x) {
                pthread_mutex_lock(&data.x_lock);
                if (!data.shutdown) {
                    S::sub(data.x, it->second, x);
                    data.shutdown = true;
                }
                pthread_mutex_unlock(&data.x_lock);
                break;
            }
            size_t i = u->x().array()[0] % m;
            S::add(x, data.sl[i]);
            P::add(*tmp, *u, data.pl[i], ctx);
            utils::swap(u, tmp);
#ifdef GEC_DEBUG
            if (!(utils::LowerKMask<LimbT, 20>::value & j.array()[0])) {
                printf("[worker %03zu]: searching, step ", worker_data.id);
                j.println();
            }
#endif // GEC_DEBUG
        }

        pthread_barrier_wait(&data.barrier);
        // check success

        if (data.shutdown) {
#ifdef GEC_DEBUG
            printf("[worker %03zu]: collision found, shutting down\n",
                   worker_data.id);
#endif // GEC_DEBUG
            return nullptr;
        }
#ifdef GEC_DEBUG
        printf("[worker %03zu]: collision not found, retry\n", worker_data.id);
#endif // GEC_DEBUG
    }
}

/** @brief multithread pollard lambda algorithm for ECDLP, requires `pthreads`
 *
 * `a` must be strictly less than `b`, otherwise the behaviour is undefined.
 */
template <typename S, typename P, typename Rng>
void multithread_pollard_lambda(S &GEC_RSTRCT x, const S &GEC_RSTRCT bound,
                                unsigned int worker_n, const S &GEC_RSTRCT a,
                                const S &GEC_RSTRCT b, const P &GEC_RSTRCT g,
                                const P &GEC_RSTRCT h, GecRng<Rng> &rng) {
    using Shared = SharedData<S, P>;
    using Data = WorkerData<S, P>;

    std::vector<pthread_t> workers(worker_n);

    typename P::template Context<> ctx;

    S::sub(x, b, a);
    // with `a` less than `b`, `m` would not underflow
    size_t m = x.most_significant_bit() - 1;

    Shared shared(worker_n, x, a, b, g, h, bound, m);

    std::vector<Data> params(worker_n, Data{shared, 0, 0});

    for (size_t i = 0; i < worker_n; ++i) {
        auto &param = params[i];
        param.seed = rng.template sample<size_t>();
        param.id = i;
        pthread_create(&workers[i], nullptr, worker<S, P, Rng>,
                       static_cast<void *>(&param));
    }

    for (size_t i = 0; i < worker_n; ++i) {
        pthread_join(workers[i], nullptr);
    }
}

} // namespace _pollard_lambda_

// NOLINTNEXTLINE(misc-unused-using-decls)
using _pollard_lambda_::multithread_pollard_lambda;

#endif // GEC_ENABLE_PTHREADS

#ifdef GEC_ENABLE_CUDA

#endif // GEC_ENABLE_CUDA

} // namespace dlp

} // namespace gec

#endif // GEC_DLP_POLLARD_LAMBDA_HPP