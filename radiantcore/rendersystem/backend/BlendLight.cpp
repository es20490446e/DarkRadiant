#include "BlendLight.h"

#include "OpenGLShader.h"
#include "glprogram/BlendLightProgram.h"

namespace render
{

BlendLight::BlendLight(RendererLight& light, IGeometryStore& store, IObjectRenderer& objectRenderer) :
    _light(light),
    _store(store),
    _objectRenderer(objectRenderer),
    _lightBounds(light.lightAABB()),
    _objectCount(0)
{}

bool BlendLight::isInView(const IRenderView& view)
{
    return view.TestAABB(_lightBounds) != VOLUME_OUTSIDE;
}

void BlendLight::collectSurfaces(const IRenderView& view, const std::set<IRenderEntityPtr>& entities)
{
    // Now check all the entities intersecting with this light
    for (const auto& entity : entities)
    {
        entity->foreachRenderableTouchingBounds(_lightBounds,
            [&](const IRenderableObject::Ptr& object, Shader* shader)
        {
            // Skip empty objects and invisible surfaces
            if (!object->isVisible() || !shader->isVisible()) return;

            // Cull surfaces that are not in view
            if (object->isOriented())
            {
                if (view.TestAABB(object->getObjectBounds(), object->getObjectTransform()) == VOLUME_OUTSIDE)
                {
                    return;
                }
            }
            else if (view.TestAABB(object->getObjectBounds()) == VOLUME_OUTSIDE) // non-oriented AABB test
            {
                return;
            }

            auto glShader = static_cast<OpenGLShader*>(shader);

            // We only consider materials designated for camera rendering
            if (!glShader->isApplicableTo(RenderViewType::Camera))
            {
                return;
            }

            _objects.emplace_back(std::ref(*object));

            ++_objectCount;
        });
    }
}

void BlendLight::draw(OpenGLState& state, RenderStateFlags globalFlagsMask, 
    BlendLightProgram& program, const IRenderView& view, std::size_t time)
{
    program.setLightTextureTransform(_light.getLightTextureTransformation());
    
    for (const auto& objectRef : _objects)
    {
        auto& object = objectRef.get();

        auto lightShader = static_cast<OpenGLShader*>(_light.getShader().get());

        lightShader->foreachPass([&](OpenGLShaderPass& pass)
        {
            // Evaluate the stage before deciding whether it's active
            pass.evaluateShaderStages(time, &_light.getLightEntity());

            if (!pass.stateIsActive()) return;

            // The light textures will be bound by applyState.
            // The texture0/texture1 units will have already been filled in when constructing the pass

            // Apply our state to the current state object
            pass.applyState(state, globalFlagsMask);
            
            program.setBlendColour(pass.state().getColour());
            program.setObjectTransform(object.getObjectTransform());
            
            _objectRenderer.submitGeometry(object.getStorageLocation(), GL_TRIANGLES);
            ++_drawCalls;
        });
    }
}

}
