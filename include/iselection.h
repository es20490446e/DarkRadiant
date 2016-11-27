#pragma once

#include <cstddef>
#include "imodule.h"
#include "ivolumetest.h"
#include <memory>
#include <sigc++/signal.h>

class RenderableCollector;
namespace render { class View; }

class ISelectable;


namespace scene
{
	class INode;
	typedef std::shared_ptr<INode> INodePtr;
}

template<typename Element> class BasicVector2;
typedef BasicVector2<double> Vector2;
template<typename Element> class BasicVector3;
typedef BasicVector3<double> Vector3;
template<typename Element> class BasicVector4;
typedef BasicVector4<double> Vector4;
class Matrix4;
class Quaternion;

typedef sigc::signal<void, const ISelectable&> SelectionChangedSignal;
typedef sigc::slot<void, const ISelectable&> SelectionChangedSlot;

class SelectionInfo;
class Face;
class Brush;
class Patch;

namespace selection 
{ 
	
struct WorkZone; 

/**
* A Manipulator is a renderable object which contains one or more
* ManipulatorComponents, each of which can be manipulated by the user. For
* example, the rotation Manipulator draws several circles which cause rotations
* around specific axes.
*/
class Manipulator
{
public:
	// Manipulator type enum, user-defined manipulators should return "Custom"
	enum Type
	{
		Drag,
		Translate,
		Rotate,
		Scale,
		Clip,
		Custom
	};

	/**
	* Part of a Manipulator which can be operated upon by the user.
	*
	* \see Manipulator
	*/
	class Component
	{
	public:
		virtual ~Component() {}

		virtual void Construct(const Matrix4& device2manip, const float x, const float y) = 0;

		// greebo: An abstract Transform() method, the implementation has to decide
		// which operations are actually called. This may be a translation,
		// rotation, or anything else.
		virtual void Transform(const Matrix4& manip2object,
			const Matrix4& device2manip,
			const float x,
			const float y) = 0;
	};

	virtual ~Manipulator() {}

	virtual Type getType() const = 0;

	/**
	* Get the currently-active ManipulatorComponent. This is determined by the
	* most recent selection test.
	*/
	virtual Component* getActiveComponent() = 0;

	virtual void testSelect(const render::View& view, const Matrix4& pivot2world) {}

	// This function is responsible for bringing the visual representation
	// of this manipulator onto the screen
	virtual void render(RenderableCollector& collector,
		const VolumeTest& volume,
		const Matrix4& pivot2world) {}

	virtual void setSelected(bool select) = 0;
	virtual bool isSelected() const = 0;
};
typedef std::shared_ptr<Manipulator> ManipulatorPtr;

}

