/**
 * -----------------------------------------------------------------------------
 *
 * Entity-Component-System (ECS) Implementation
 * https://github.com/kensuke-kamata/ecs
 *
 * -----------------------------------------------------------------------------
 *
 * This header file contains an implementation of an Entity-Component-System (ECS)
 * designed to be efficient, modular, and easy to use for game development and similar
 * high-performance applications.
 *
 * The ECS is split into several namespaces and classes:
 *
 * ecs:: The main namespace that includes important constants.
 *
 * ecs::Entity:: A namespace that includes types and functions related to entities,
 * including their creation, manipulation, and validation.
 *
 * ecs::Component:: A namespace that includes types and functions related to
 * components, including their creation and identification.
 *
 * ecs::ComponentPool:: A class that manages a pool of components of a particular type.
 * It allows adding and removing components for entities.
 *
 * ecs::Scene:: The main class that represents a scene in the ECS. A scene consists
 * of a number of entities, each of which can have any number of components.
 *
 * ecs::SceneView:: A class that allows iterating over entities in a scene that
 * have specific component types.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <vector>

//=========================
// ECS
//=========================
namespace ecs {

const uint64_t MAX_COMPONENTS(64);
const uint64_t MAX_ENTITIES(1000000);

// A mask used for representing and identifying components
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

//=========================
// Entity
//=========================
namespace Entity {

typedef uint64_t Id;
typedef uint32_t Index;
typedef uint32_t Version;

/**
 * Create a new entity ID from an index and version.
 * The index and version are packed into a 64b bit ID (32 bits each).
 */
static constexpr Id NewId(Index index, Version version) {
    return static_cast<Id>(index) << 32 | static_cast<Id>(version);
}

/**
 * Extract the index from the entity ID. The index is stored in the upper 32 bits.
 */
static constexpr Index GetIndex(Id id) {
    return id >> 32;
}

/**
 * Extract the version from the entity ID. The version is stored in the lower 32 bits.
 */
static constexpr Version GetVersion(Id id) {
    return static_cast<Version>(id);
}

/**
 * Validate the entity ID. An entity ID is invalid if the part is the maximum
 * value of uint32_t, which means removed.
 */
static constexpr bool IsValid(Id id) {
    return (id >> 32) != static_cast<Index>(-1);
}
}  // Entity

//=========================
// Component
//=========================
namespace Component {

typedef uint64_t Id;

/**
 * Create a new component ID. This function is thread-safe.
 */
inline static Id NewId() {
    static std::atomic<Id> counter(0);
    return counter++;
}

/**
 * Get the component ID for a specific component type. The component ID is a
 * static local variable that is initialized only once during the first call
 * for each component type.
 */
template<class T>
inline static Id GetId() {
    static Id id = NewId();
    return id;
}
}  // Component

//=========================
// ComponentPool
//=========================
class ComponentPool {
public:
    /**
     * Allocates enough memory to hold the maximum number of components of the
     * specified size.
     */
    ComponentPool(uint64_t size) : size_(size) {
        head_ = new char[size * MAX_ENTITIES];
    }

    /**
     * Releases the memory allocated for components.
     */
    ~ComponentPool() {
        delete[] head_;
    }

    /**
     * Get a pointer to the component at the specified index in the pool.
     */
    inline void *Get(Entity::Index index) const {
        return (head_ + index * size_);
    }

private:
    uint64_t size_{ 0 };
    char    *head_{ nullptr };
};

//=========================
// Scene
//=========================
class Scene {
public:
    /**
     * EntityPack holds an entity ID and a component mask that represents which
     * components the entity has.
     */
    struct EntityPack {
        Entity::Id      id_{ static_cast<Entity::Id>(-1) };
        ComponentMask mask_{ };
    };

    /**
     * Create a new entity. The entity is either created by resuing an index
     * from the freelist or by creating a new ID.
     */
    Entity::Id NewEntity() {
        if (!freelist_.empty()) {
            auto  i = freelist_.back(); freelist_.pop_back();
            auto id = Entity::NewId(i, Entity::GetVersion(entities_[i].id_));
            entities_[i].id_ = id;
            return id;
        }
        auto id = Entity::NewId(Entity::Index(entities_.size()), 0);
        entities_.emplace_back(EntityPack{ id, ComponentMask() });
        return id;
    }

