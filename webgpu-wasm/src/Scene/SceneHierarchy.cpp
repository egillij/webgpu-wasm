// Copyright 2023 Egill Ingi Jacobsen

#include "SceneHierarchy.h"

SceneHierarchy::SceneHierarchy()
{
    m_Root = new Node;
}   

SceneHierarchy::~SceneHierarchy()
{
    //TODO: cleanup
}