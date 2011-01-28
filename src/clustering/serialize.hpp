
#ifndef __CLUSTERING_SERIALIZE_HPP__
#define __CLUSTERING_SERIALIZE_HPP__

#include "arch/arch.hpp"

/* Serializing and unserializing built-in integer types */

void serialize(tcp_conn_t *conn, bool b) {
    conn->write(&b, sizeof(b));
}

void unserialize(tcp_conn_t *conn, bool *b) {
    conn->read(b, sizeof(*b));
}

void serialize(tcp_conn_t *conn, int i) {
    conn->write(&i, sizeof(i));
}

void unserialize(tcp_conn_t *conn, int *i) {
    conn->read(i, sizeof(*i));
}

void serialize(tcp_conn_t *conn, char c) {
    conn->write(&c, 1);
}

void unserialize(tcp_conn_t *conn, char *c) {
    conn->read(c, 1);
}

/* Serializing and unserializing std::string */

void serialize(tcp_conn_t *conn, const std::string &s) {
    int count = s.size();
    conn->write(&count, sizeof(int));
    conn->write(s.data(), count);
}

void unserialize(tcp_conn_t *conn, std::string *s) {
    int count;
    conn->read(&count, sizeof(int));
    char buffer[count];   // Makes an extra copy... Can we fix that?
    conn->read(buffer, count);
    s->assign(buffer, count);
}

/* Serializing and unserializing std::vector of a serializable type */

template<class element_t>
void serialize(tcp_conn_t *conn, const std::vector<element_t> &e) {
    serialize(conn, e.size());
    for (int i = 0; i < e.size(); i++) {
        serialize(conn, e[i]);
    }
}

template<class element_t>
void unserialize(tcp_conn_t *conn, std::vector<element_t> *e) {
    int count;
    unserialize(conn, &count);
    e->resize(count);
    for (int i = 0; i < count; i++) {
        unserialize(conn, &e[i]);
    }
}

/* Serializing and unserializing ip_address_t */

void serialize(tcp_conn_t *conn, const ip_address_t &addr) {
    rassert(sizeof(addr) == 4);
    conn->write(&addr, sizeof(addr));
}

void unserialize(tcp_conn_t *conn, ip_address_t *addr) {
    rassert(sizeof(*addr) == 4);
    conn->read(addr, sizeof(*addr));
}

/* Serializing and unserializing keys */

void serialize(tcp_conn_t *conn, const store_key_and_buffer_t &key) {
    conn->write(&key.key.size, sizeof(key.key.size));
    conn->write(key.key.contents, key.key.size);
}

void unserialize(tcp_conn_t *conn, store_key_and_buffer_t *key) {
    conn->read(&key->key.size, sizeof(key->key.size));
    conn->read(key->key.contents, key->key.size);
}

/* Serializing and unserializing data_provider_ts */

void serialize(tcp_conn_t *conn, boost::shared_ptr<data_provider_t> data) {
    int size = data->get_size();
    conn->write(&size, sizeof(size));
    const const_buffer_group_t *buffers = data->get_data_as_buffers();
    for (int i = 0; i < (int)buffers->buffers.size(); i++) {
        conn->write(buffers->buffers[i].data, buffers->buffers[i].size);
    }
}

void unserialize(tcp_conn_t *conn, boost::shared_ptr<data_provider_t> *data) {
    int size;
    conn->read(&size, sizeof(size));
    void *buffer;
    *data.reset(new buffered_data_provider_t(size, &buffer));
    conn->read(buffer, size);
}

#endif /* __CLUSTERING_SERIALIZE_HPP__ */