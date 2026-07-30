package main

import "core:fmt"

Entity_ID :: u32
INVALID_ENTITY_ID :: Entity_ID(0)

MAX_COMPONENTS :: 256

Vec3 :: struct {
    x, y, z: f32,
}

Transform_Component :: struct {
    position: Vec3,
    rotation: Vec3,
    scale: Vec3,
}

Name_Component :: struct {
    value: string,
}

Component_Storage :: struct($T: typeid) {
    entities: [MAX_COMPONENTS]Entity_ID,
    components: [MAX_COMPONENTS]T,
    count: int,
}

storage_add :: proc(storage: ^Component_Storage($T), entity: Entity_ID, component: T) -> bool {
    if entity == INVALID_ENTITY_ID {
        return false
    }

    if storage_get(storage, entity) != nil {
        return false
    }

    if storage.count >= MAX_COMPONENTS {
        return false
    }

    storage.entities[storage.count] = entity
    storage.components[storage.count] = component
    storage.count += 1

    return true
}

storage_get :: proc(storage: ^Component_Storage($T), entity: Entity_ID) -> ^T {
    for i in 0..<storage.count {
        if storage.entities[i] == entity {
            return &storage.components[i]
        }
    }
    return nil
}

storage_remove :: proc(storage: ^Component_Storage($T), entity: Entity_ID) -> bool {
    for i in 0..<storage.count {
        if storage.entities[i] == entity {
            last := storage.count - 1

            storage.entities[i] = storage.entities[last]
            storage.components[i] = storage.components[last]

            storage.count -=1
            return true
        }
    }

    return false
}

main :: proc() {
    transforms: Component_Storage(Transform_Component)

    entity := Entity_ID(1)

    transform := Transform_Component {
        position = Vec3{0, 0, 0},
        rotation = Vec3{0, 0, 0},
        scale = Vec3{1, 1, 1},
    }

    added:= storage_add(&transforms, entity, transform)
    fmt.println("added:", added)

    found := storage_get(&transforms, entity)
    if found != nil {
        fmt.println("found position:", found.position)
    }

    removed := storage_remove(&transforms, entity)
    fmt.println("removed:", removed)

    missing := storage_get(&transforms, entity)
    fmt.println("missing after remove:", missing == nil)
}