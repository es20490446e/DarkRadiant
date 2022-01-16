#pragma once

#include "debugging/debugging.h"

#include "ientity.h"
#include "ieclass.h"
#include "irender.h"
#include "iselection.h"
#include "igl.h"
#include "iselectiontest.h"

#include "generic/callback.h"
#include "math/AABB.h"
#include "scenelib.h"

#include <list>
#include <set>

class SelectionIntersection;

inline void aabb_testselect(const AABB& aabb, SelectionTest& test, SelectionIntersection& best)
{
  const IndexPointer::index_type indices[24] = {
    2, 1, 5, 6,
    1, 0, 4, 5,
    0, 1, 2, 3,
    3, 7, 4, 0,
    3, 2, 6, 7,
    7, 6, 5, 4,
  };

  Vector3 points[8];
  aabb.getCorners(points);
  VertexPointer pointer(points, sizeof(Vector3));
  test.TestQuads(pointer, IndexPointer(indices, 24), best);
}

inline void aabb_draw_wire(const Vector3 points[8])
{
  typedef unsigned int index_t;
  index_t indices[24] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7,
  };
#if 1
  glVertexPointer(3, GL_DOUBLE, 0, points);
  glDrawElements(GL_LINES, sizeof(indices)/sizeof(index_t), GL_UNSIGNED_INT, indices);
#else
  glBegin(GL_LINES);
  for(std::size_t i = 0; i < sizeof(indices)/sizeof(index_t); ++i)
  {
    glVertex3dv(points[indices[i]]);
  }
  glEnd();
#endif
}

inline void aabb_draw_flatshade(const Vector3 points[8])
{
  glBegin(GL_QUADS);

  glNormal3dv(aabb_normals[0]);
  glVertex3dv(points[2]);
  glVertex3dv(points[1]);
  glVertex3dv(points[5]);
  glVertex3dv(points[6]);

  glNormal3dv(aabb_normals[1]);
  glVertex3dv(points[1]);
  glVertex3dv(points[0]);
  glVertex3dv(points[4]);
  glVertex3dv(points[5]);

  glNormal3dv(aabb_normals[2]);
  glVertex3dv(points[0]);
  glVertex3dv(points[1]);
  glVertex3dv(points[2]);
  glVertex3dv(points[3]);

  glNormal3dv(aabb_normals[3]);
  glVertex3dv(points[0]);
  glVertex3dv(points[3]);
  glVertex3dv(points[7]);
  glVertex3dv(points[4]);

  glNormal3dv(aabb_normals[4]);
  glVertex3dv(points[3]);
  glVertex3dv(points[2]);
  glVertex3dv(points[6]);
  glVertex3dv(points[7]);

  glNormal3dv(aabb_normals[5]);
  glVertex3dv(points[7]);
  glVertex3dv(points[6]);
  glVertex3dv(points[5]);
  glVertex3dv(points[4]);

  glEnd();
}

inline void aabb_draw_wire(const AABB& aabb)
{
	Vector3 points[8];
	aabb.getCorners(points);
	aabb_draw_wire(points);
}

inline void aabb_draw_flatshade(const AABB& aabb)
{
	Vector3 points[8];
	aabb.getCorners(points);
	aabb_draw_flatshade(points);
}

inline void aabb_draw_textured(const AABB& aabb)
{
	Vector3 points[8];
	aabb.getCorners(points);

  glBegin(GL_QUADS);

  glNormal3dv(aabb_normals[0]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[2]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[1]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[5]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[6]);

  glNormal3dv(aabb_normals[1]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[1]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[0]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[4]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[5]);

  glNormal3dv(aabb_normals[2]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[0]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[1]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[2]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[3]);

  glNormal3dv(aabb_normals[3]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[0]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[3]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[7]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[4]);

  glNormal3dv(aabb_normals[4]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[3]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[2]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[6]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[7]);

  glNormal3dv(aabb_normals[5]);
  glTexCoord2dv(aabb_texcoord_topleft);
  glVertex3dv(points[7]);
  glTexCoord2dv(aabb_texcoord_topright);
  glVertex3dv(points[6]);
  glTexCoord2dv(aabb_texcoord_botright);
  glVertex3dv(points[5]);
  glTexCoord2dv(aabb_texcoord_botleft);
  glVertex3dv(points[4]);

  glEnd();
}

inline void aabb_draw_solid(const AABB& aabb, RenderStateFlags state)
{
  if(state & RENDER_TEXTURE_2D)
  {
    aabb_draw_textured(aabb);
  }
  else
  {
    aabb_draw_flatshade(aabb);
  }
}

inline void aabb_draw(const AABB& aabb, RenderStateFlags state)
{
  if(state & RENDER_FILL)
  {
    aabb_draw_solid(aabb, state);
  }
  else
  {
    aabb_draw_wire(aabb);
  }
}

class RenderableSolidAABB : public OpenGLRenderable
{
  const AABB& m_aabb;
public:
  RenderableSolidAABB(const AABB& aabb) : m_aabb(aabb)
  {
  }
  void render(const RenderInfo& info) const
  {
    aabb_draw_solid(m_aabb, info.getFlags());
  }
  const AABB& getAABB() const
  {
	  return m_aabb;
  }
};

class RenderableWireframeAABB : public OpenGLRenderable
{
  const AABB& m_aabb;
public:
  RenderableWireframeAABB(const AABB& aabb) : m_aabb(aabb)
  {
  }
  void render(const RenderInfo& info) const
  {
    aabb_draw_wire(m_aabb);
  }
};

