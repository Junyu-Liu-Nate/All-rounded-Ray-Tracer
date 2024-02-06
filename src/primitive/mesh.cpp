#include "mesh.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

// Note: error handling added for invalid mesh file
Mesh loadMesh(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open .obj file");
    }

    Mesh mesh;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "v") {
            glm::vec3 vertex;
            stream >> vertex.x >> vertex.y >> vertex.z;
            if (stream.fail()) {
                throw std::runtime_error("Error reading vertex data.");
            }
            mesh.vertices.push_back(vertex);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            if (stream.fail()) {
                throw std::runtime_error("Error reading vertex normal data.");
            }
            mesh.normals.push_back(normal);
        }
        else if (type == "f") {
            Face face;
            char skip;
            for (int i = 0; i < 3; i++) {
                if (line.find("//") != std::string::npos) {
                    stream >> face.v[i] >> skip >> skip >> face.vn[i];
                } else {
                    stream >> face.v[i];                }

                if (stream.fail()) {
                    throw std::runtime_error("Error reading face data.");
                }

                // Adjusting for 0-based index
                face.v[i] -= 1;
                face.vn[i] -= 1;
            }
            mesh.faces.push_back(face);
        }
        // Skipping comment lines
        else if (type == "#") {
            continue;
        }
    }

    file.close();
    return mesh;
}