class SelectionSystem :
	public RegisterableModule
{
public:
  enum EModifier {
	eManipulator,	// greebo: This is the standard case (drag, click without modifiers)
	eToggle,	// This is for Shift-Clicks to toggle the selection of an instance
	eReplace,	// This is active if the mouse is moved to a NEW location and Alt-Shift is held
	eCycle,		// This is active if the mouse STAYS at the same position and Alt-Shift is held
  };

  enum EMode
  {
    eEntity,
    ePrimitive,
	eGroupPart,
    eComponent,
  };

	// The possible modes when in "component manipulation mode"
	enum EComponentMode {
		eDefault,
		eVertex,
		eEdge,
		eFace,
	};

	/** greebo: An SelectionSystem::Observer gets notified
	 * as soon as the selection is changed.
	 */
	class Observer
	{
	public:
		virtual ~Observer() {}
		/** greebo: This gets called upon selection change.
		 *
		 * @instance: The instance that got affected (this may also be the parent brush of a FaceInstance).
		 * @isComponent: is TRUE if the changed selectable is a component (like a FaceInstance, VertexInstance).
		 */
		virtual void selectionChanged(const scene::INodePtr& node, bool isComponent) = 0;
	};

	virtual void addObserver(Observer* observer) = 0;
	virtual void removeObserver(Observer* observer) = 0;

	// Returns the ID of the registered manipulator
	virtual std::size_t registerManipulator(const selection::ManipulatorPtr& manipulator) = 0;
	virtual void unregisterManipulator(const selection::ManipulatorPtr& manipulator) = 0;

	virtual selection::Manipulator::Type getActiveManipulatorType() = 0;
	virtual void setActiveManipulator(std::size_t manipulatorId) = 0;
	virtual void setActiveManipulator(selection::Manipulator::Type manipulatorType) = 0;

	virtual const SelectionInfo& getSelectionInfo() = 0;

  virtual void SetMode(EMode mode) = 0;
  virtual EMode Mode() const = 0;
  virtual void SetComponentMode(EComponentMode mode) = 0;
  virtual EComponentMode ComponentMode() const = 0;

  virtual std::size_t countSelected() const = 0;
  virtual std::size_t countSelectedComponents() const = 0;
  virtual void onSelectedChanged(const scene::INodePtr& node, const ISelectable& selectable) = 0;
  virtual void onComponentSelection(const scene::INodePtr& node, const ISelectable& selectable) = 0;

	virtual scene::INodePtr ultimateSelected() = 0;
	virtual scene::INodePtr penultimateSelected() = 0;

    /**
     * \brief
     * Set the selection status of all objects in the scene.
     *
     * \param selected
     * true to select all objects, false to deselect all objects.
     */
    virtual void setSelectedAll(bool selected) = 0;

  virtual void setSelectedAllComponents(bool selected) = 0;

    /**
     * @brief
     * Visitor interface the for the selection system.
     *
     * This defines the Visitor interface which is used in the foreachSelected()
     * and foreachSelectedComponent() visit methods.
     */
	class Visitor
	{
	public:
		virtual ~Visitor() {}

        /**
         * @brief
         * Called by the selection system for each visited node.
         */
		virtual void visit(const scene::INodePtr& node) const = 0;
	};

    /**
     * @brief
     * Use the provided Visitor object to enumerate each selected node.
     */
    virtual void foreachSelected(const Visitor& visitor) = 0;

    /**
     * @brief
     * Use the provided Visitor object to enumerate each selected component.
     */
    virtual void foreachSelectedComponent(const Visitor& visitor) = 0;

	/**
	 * Call the given functor to enumerate each selected node.
	 */
	virtual void foreachSelected(const std::function<void(const scene::INodePtr&)>& functor) = 0;

	/**
     * @brief
     * Use the provided functor to enumerate each selected component.
     */
	virtual void foreachSelectedComponent(const std::function<void(const scene::INodePtr&)>& functor) = 0;

	/**
	 * Call the given functor for each selected brush. Selected group nodes like func_statics
	 * are traversed recursively, invoking the functor for each visible brush in question.
	 */
	virtual void foreachBrush(const std::function<void(Brush&)>& functor) = 0;

	/**
	 * Call the given functor for each selected face. Selected group nodes like func_statics
	 * are traversed recursively, invoking the functor for each visible face in question.
	 * Singly selected faces (those which have been selected in component mode) are 
	 * considered as well by this method.
	 */
	virtual void foreachFace(const std::function<void(Face&)>& functor) = 0;

	/**
	 * Call the given functor for each selected patch. Selected group nodes like func_statics
	 * are traversed recursively, invoking the functor for each visible patch in question.
	 */
	virtual void foreachPatch(const std::function<void(Patch&)>& functor) = 0;

    /// Signal emitted when the selection is changed
    virtual SelectionChangedSignal signal_selectionChanged() const = 0;

    virtual void translateSelected(const Vector3& translation) = 0;
    virtual void rotateSelected(const Quaternion& rotation) = 0;
    virtual void scaleSelected(const Vector3& scaling) = 0;
    
    virtual void pivotChanged() const = 0;
    
    virtual bool SelectManipulator(const render::View& view, const Vector2& devicePoint, const Vector2& deviceEpsilon) = 0;
    virtual void SelectPoint(const render::View& view, const Vector2& devicePoint, const Vector2& deviceEpsilon, EModifier modifier, bool face) = 0;
    virtual void SelectArea(const render::View& view, const Vector2& devicePoint, const Vector2& deviceDelta, EModifier modifier, bool face) = 0;
    
    virtual void MoveSelected(const render::View& view, const Vector2& devicePoint) = 0;
    virtual void endMove() = 0;
    virtual void cancelMove() = 0;

	/**
	 * Returns the current "work zone", which is defined by the
	 * currently selected elements. Each time a scene node is selected,
	 * the workzone is adjusted to surround the current selection.
	 * Deselecting nodes doesn't change the workzone.
	 *
	 * The result is used to determine the "third" component of operations
	 * performed in the 2D views, like placing an entity.
	 *
	 * Note: the struct is defined in selectionlib.h.
	 */
	virtual const selection::WorkZone& getWorkZone() = 0;
};

const char* const MODULE_SELECTIONSYSTEM("SelectionSystem");

inline SelectionSystem& GlobalSelectionSystem() {
	// Cache the reference locally
	static SelectionSystem& _selectionSystem(
		*std::static_pointer_cast<SelectionSystem>(
			module::GlobalModuleRegistry().getModule(MODULE_SELECTIONSYSTEM)
		)
	);
	return _selectionSystem;
}
