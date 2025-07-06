#pragma once

#include <util/geometry.h>
#include <vector>

class Model {
  private:
    std::vector<Vec3f> vertices_ = {}; // 顶点
    std::vector<int> faces_ = {};      // 每个面顶点在上面顶点数组中的索引
  public:
    Model(const std::string &filename);
    int num_vertices() const { return vertices_.size(); } // 顶点个数
    int num_faces() const { return faces_.size() / 3; }       // 面的个数
    // 0 <= vertex_index < num_vertices()
    Vec3f vertex(int vertex_index) { return vertices_[vertex_index]; }
    // 0 <= face_index < nums_faces, 0 <= vertex_nth_of_face < 3
    Vec3f vertex(int face_index, int vertex_nth_of_face) {
        return vertices_[faces_[face_index * 3 + vertex_nth_of_face]];
    }
};