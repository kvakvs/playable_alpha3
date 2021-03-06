#pragma once

//#include <set>
#include <random>
#include <point_multimap.hpp>
#include "region_iterator.hpp"

#include "game/co_entity.h"
#include "game/order.h"
#include "ai/goal.h"
#include "util/optional.h"
#include "world_volume.h"
#include "game/g_defs.h"
#include "game/inanimate.h"

namespace bm {

//using AnimateObjectMap = std::map<EntityId, AnimateObject *>;
using SpatialAnimateMap = spatial::point_multimap<3, Array3i, AnimateObject *>;

class World {
public:
    World(bm::RawVolume& vol);

    //
    // Animate objects
    //
    void add_animate_object(AnimateObject *ent);
    void animate_position_changed(AnimateObject *a,
                                  const Vec3i& old,
                                  const Vec3i& updated);
    // deferred variant that keeps track of old coords and changed objects but
    // does not modify animate_ yet
    void animate_position_changed_d(AnimateObject *a,
                                    const Vec3i& old);
    const SpatialAnimateMap& get_animate_objects() const {
        return objects_.animate_;
    }

    //
    // Inanimate objects
    //
    void spawn_inanimate_object(const Vec3i& pos, InanimateType ot);
    const SpatialInanimateMap& get_inanimate_objects() const {
        return objects_.inanimate_;
    }

    //
    // Loops
    //
//    using EachInanimateFn = void (*)(InanimateObject& ia, const Vec3i& pos);
//    using EachAnimateFn = void (*)(AnimateObject* ao, const Vec3i& pos);

    template <typename EachAnimateFn>
    void for_each_animate(EachAnimateFn fn) {
        for (auto pair: objects_.animate_) {
            fn(pair.second, util::make_vec3i(pair.first));
        }
    }

    template <typename EachInanimateFn>
    void for_each_inanimate(EachInanimateFn fn) {
        for (auto pair: objects_.inanimate_) {
            fn(pair.second, util::make_vec3i(pair.first));
        }
    }

    template <typename EachAnimateFn>
    void for_each_animate_r(const Vec3i& from, const Vec3i& to,
                            EachAnimateFn fn)
    {
        Array3i p0 = util::make_array(from);
        Array3i p1 = util::make_array(to);

        auto iter = spatial::region_cbegin(objects_.animate_, p0, p1);
        auto iter_end = spatial::region_cend(objects_.animate_, p0, p1);
        for (; iter != iter_end; ++iter) {
            auto ao  = iter->second;
            auto pos = util::make_vec3i(iter->first);
            fn(ao, pos);
        }
    }

    template <typename EachAnimateFn>
    void for_each_animate_r(const Region& reg, EachAnimateFn fn)
    {
        for_each_animate_r(reg.getLowerCorner(), reg.getUpperCorner(), fn);
    }

    template <typename EachInanimateFn>
    void for_each_inanimate_r(const Vec3i& from, const Vec3i& to,
                            EachInanimateFn fn)
    {
        Array3i p0 = util::make_array(from);
        Array3i p1 = util::make_array(to);

        auto iter = spatial::region_cbegin(objects_.inanimate_, p0, p1);
        auto iter_end = spatial::region_cend(objects_.inanimate_, p0, p1);
        for (; iter != iter_end; ++iter) {
            auto o  = iter->second;
            auto pos = util::make_vec3i(iter->first);
            fn(o, pos);
        }
    }

    template <typename EachInanimateFn>
    void for_each_inanimate_r(const Region& reg, EachInanimateFn fn)
    {
        for_each_inanimate_r(reg.getLowerCorner(), reg.getUpperCorner(), fn);
    }

    void think();

    VoxelType get_under(const Vec3i &pos) const {
        return volume_.getVoxel(pos + Vec3i(0, 1, 0));
    }

    bool is_mineable(const Vec3i &pos) const;

    bm::VolumeType* get_volume() { return &volume_; }
    const bm::VolumeType* get_volume() const { return &volume_; }

    void mine_voxel(const Vec3i &pos);
    VoxelType get_voxel(int x, int y, int z) const {
        return volume_.getVoxel(x, y, z);
    }
    VoxelType get_voxel(const Vec3i& pos) const {
        return volume_.getVoxel(pos);
    }
    void set_voxel(const Vec3i& pos, VoxelType v) {
        volume_.setVoxel(pos, v);
    }

    //
    // Orders, planning and AI
    //

    // Check if any orders are available
    bool have_orders() const;
    bool add_order(ai::Order::Ptr desired);
    void remove_order(ai::OrderId id);
    // Get a random order. See if it is not completed.
    ai::Order::Ptr get_random_order(AnimateObject *actor);

    // Give positional or area mining goal (fun add_fn is called for each
    // position in selected area, or for single position if no area selection
    void add_goal(const Optional<Vec3i>& mark_begin,
                  const Vec3i& pos,
                  std::function<void(const Vec3i&)> add_fn);
    void add_goal_dig(const Vec3i& pos);
    void add_goal_ramp(const Vec3i& pos);

    // Report from actors if order is completed
    void report_fulfilled(ai::OrderId id, PlanResult pr);
    // Report from actors if order is failed
    void report_failed(ai::OrderId id, PlanResult pr);
    void report_impossible(ai::OrderId id, PlanResult pr);

    // Rendering helpers allow to view some data
    const ai::OrderMap& get_orders() const { return orders_; }
    const ai::OrderMap& get_orders_low() const { return orders_low_; }
    const ai::OrderMap& get_orders_verylow() const { return orders_verylow_; }

    //
    // Sensors and metrics system
    //
    // For every metric, read current situation and compare
    bool conditions_stand_true(const ai::MetricVec& cond,
                               const ai::Context& ctx) const;
    // For every condition type in desired, reads current situation
    ai::MetricVec get_current_situation(const ai::MetricVec& desired,
                                        const ai::Context& ctx) const;
    ai::Metric read_metric(const ai::Metric& metric,
                           const ai::Context& ctx) const;

public:
    bool force_update_terrain_mesh_ = false;

private:
    std::mt19937 rand_;
    std::uniform_int_distribution<int> d100_ {0, 99};

    uint64_t ent_id_ = 0;

    struct {
        SpatialAnimateMap animate_;
        // Moved objects after logic run - we update spatial map
        std::vector<std::pair<AnimateObject*, Vec3i>> animate_moved_;

        SpatialInanimateMap inanimate_;
    } objects_;

    // Visible piece of world + some nearby
    bm::VolumeType& volume_;

    // TODO: Move desires that failed too many times to low priority list

    // What player desires (goals of sort, without preconditions) - will
    // propagate to workers and they will see how to fulfill master's wish.
    ai::OrderMap orders_;
    ai::OrderMap orders_low_; // lower prio
    ai::OrderMap orders_verylow_; // even lower prio
    constexpr static uint64_t LOW_PRIO_ORDERS_EVERY = 3;
    constexpr static uint64_t VERY_LOW_PRIO_ORDERS_EVERY = 6;

    // Simulation step (equivalent of time)
    uint64_t sim_step_ = 0;

private:
    ai::Order::Ptr get_random_order(AnimateObject *actor,
                                    ai::OrderMap& registry);
    void lower_prio(ai::OrderId id);
    void run_animate_entities();
    void run_animate_brains();
};

} // ns bm
