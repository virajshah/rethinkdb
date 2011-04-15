#include "replication/backfill_receiver.hpp"

namespace replication {

void backfill_receiver_t::send(scoped_malloc<net_backfill_complete_t>& message) {
    cb->backfill_done(message->time_barrier_timestamp);
}

void backfill_receiver_t::send(scoped_malloc<net_get_cas_t>& msg) {
    store_key_t key(msg->key_size, msg->key);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_get_cas(key, castime);
}

void backfill_receiver_t::send(stream_pair<net_sarc_t>& msg) {
    store_key_t key(msg->key_size, msg->keyvalue);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_sarc(key, msg.stream, msg->flags, msg->exptime, castime,
        add_policy_t(msg->add_policy), replace_policy_t(msg->replace_policy),
        msg->old_cas);
}

void backfill_receiver_t::send(stream_pair<net_backfill_set_t>& msg) {
    backfill_atom_t atom;
    atom.key.assign(msg->key_size, msg->keyvalue);
    atom.value = msg.stream;
    atom.flags = msg->flags;
    atom.exptime = msg->exptime;
    atom.recency = msg->timestamp;
    atom.cas_or_zero = msg->cas_or_zero;
    cb->backfill_set(atom);
}

void backfill_receiver_t::send(scoped_malloc<net_incr_t>& msg) {
    store_key_t key(msg->key_size, msg->key);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_incr_decr(incr_decr_INCR, key, msg->amount, castime);
}

void backfill_receiver_t::send(scoped_malloc<net_decr_t>& msg) {
    store_key_t key(msg->key_size, msg->key);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_incr_decr(incr_decr_DECR, key, msg->amount, castime);
}

void backfill_receiver_t::send(stream_pair<net_append_t>& msg) {
    store_key_t key(msg->key_size, msg->keyvalue);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_append_prepend(append_prepend_APPEND, key, msg.stream, castime);
}

void backfill_receiver_t::send(stream_pair<net_prepend_t>& msg) {
    store_key_t key(msg->key_size, msg->keyvalue);
    castime_t castime(msg->proposed_cas, msg->timestamp);
    cb->realtime_append_prepend(append_prepend_PREPEND, key, msg.stream, castime);
}

void backfill_receiver_t::send(scoped_malloc<net_delete_t>& msg) {
    store_key_t key(msg->key_size, msg->key);
    cb->realtime_delete_key(key, msg->timestamp);
}

void backfill_receiver_t::send(scoped_malloc<net_backfill_delete_t>& msg) {
    store_key_t key(msg->key_size, msg->key);
    cb->backfill_deletion(key);
}

void backfill_receiver_t::send(scoped_malloc<net_nop_t>& message) {
    cb->realtime_time_barrier(message->timestamp);
}

}