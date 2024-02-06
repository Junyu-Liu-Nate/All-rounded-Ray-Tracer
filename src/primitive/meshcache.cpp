#include "meshcache.h"

Mesh MeshCache::loadMeshWithCache(const std::string& meshfile) {
    auto it = cache.find(meshfile);
    if (it != cache.end()) {
        return it->second;
    }

    Mesh mesh = loadMesh(meshfile);
    cache[meshfile] = mesh;
    return mesh;
}
