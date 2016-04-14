#pragma once

#include <memory>

#include "game/entity.h"
#include "vector.h"

namespace bm {

// Abstract order interface, that actors will adhere to.
// Order can be an area order (i.e. dig these halls) a position order (i.e.
// dig here, move here), target order (kill target).
enum class OrderTargetType: uint16_t {
    Area,
    Position,
    Entity
};

enum class JobType: uint16_t {
    Mine,   // Remove cells (or one cell) preserving walls, ceiling and floor
};

class Order {
public:
    Order(OrderTargetType tt, JobType jt): target_type_(tt), job_(jt) {
    }
    using Ptr = std::shared_ptr<Order>;
protected:
    OrderTargetType target_type_;
    JobType job_;
};

class AreaOrder: public Order {
private:
    std::vector<Region> areas_;
public:
    AreaOrder(JobType jt)
        : Order(OrderTargetType::Area, jt) {
    }
};

class PositionOrder: public Order {
private:
    Vec3i pos_;
public:
    PositionOrder(const Vec3i &p, JobType jt)
        : Order(OrderTargetType::Position, jt), pos_(p) {
    }
};

class EntityOrder: public Order {
private:
    std::vector<EntityId> targets_;
public:
    EntityOrder(JobType jt)
        : Order(OrderTargetType::Entity, jt) {
    }
};

} // namespace bm
