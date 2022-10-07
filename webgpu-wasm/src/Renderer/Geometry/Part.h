#pragma once

#include <string>

class TriangleMesh;

class Part {
public:
    Part(const std::string& name);
    ~Part();

private:
    std::string m_Name;
    TriangleMesh* m_Mesh;
};