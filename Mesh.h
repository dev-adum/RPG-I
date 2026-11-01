#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

// One corner of a triangle: everything the fixed-function pipeline needs to
// place and light it.
struct MeshVertex
{
    glm::vec3 position{ 0.f };
    glm::vec3 normal{ 0.f, 1.f, 0.f };
    glm::vec2 uv{ 0.f };
};

// A CPU-side triangle mesh loaded from a Wavefront .obj file.
//
// The .obj is read exactly once, at load time. Faces are fan-triangulated as
// they are parsed (Blender exports quads and n-gons), so Draw() only ever has
// to emit GL_TRIANGLES. Vertices are de-indexed into a flat list, which keeps
// the immediate-mode path trivial; an indexed VBO version can replace Draw()
// later without touching callers.
class Mesh
{
public:
    bool LoadOBJ(const std::string& path);

    // Immediate-mode draw. Requires a current model-view matrix and (for
    // lighting) GL_LIGHTING enabled.
    void Draw() const;

    bool IsLoaded() const { return !m_vertices.empty(); }

    // Local-space bounds, useful for checking the mesh scale after loading.
    const glm::vec3& Min() const { return m_min; }
    const glm::vec3& Max() const { return m_max; }

private:
    std::vector<MeshVertex> m_vertices;   // three per triangle
    glm::vec3 m_min{ 0.f };
    glm::vec3 m_max{ 0.f };
};
