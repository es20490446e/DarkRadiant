#pragma once

#include "render/RenderableGeometry.h"

namespace entity
{

class EntityNode;

class RenderableEntityBox :
    public render::RenderableGeometry
{
private:
    const EntityNode& _node;
    bool _needsUpdate;
    bool _filledBox;

public:
    RenderableEntityBox(EntityNode& node);

    void queueUpdate();
    void setFillMode(bool fill);

    virtual void updateGeometry() override;
};

}