/**
 * Stream insertion for Entity objects.
 */
inline std::ostream& operator<< (std::ostream& os, const Entity& entity) {
	os << "Entity { name=\"" << entity.getKeyValue("name") << "\", "
	   << "classname=\"" << entity.getKeyValue("classname") << "\", "
	   << "origin=\"" << entity.getKeyValue("origin") << "\" }";

	return os;
}

class EntityNodeFindByClassnameWalker :
	public scene::NodeVisitor
{
protected:
	// Name to search for
	std::string _name;

	// The search result
	scene::INodePtr _entityNode;

public:
	// Constructor
	EntityNodeFindByClassnameWalker(const std::string& name) :
		_name(name)
	{}

	scene::INodePtr getEntityNode() {
		return _entityNode;
	}

	Entity* getEntity() {
		return _entityNode != NULL ? Node_getEntity(_entityNode) : NULL;
	}

	// Pre-descent callback
	bool pre(const scene::INodePtr& node) {
		if (_entityNode == NULL) {
			// Entity not found yet
			Entity* entity = Node_getEntity(node);

			if (entity != NULL) {
				// Got an entity, let's see if the name matches
				if (entity->getKeyValue("classname") == _name) {
					_entityNode = node;
				}

				return false; // don't traverse entities
			}
			else {
				// Not an entity, traverse
				return true;
			}
		}
		else {
			// Entity already found, don't traverse any further
			return false;
		}
	}
};

/* greebo: Finds an entity with the given classname
 */
inline Entity* Scene_FindEntityByClass(const std::string& className)
{
	// Instantiate a walker to find the entity
	EntityNodeFindByClassnameWalker walker(className);

	// Walk the scenegraph
	GlobalSceneGraph().root()->traverse(walker);

	return walker.getEntity();
}

/* Check if a node is the worldspawn.
 */
inline bool Node_isWorldspawn(const scene::INodePtr& node) 
{
	Entity* entity = Node_getEntity(node);

	return entity != nullptr && entity->isWorldspawn();
}

/**
 * greebo: Changing the entity classname is a non-trivial operation in DarkRadiant, as
 * the actual c++ class of an entity is depending on it. Changing the classname
 * therefore means 1) to recreate a new entity 2) to copy all spawnargs over from the old one
 * and 3) re-parent any child nodes to the new entity.
 *
 * @node: The entity node to change the classname of.
 * @classname: The new classname.
 *
 * @returns: The new entity node.
 */
inline scene::INodePtr changeEntityClassname(const scene::INodePtr& node,
                                             const std::string& classname)
{
	// Make a copy of this node first
	scene::INodePtr oldNode(node);

	// greebo: First, get the eclass
	IEntityClassPtr eclass = GlobalEntityClassManager().findOrInsert(
		classname,
		scene::hasChildPrimitives(oldNode) // whether this entity has child primitives
	);

	// must not fail, findOrInsert always returns non-NULL
	assert(eclass);

	// Create a new entity with the given class
	IEntityNodePtr newNode(GlobalEntityModule().createEntity(eclass));

	Entity* oldEntity = Node_getEntity(oldNode);

	// Traverse the old entity with a walker
	Entity& newEntity = newNode->getEntity();

    // Copy all keyvalues except classname
    oldEntity->forEachKeyValue([&](const std::string& key, const std::string& value)
    {
        if (key != "classname") 
        {
            newEntity.setKeyValue(key, value);
        }
    });

	// Remember the oldNode's parent before removing it
	scene::INodePtr parent = oldNode->getParent();

	// The old node must not be the root node or an orphaned one
	assert(parent);

	// Traverse the child and reparent all primitives to the new entity node
	scene::parentPrimitives(oldNode, newNode);

	// Remove the old entity node from the parent. This will disconnect 
	// oldNode from the scene and the UndoSystem, so it's important to do 
	// this step last, after the primitives have been moved. (#4718)
	scene::removeNodeFromParent(oldNode);

	// Let the new node keep its layer information (#4710)
    // Apply the layers to the whole subgraph (#5214)
    scene::AssignNodeToLayersWalker layerWalker(oldNode->getLayers());
    newNode->traverse(layerWalker);

	// Insert the new entity to the parent
	parent->addChildNode(newNode);

	return newNode;
}

/**
 * greebo: This class can be used to traverse a subgraph to search
 * for a specific spawnarg on the worldspawn entity. The method
 * getValue() can be used to retrieve the value of the key as specified
 * in the constructor.
 */
class WorldspawnArgFinder :
    public scene::NodeVisitor
{
    std::string _key;
    std::string _value;

public:
    WorldspawnArgFinder(const std::string& keyName) :
        _key(keyName)
    {}

    bool pre(const scene::INodePtr& node) override
    {
        // Try to cast this node onto an entity
        auto* ent = Node_getEntity(node);

        if (ent != nullptr)
        {
            if (ent->isWorldspawn())
            {
                // Load the description spawnarg
                _value = ent->getKeyValue(_key);
            }

            return false; // don't traverse entities
        }

        return true;
    }

    /**
     * Returns the found value for the desired spawnarg. If not found,
     * this function will return an empty string "".
     */
    const std::string& getFoundValue() const
    {
        return _value;
    }
};

namespace scene
{

inline void foreachSelectedEntity(const std::function<void(Entity&)>& functor)
{
    GlobalSelectionSystem().foreachSelected([&](const INodePtr& node)
    {
        if (Node_isEntity(node))
        {
            functor(*Node_getEntity(node));
        }
    });
}

}
