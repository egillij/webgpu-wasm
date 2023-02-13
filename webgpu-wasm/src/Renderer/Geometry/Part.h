// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>

class TriangleMesh;

class Part {
public:
    Part(const std::string& name, TriangleMesh* mesh);
    ~Part();

    TriangleMesh* getMesh() const { return m_Mesh; }

private:
    std::string m_Name;
    TriangleMesh* m_Mesh;
};