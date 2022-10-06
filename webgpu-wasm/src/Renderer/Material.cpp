#include "Material.h"

Material::Material(const std::string& name, MaterialType type)
: m_Name(name), m_Type(type),
  m_MaterialBindGroupLayout(nullptr), m_MaterialBindGroup(nullptr),
  m_UniformBuffer(nullptr)
{
}

Material::~Material()
{
}