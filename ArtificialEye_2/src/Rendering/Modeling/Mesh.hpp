#pragma once

#include "../Drawable.hpp"
#include "../Renderer.hpp"
#include "../TexturePacks/TexturePack.hpp"
#include "../../Types.hpp"

#include <vector>
#include <string>
#include <memory>
#include <glad/glad.h>

namespace ee
{
    struct Vertex
    {
        Vector3 m_position;
        Vector3 m_textCoord;
        Vector3 m_normal;

        Vertex() {}
        Vertex(Vector3 pos) :
            m_position(pos) {}
        Vertex(Vector3 pos, Vector3 textCoord, Vector3 normal) :
            m_position(pos), m_textCoord(textCoord), m_normal(normal) {}
    };

    using VertBuffer = std::vector<Vertex>;
    using IndexBuffer = std::vector<GLuint>;

    class Mesh : public Drawable
    {
    public:
        explicit Mesh(std::string textPack, VertBuffer vertices, IndexBuffer indices, GLenum dataUsage = GL_STATIC_DRAW);
        explicit Mesh(const Mesh& model);
        Mesh(Mesh&& model);

        virtual ~Mesh();

        virtual const Vertex& getVertex(std::size_t vertexID) const;
        virtual std::size_t getNumVertices() const;

        virtual std::size_t getVertexID(std::size_t indexID) const;
        virtual std::size_t getNumIndices() const;

        // used to position the model in the world
        glm::mat4 m_modelTrans;

        virtual void draw() override;

        virtual ee::Float calcVolume() const;
        virtual void calcNormals();

    private:
        std::vector<Vector3> m_tempNormals;

        void constructVAO();
        const GLenum m_type; // for when copying the object

    protected:
        VertBuffer m_vertices;
        IndexBuffer m_indices;

        // if a subclass wants to take care of the models
        GLuint m_VAO;
        GLuint m_EBO;
        GLuint m_VBO;
    };
}