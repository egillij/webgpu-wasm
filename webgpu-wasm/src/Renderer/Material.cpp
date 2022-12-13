#include "Material.h"

Material::Material(const std::string& name, MaterialType type)
: m_Name(name), m_Type(type),
  m_MaterialBindGroupLayout(nullptr), m_MaterialBindGroup(nullptr),
  m_UniformBuffer(nullptr)
{
}

Material::~Material()
{
    if(m_MaterialBindGroupLayout) delete m_MaterialBindGroupLayout;
    m_MaterialBindGroupLayout = nullptr;
    if(m_MaterialBindGroup) delete m_MaterialBindGroup;
    m_MaterialBindGroup = nullptr;
    if(m_UniformBuffer) delete m_UniformBuffer;
    m_UniformBuffer = nullptr;
}