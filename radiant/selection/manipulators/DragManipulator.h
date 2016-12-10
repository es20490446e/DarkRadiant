#pragma once

#include "ManipulatorBase.h"
#include "ManipulatorComponents.h"

#include "selection/SelectionTest.h"
#include "selection/SelectionPool.h"
#include "selection/BasicSelectable.h"

namespace selection
{

/**
 * The DragManipulator operates on all the currently selected map objects. 
 *
 * It is a multi-purpose manipulator that can be used either to drag 
 * all the objects around (freely) or to resize applicable PlaneSelectables 
 * which are among the selection.
 *
 * It may report two different Manipulator::Components to the calling code:
 * The "free resize" component or the "free drag" component. Both components
 * operate on an abstract Translatable object, which will be invoked with
 * a translation vector on mouse movement.
 *
 * The "free drag" component will invoke the DragTranslatable::translate() method
 * which passes the translation to all selected objects or components, respectively.
 *
 * The "free resize" component will apply the translation to the component of the 
 * successfully selected PlaneSelectable objects. This might resize lights, brushes
 * or patches, for example.
 */
class DragManipulator : 
	public ManipulatorBase
{
private:
	// Resize component
	TranslateFree _freeResizeComponent;
	ResizeTranslatable _resizeTranslatable;

	// True if we are currently operating on PlaneSelectables (resize mode)
	bool _resizeModeActive;

	// Drag component
	TranslateFree _freeDragComponent;
	DragTranslatable _dragTranslatable;
	BasicSelectable _dragSelectable;

public:
	DragManipulator();

	Type getType() const override;
	Component* getActiveComponent() override;
	void testSelect(const render::View& view, const Matrix4& pivot2world) override;
	void setSelected(bool select) override;
	bool isSelected() const override;

private:
	void testSelectPrimitiveMode(const render::View& view, SelectionVolume& test, SelectionPool& selector);
	void testSelectGroupPartMode(const render::View& view, SelectionVolume& test, SelectionPool& selector);
	void testSelectEntityMode(const render::View& view, SelectionVolume& test, SelectionPool& selector);
	void testSelectComponentMode(const render::View& view, SelectionVolume& test, SelectionPool& selector);
};

}
