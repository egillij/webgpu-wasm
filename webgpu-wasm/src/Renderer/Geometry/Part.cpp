#include "Part.h"

#include "Renderer/Geometry/TriangleMesh.h"

Part::Part(const std::string& name)
: m_Name(name), m_Mesh(nullptr)
{
}

Part::~Part()
{
}