    /**
     * Remove an entity. The entity's ID is invalidated and added to the
     * freelist, and all its components are removed.
     */
    void RemoveEntity(Entity::Id id) {
        auto i = Entity::GetIndex(id);
        entities_[i].id_ = Entity::NewId(
            Entity::Index(-1),
            Entity::GetVersion(id) + 1
        );
        entities_[i].mask_.reset();
        freelist_.push_back(i);
    }

    /**
     * Check if an entity has a component of a specific type.
     */
    template<class T>
    bool HasComponent(Entity::Id id) {
        auto   i = Entity::GetIndex(id);
        auto cid = Component::GetId<T>();
        return entities_[i].mask_.test(cid);
    }

    /**
     * Add a component to an entity. The component is created in the pool
     * corresponding to the component type.
     */
    template<class T, typename... Args>
    T *AddComponent(Entity::Id id, Args&&... args) {
        auto   i = Entity::GetIndex(id);
        auto cid = Component::GetId<T>();
        if (pools_.size() <= cid) {
            pools_.resize(cid + MAX_COMPONENTS / 4);
        }
        if (pools_[cid] == nullptr) {
            pools_[cid] = std::make_unique<ComponentPool>(sizeof(T));
        }
        auto component = new (pools_[cid]->Get(i)) T(std::forward<Args>(args)...);
        entities_[i].mask_.set(cid);
        return component;
    }

    /**
     * Remove a component from an entity.
     */
    template<class T>
    void RemoveComponent(Entity::Id id) {
        auto   i = Entity::GetIndex(id);
        auto cid = Component::GetId<T>();
        if (!HasComponent<T>(id)) {
            throw;
        }
        if (entities_[i].id_ != id) {
            throw;
        }
        entities_[i].mask_.reset(cid);
    }

    /**
     * Get a component from an entity.
     */
    template<class T>
    T *GetComponent(Entity::Id id) {
        auto   i = Entity::GetIndex(id);
        auto cid = Component::GetId<T>();
        if (!HasComponent<T>(id)) {
            throw;
        }
        if (entities_[i].id_ != id) {
            throw;
        }
        return static_cast<T *>(pools_[cid]->Get(i));
    }

    /**
     * Get a const reference to the vector of entities.
     */
    const std::vector<EntityPack> &GetEntities() const {
        return entities_;
    }

private:
    std::vector<EntityPack>    entities_{};
    std::vector<Entity::Index> freelist_{};
    std::vector<std::unique_ptr<ComponentPool>> pools_{};
};

/**
 * SceneView allows to iterate over entities in a scene that have specific
 * component types.
 */
template<class... ComponentTypes>
class SceneView {
public:
    /**
     * Constructor.
     */
    SceneView(Scene &scene) : scene_(&scene) {
        if (sizeof...(ComponentTypes) == 0) {
            all_ = true;
        } else {
            Component::Id ids[] = { Component::GetId<ComponentTypes>() ... };
            for (auto i = 0; i < sizeof...(ComponentTypes); i++) {
                mask_.set(ids[i]);
            }
        }
    }

    /**
     * Iterator allows to iterate over entities in a SceneView.
     */
    struct Iterator {
        Iterator(Scene *scene, Entity::Index index, ComponentMask mask, bool all)
            : scene_(scene), index_(index), mask_(mask), all_(all) {}

        Entity::Id operator*() const {
            return scene_->GetEntities()[index_].id_;
        }

        bool operator==(const Iterator &other) const {
            return index_ == other.index_;
        }

        bool operator!=(const Iterator &other) const {
            return index_ != other.index_;
        }

        bool IsValid() {
            return Entity::IsValid(scene_->GetEntities()[index_].id_) &&
                  (all_ || mask_ == (mask_ & scene_->GetEntities()[index_].mask_));
        }

        Iterator &operator++() {
            while (++index_ < scene_->GetEntities().size() && !IsValid());
            return *this;
        }

        Scene        *scene_{ nullptr };
        Entity::Index index_{ static_cast<Entity::Index>(-1) };
        ComponentMask  mask_{ };
        bool            all_{ false };
    };

    const Iterator begin() const {
        int first = 0;
        while (first < scene_->GetEntities().size() &&
              (mask_ != (mask_ & scene_->GetEntities()[first].mask_) || !Entity::IsValid(scene_->GetEntities()[first].id_))) {
            first++;
        }
        return Iterator(scene_, first, mask_, all_);
    }

    const Iterator end() const {
        return Iterator(scene_, Entity::Index(scene_->GetEntities().size()), mask_, all_);
    }

private:
    Scene       *scene_{ nullptr };
    bool           all_{ false };
    ComponentMask mask_{ };
};

}  // ecs
