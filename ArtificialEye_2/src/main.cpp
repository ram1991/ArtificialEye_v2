// #define USE_DOUBLE
#include "Rendering/Renderer.hpp"
#include "Rendering/TexturePacks/UniColorTextPack.hpp"
#include "Rendering/TexturePacks/LightUniColorTextpack.hpp"
#include "Rendering/Modeling/DynamicMesh.hpp"
#include "Rendering/Modeling/SkyBox.hpp"
#include "Rendering/Modeling/Line.hpp"
#include "Rendering/RenderingUtilities.hpp"
#include "Rendering/TexturePacks/SkyBoxTextPack.hpp"
#include "Rendering/TexturePacks/RefractTextPack.hpp"
#include "Rendering/TexturePacks/LineUniColorTextPack.hpp"

#include "SoftBody/Simulation/SBClosedBodySim.hpp"
#include "SoftBody/ForceGens/SBGravity.hpp"
#include "SoftBody/Constraints/SBPointConstraint.hpp"
#include "SoftBody/Integrators/SBVerletIntegrator.hpp"
#include "SoftBody/SBUtilities.hpp"

#include "Rendering/RayTracing/Intersections.hpp"

#include <string>
#include <iostream>
#include <vector>

bool g_startSoftBody = false;
bool g_enableWireFram = false;

std::vector<ee::SBPointConstraint*> g_constraints;
const float g_constraintMoveSpeed = 0.1f;

const float DEFAULT_P = 0.f; // 10.f;

const unsigned g_sphereLat = 64U;
const unsigned g_sphereLon = 64U;

bool g_defaultP = true;

void setSpaceCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE)
    {
        if (action == GLFW_PRESS)
        {
            g_startSoftBody = !g_startSoftBody;
        }
    }
    else if (key == GLFW_KEY_N)
    {
        if (action == GLFW_PRESS)
        {
            g_enableWireFram = !g_enableWireFram;
        }
    }
    else if (key == GLFW_KEY_R)
    {
        if (action == GLFW_PRESS)
        {
            g_defaultP = !g_defaultP;
        }
    }

    if (action == GLFW_PRESS && (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN))
    {
        float dir;
        switch (key)
        {
        case GLFW_KEY_UP:
            dir = 1.f;
            break;
        case GLFW_KEY_DOWN:
            dir = -1.f;
            break;
        }

        for (auto& constraint : g_constraints)
        {
            constraint->m_point += g_constraintMoveSpeed * dir * glm::normalize(constraint->m_point);
        }
    }
}

