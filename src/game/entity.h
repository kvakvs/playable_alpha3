#pragma once

#include "vector.h"
#include <stdint.h>

namespace bm {

using EntityId = uint64_t;

// Entity is anything which has a position in the world.
// This includes both animate and inanimate objects (dropped resources,
// dead bodies etc)
class IEntity {
public:
    // Each entity has a position in 3d world (cell)
    virtual Vec3i get_pos() const = 0;
    virtual void set_pos(const Vec3i &v) = 0;
    // Each entity has ID so that we don't have to use pointers everywhere
    virtual EntityId get_id() const = 0;
    virtual void set_id(EntityId id) = 0;
};

class Entity: public IEntity {
    Vec3i pos_;
    EntityId id_;
public:
    virtual Vec3i get_pos() const override { return pos_; }
    virtual void set_pos(const Vec3i &v) const override { pos_ = v; }
    virtual EntityId get_id() const override { return id_; }
    virtual void set_id(EntityId id) const override { id_ = id; }
};

} // namespace bm