#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Types.hpp"

namespace ee
{
    class Shader
    {
    public:
        Shader();

        bool initialize(const std::string &vtxPath, const std::string &frgPath, const std::string & geomPath = "");
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& shader);

        void use() const;

        void assignTexture(std::string name, int textureUnit);
        void assignMat4f(std::string name, const glm::mat4& mat);
        void assignBool(std::string name, bool value);
        void assignColor(std::string name, glm::vec3 color);
        void assignColor(std::string name, glm::vec4 color);
        void assignVec3f(std::string name, glm::vec3 vec);
        void assignfloat(std::string name, float val);

        void bindTexture(GLenum target, GLuint number, GLuint texture);
        void unbindTexture(GLenum target);

    private:
        GLuint m_programID;
    };
}