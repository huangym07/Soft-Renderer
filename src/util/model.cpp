#include <util/model.h>
#include <fstream>
#include <sstream>

Model::Model(const std::string &filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open " << filename << '\n';
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::stringstream line_ss(line);
        std::string prefix;
        line_ss >> prefix;
        if (prefix == "v") {
            float x, y, z;
            line_ss >> x >> y >> z;
            vertices_.push_back({x, y, z});
        } else if (prefix == "f") {
            std::string seg;
            while (line_ss >> seg) {
                std::stringstream seg_ss(seg);
                int index;
                seg_ss >> index;
                faces_.push_back(--index);
            }
        }
    }

    std::cerr << "Vertices: #" << num_vertices() << '\n';
    std::cerr << "Faces: #" << num_faces() << '\n';
}