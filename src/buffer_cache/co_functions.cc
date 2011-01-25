#include "buffer_cache/co_functions.hpp"

struct co_block_available_callback_t : public block_available_callback_t {
    coro_t *self;
    buf_t *value;

    virtual void on_block_available(buf_t *block) {
        value = block;
        self->notify();
    }

    buf_t *join() {
        self = coro_t::self();
        coro_t::wait();
        return value;
    }
};

buf_t *co_acquire_block(transaction_t *transaction, block_id_t block_id, access_t mode) {
    co_block_available_callback_t cb;
    buf_t *value = transaction->acquire(block_id, mode, &cb);
    if (!value) {
        value = cb.join();
    }
    rassert(value);
    return value;
}


struct large_value_acquired_t : public large_buf_available_callback_t {
    coro_t *self;
    large_value_acquired_t() : self(coro_t::self()) { }
    void on_large_buf_available(large_buf_t *large_value) { self->notify(); }
};

void co_acquire_large_value(large_buf_t *large_value, large_buf_ref root_ref_, access_t access_) {
    large_value_acquired_t acquired;
    // TODO: what if this calls acquired immediately?  Then
    // self->notify() gets called before coro_t::wait().

    // One way to fix this possible bug without learning about
    // coroutines: make large_value->acquire return bool, the usual
    // way, and then do what co_commit does.
    large_value->acquire(root_ref_, access_, &acquired);
    coro_t::wait();
}

void co_acquire_large_value_lhs(large_buf_t *large_value, large_buf_ref root_ref_, access_t access_) {
    large_value_acquired_t acquired;
    large_value->acquire_lhs(root_ref_, access_, &acquired);
    coro_t::wait();
}

void co_acquire_large_value_rhs(large_buf_t *large_value, large_buf_ref root_ref_, access_t access_) {
    large_value_acquired_t acquired;
    large_value->acquire_rhs(root_ref_, access_, &acquired);
    coro_t::wait();
}


// Well this is repetitive.
struct transaction_begun_callback_t : public transaction_begin_callback_t {
    coro_t *self;
    transaction_t *value;

    void on_txn_begin(transaction_t *txn) {
        value = txn;
        self->notify();
    }

    transaction_t *join() {
        self = coro_t::self();
        coro_t::wait();
        return value;
    }
};

transaction_t *co_begin_transaction(cache_t *cache, access_t access) {
    transaction_begun_callback_t cb;
    transaction_t *value = cache->begin_transaction(access, &cb);
    if (!value) {
        value = cb.join();
    }
    rassert(value);
    return value;
}



struct transaction_committed_t : public transaction_commit_callback_t {
    coro_t *self;
    transaction_committed_t() : self(coro_t::self()) { }
    void on_txn_commit(transaction_t *transaction) {
        self->notify();
    }
};


void co_commit_transaction(transaction_t *transaction) {
    transaction_committed_t cb;
    if (!transaction->commit(&cb)) {
        coro_t::wait();
    }
}