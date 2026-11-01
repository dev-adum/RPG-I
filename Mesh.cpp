#include "Mesh.h"

#include <SFML/OpenGL.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
    // One corner of an .obj face. Indices are 1-based (or negative, meaning
    // "counted back from the end"); 0 means "this face does not supply one".
    struct FaceCorner
    {
        int position = 0;
        int uv = 0;
        int normal = 0;
    };

    // Parses "v", "v/vt", "v//vn" or "v/vt/vn".
    FaceCorner ParseCorner(const std::string& token)
    {
        FaceCorner corner;

        int slot = 0;
        std::string field;
        for (std::size_t i = 0; i <= token.size(); ++i)
        {
            if (i == token.size() || token[i] == '/')
            {
                if (!field.empty())
                {
                    const int value = std::atoi(field.c_str());
                    if (slot == 0)      corner.position = value;
                    else if (slot == 1) corner.uv = value;
                    else if (slot == 2) corner.normal = value;
                }
                field.clear();
                ++slot;
            }
            else
            {
                field += token[i];
            }
        }

        return corner;
    }

    int ResolveIndex(int index, std::size_t count)
    {
        if (index > 0)
            return index - 1;
        if (index < 0)
            return static_cast<int>(count) + index;
        return -1;
    }

    bool ValidIndex(int index, std::size_t count)
    {
        return index >= 0 && index < static_cast<int>(count);
    }
}

bool Mesh::LoadOBJ(const std::string& path)
{
    m_vertices.clear();

    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Mesh: cannot open " << path << '\n';
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        std::string tag;
        stream >> tag;

        if (tag == "v")
        {
            glm::vec3 p{};
            stream >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn")
        {
            glm::vec3 n{};
            stream >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "vt")
        {
            glm::vec2 t{};
            stream >> t.x >> t.y;
            uvs.push_back(t);
        }
        else if (tag == "f")
        {
            std::vector<FaceCorner> corners;
            std::string token;
            while (stream >> token)
                corners.push_back(ParseCorner(token));

            if (corners.size() < 3)
                continue;

            // Fan triangulation. Faces in this asset are convex, so a fan
            // from the first corner is exact.
            for (std::size_t i = 1; i + 1 < corners.size(); ++i)
            {
                const FaceCorner* triangle[3] = {
                    &corners[0], &corners[i], &corners[i + 1]
                };

                bool valid = true;
                for (const FaceCorner* corner : triangle)
                {
                    const int pi = ResolveIndex(corner->position, positions.size());
                    if (!ValidIndex(pi, positions.size()))
                        valid = false;
                }
                if (!valid)
                    continue;

                for (const FaceCorner* corner : triangle)
                {
                    MeshVertex vertex{};
                    vertex.position =
                        positions[ResolveIndex(corner->position, positions.size())];

                    if (corner->normal != 0)
                    {
                        const int ni = ResolveIndex(corner->normal, normals.size());
                        if (ValidIndex(ni, normals.size()))
                            vertex.normal = normals[ni];
                    }

                    if (corner->uv != 0)
                    {
                        const int ti = ResolveIndex(corner->uv, uvs.size());
                        if (ValidIndex(ti, uvs.size()))
                            vertex.uv = uvs[ti];
                    }

                    m_vertices.push_back(vertex);
                }
            }
        }
    }

    if (m_vertices.empty())
    {
        std::cerr << "Mesh: no triangles in " << path << '\n';
        return false;
    }

    m_min = m_vertices.front().position;
    m_max = m_vertices.front().position;
    for (const MeshVertex& vertex : m_vertices)
    {
        m_min = glm::min(m_min, vertex.position);
        m_max = glm::max(m_max, vertex.position);
    }

    std::cout << "Mesh: loaded " << path
              << " (" << m_vertices.size() / 3 << " triangles)\n";
    return true;
}

void Mesh::Draw() const
{
    glBegin(GL_TRIANGLES);
    for (const MeshVertex& vertex : m_vertices)
    {
        glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
        glTexCoord2f(vertex.uv.x, vertex.uv.y);
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    glEnd();
}