int main()
{
    try
    {
        ee::RendererParam rendererParams;
        ee::CameraParam cameraParams;
        std::string sharedDir = "Shaders";

        rendererParams.m_aspect = 16.f / 9.f;
        rendererParams.m_screenWidth = 1920;
        rendererParams.m_screenHeight = 1080;
        rendererParams.m_fov = 45.f;
        rendererParams.m_far = 100.f;
        rendererParams.m_near = 0.1f;

        cameraParams.m_position = ee::Vector3(10.f, 0.f, 0.f);
        cameraParams.m_up = ee::Vector3(0.f, 1.f, 0.f);
        cameraParams.m_yaw = 180.f;
        cameraParams.m_pitch = 0.f;

        ee::Renderer::ErrorCode res = ee::Renderer::initialize(sharedDir, rendererParams, cameraParams);
        ee::Renderer::setCustomKeyboardCallback(setSpaceCallback);

        ee::VertBuffer vertBuffer;
        ee::MeshFaceBuffer indBuffer;
        // ee::loadIcosphere(4, &vertBuffer, &indBuffer);
        ee::loadUVsphere(g_sphereLon, g_sphereLat, &vertBuffer, &indBuffer);

        // now create a model:

        ee::UniColorTextPack uniColor(ee::Color4(0.f, 1.f, 0.f, 1.f));
        ee::Renderer::addTexturePack("uniColor", &uniColor);

        ee::LightUniColorTextPack lightUniColor;
        lightUniColor.m_color = ee::Color3(0.2f, 1.f, 0.f);
        lightUniColor.m_lightPosition = ee::Vector3(2.f, 2.f, 2.f);
        lightUniColor.m_material.m_ambient = ee::Vector3(1.0f, 0.5f, 0.31f);
        lightUniColor.m_material.m_diffuse = ee::Vector3(1.0f, 0.5f, 0.31f);
        ee::Renderer::addTexturePack("lightUniColor", &lightUniColor);

        std::vector<std::string> cubemapcomp
        {
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "back.jpg",
            "front.jpg"
        };

        ee::RefractTextPack refractTextPack(ee::Color3(), "Textures/SkyBox", cubemapcomp, 1.33f);
        ee::SkyBoxTextPack skyBoxTextPack("Textures/SkyBox", cubemapcomp);
        ee::LineUniColorTextPack lineTextPack(ee::Color3(1.f, 0.f, 0.f));
        ee::Renderer::addTexturePack("SkyBoxTextPack", &skyBoxTextPack);
        ee::Renderer::addTexturePack("refractTextPack", &refractTextPack);
        ee::Renderer::addTexturePack("lineTextPack", &lineTextPack);

        // load the skybox
        ee::SkyBox skyBox("SkyBoxTextPack");
        ee::Renderer::addDrawable(&skyBox);

        // load the dynModel
        ee::DynamicMesh dynModel("refractTextPack", std::move(vertBuffer), std::move(indBuffer));
        ee::Renderer::addDrawable(&dynModel);

        // load the model
        ee::Line startLine("lineTextPack", ee::Vector3(0.65f, 0.65f, -10.f), ee::Vector3(0.65f, 0.65f, 0.f));
        std::vector<ee::Line> lines;
        lines.push_back(ee::Line("lineTextPack", ee::Vector3(), ee::Vector3()));
        lines.push_back(ee::Line("lineTextPack", ee::Vector3(), ee::Vector3()));
        ee::Renderer::addDrawable(&startLine);
        ee::Renderer::addDrawable(&lines[0]);
        ee::Renderer::addDrawable(&lines[1]);

        // transform the sphere:
        glm::mat4 modelTrans = glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
        modelTrans = glm::scale(modelTrans, glm::vec3(1.f, 0.5, 1.f));

        dynModel.applyTransformation(modelTrans);
        dynModel.m_modelTrans = glm::mat4();

        ee::Renderer::setClearColor(ee::Color3(0.45f, 0.45f, 0.45f));

        ee::SBClosedBodySim clothSim(DEFAULT_P, &dynModel, 5.f, 3.f, 0.0f);
        clothSim.m_constIterations = 10.f;

        ee::SBGravity* gravity = new ee::SBGravity();
        // clothSim.addGlobalForceGen(gravity);

        // find the middle point of the sphere to start the ring:
        std::size_t index0 = (g_sphereLat / 2 - 2) * (g_sphereLon) + 1;
        std::size_t end0 = index0 + g_sphereLon;

        std::size_t index1 = (g_sphereLat / 2 - 1) * (g_sphereLon) + 1;
        std::size_t end1 = index1 + g_sphereLon;

        std::size_t index2 = (g_sphereLat / 2) * (g_sphereLon) + 1;
        std::size_t end2 = index2 + g_sphereLon;

        // add the constraints:
        for (std::size_t i = index0; i < end0; i++)
        {
            auto ptr = clothSim.addConstraint(&ee::SBPointConstraint(dynModel.getVertex(i).m_position, clothSim.getVertexObject(i)));
            g_constraints.push_back(ptr);
        }

        for (std::size_t i = index1; i < end1; i++)
        {
            auto ptr = clothSim.addConstraint(&ee::SBPointConstraint(dynModel.getVertex(i).m_position, clothSim.getVertexObject(i)));
            g_constraints.push_back(ptr);
        }

        for (std::size_t i = index2; i < end2; i++)
        {
            auto ptr = clothSim.addConstraint(&ee::SBPointConstraint(dynModel.getVertex(i).m_position, clothSim.getVertexObject(i)));
            g_constraints.push_back(ptr);
        }

        float len = glm::length(dynModel.getVertex(0).m_position - dynModel.getVertex(dynModel.getNumVertices() - 1).m_position);
        //clothSim.addCustomLengthConstraint(len, 0, dynModel.getNumVertices() - 1);

        clothSim.addIntegrator(&ee::SBVerletIntegrator(1.f / 30.f, 0.01f));

        ee::addBendSpringsForUVSphere(&clothSim, g_sphereLat, g_sphereLon, 100.f, 1.f);

        while (ee::Renderer::isInitialized())
        {
            if (g_enableWireFram)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            if (g_defaultP)
            {
                clothSim.setP(DEFAULT_P);
            }
            else
            {
                clothSim.setP(0.f);
            }

            ee::Renderer::clearBuffers();

            float time = ee::Renderer::timeElapsed();
            if (g_startSoftBody)
            {
                clothSim.update(time);
            }

            auto updatedDir = meshRefract(&dynModel, startLine.getRay(), 1.f / 1.66f);
            for (int i = 0; i < updatedDir.size(); i++)
                lines[i].setRay(updatedDir[i], 5);
            
            ee::Renderer::drawAll();

            ee::Renderer::update(time);
            ee::Renderer::swapBuffers();
            ee::Renderer::pollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[EXCEP THROWN]: " << std::endl;
        std::cout << e.what() << std::endl;
        std::cout << "Press ENTER to exit." << std::endl;
        std::cin.get();
        return -1;
    }

    return 0;
}