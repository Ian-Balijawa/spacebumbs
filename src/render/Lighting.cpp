#include "render/Lighting.hpp"

#include "core/Paths.hpp"

namespace sg {

Lighting::Lighting() {
    const Shader shader = LoadShader(assetPath("shaders/lit.vs").c_str(), assetPath("shaders/lit.fs").c_str());
    material_ = LoadMaterialDefault();
    material_.shader = shader;

    sunLoc_ = GetShaderLocation(shader, "sunPos");
    viewLoc_ = GetShaderLocation(shader, "viewPos");
    lightColorLoc_ = GetShaderLocation(shader, "lightColor");
    ambientLoc_ = GetShaderLocation(shader, "ambientColor");

    const Vector3 lightColor{1.0f, 0.97f, 0.9f};
    const Vector3 ambient{0.10f, 0.11f, 0.15f};
    SetShaderValue(shader, lightColorLoc_, &lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, ambientLoc_, &ambient, SHADER_UNIFORM_VEC3);
}

// Unloads the shader that the material owns.
Lighting::~Lighting() { UnloadMaterial(material_); }

void Lighting::beginFrame(Vector3 cameraPosition) {
    SetShaderValue(material_.shader, viewLoc_, &cameraPosition, SHADER_UNIFORM_VEC3);
    SetShaderValue(material_.shader, sunLoc_, &sun_, SHADER_UNIFORM_VEC3);
}

void Lighting::drawMesh(const Mesh& mesh, Matrix transform, Color color) {
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color;
    DrawMesh(mesh, material_, transform);
}

}  // namespace sg
