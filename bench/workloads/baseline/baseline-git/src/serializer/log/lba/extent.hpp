#ifndef __SERIALIZER_LOG_LBA_EXTENT_HPP__
#define __SERIALIZER_LOG_LBA_EXTENT_HPP__

#include "serializer/log/extents/extent_manager.hpp"
#include "arch/arch.hpp"

struct extent_block_t;

class extent_t
{
    friend class extent_block_t;
    
public:
    extent_t(extent_manager_t *em, direct_file_t *file);   // Creates new extent
    extent_t(extent_manager_t *em, direct_file_t *file, off64_t loc, size_t size);   // Recreates extent at given offset (used during startup)
    
    void destroy();   // Releases extent and destroys structure in memory
    void shutdown();   // Only destroys structure in memory

public:
    struct read_callback_t : private iocallback_t {
        friend class extent_t;
        virtual void on_extent_read() = 0;
    private:
        void on_io_complete(event_t *) { on_extent_read(); }
    };
    void read(size_t pos, size_t length, void *buffer, read_callback_t *);
    
    void append(void *buffer, size_t length);
    
    struct sync_callback_t {
        virtual void on_extent_sync() = 0;
        virtual ~sync_callback_t() {}
    };
    void sync(sync_callback_t *cb);
    
    off64_t offset;
    size_t amount_filled;

private:
    ~extent_t();   // Use destroy() or shutdown() instead
    extent_manager_t *em;
    direct_file_t *file;
    extent_block_t *last_block, *current_block;
};

#endif /* __SERIALIZER_LOG_LBA_EXTENT_HPP__ */