#include "Part.h"

#include "Renderer/Geometry/TriangleMesh.h"

Part::Part(const std::string& name, TriangleMesh* mesh)
: m_Name(name), m_Mesh(mesh)
{
}

Part::~Part()
{
}