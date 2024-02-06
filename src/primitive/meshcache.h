#pragma once

#include <unordered_map>
#include "primitive/mesh.h"

class MeshCache {
public:
    static MeshCache& getInstance() {
        static MeshCache instance;
        return instance;
    }

    Mesh loadMeshWithCache(const std::string& meshfile);

private:
    MeshCache() {} // Private constructor

    // Delete copy and assignment operators
    MeshCache(MeshCache const&) = delete;
    void operator=(MeshCache const&)  = delete;

    std::unordered_map<std::string, Mesh> cache;
};
