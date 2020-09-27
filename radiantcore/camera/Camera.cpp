#include "Camera.h"

#include <functional>
#include "iregistry.h"
#include "CameraManager.h"
#include "render/View.h"
#include "selection/SelectionVolume.h"
#include "Rectangle.h"

namespace camera
{

namespace
{
	const std::string RKEY_SELECT_EPSILON = "user/ui/selectionEpsilon";

	const Matrix4 g_radiant2opengl = Matrix4::byColumns(
		0, -1, 0, 0,
		0, 0, 1, 0,
		-1, 0, 0, 0,
		0, 0, 0, 1
	);

	const Matrix4 g_opengl2radiant = Matrix4::byColumns(
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	);

	inline Matrix4 projection_for_camera(float near_z, float far_z, float fieldOfView, int width, int height)
	{
		const auto half_width = near_z * tan(degrees_to_radians(fieldOfView * 0.5));
		const auto half_height = half_width * (static_cast<double>(height) / static_cast<double>(width));

		return Matrix4::getProjectionForFrustum(
			-half_width,
			half_width,
			-half_height,
			half_height,
			near_z,
			far_z
		);
	}
}

Vector3 Camera::_prevOrigin(0,0,0);
Vector3 Camera::_prevAngles(0,0,0);

Camera::Camera(render::IRenderView& view, const Callback& queueDraw, const Callback& forceRedraw) :
	_origin(_prevOrigin), // Use previous origin for camera position
	_angles(_prevAngles),
	_queueDraw(queueDraw),
	_forceRedraw(forceRedraw),
	_fieldOfView(75.0f),
	_farClipPlane(32768),
	_width(0),
	_height(0),
	_projection(Matrix4::getIdentity()),
	_modelview(Matrix4::getIdentity()),
	_view(view)
{}

void Camera::updateModelview()
{
	_prevAngles = _angles;
	_prevOrigin = _origin;

	_modelview = Matrix4::getIdentity();

	// roll, pitch, yaw
	Vector3 radiant_eulerXYZ(0, -_angles[camera::CAMERA_PITCH], _angles[camera::CAMERA_YAW]);

	_modelview.translateBy(_origin);
	_modelview.rotateByEulerXYZDegrees(radiant_eulerXYZ);
	_modelview.multiplyBy(g_radiant2opengl);
	_modelview.invert();

	updateVectors();

	_view.construct(_projection, _modelview, _width, _height);
}

void Camera::updateVectors()
{
	for (int i = 0; i < 3; i++)
	{
		_vright[i] = _modelview[(i<<2)+0];
		_vup[i] = _modelview[(i<<2)+1];
		_vpn[i] = _modelview[(i<<2)+2];
	}
}

void Camera::freemoveUpdateAxes() 
{
	_right = _vright;
	_forward = -_vpn;
}

const Vector3& Camera::getCameraOrigin() const
{
	return _origin;
}

void Camera::setCameraOrigin(const Vector3& newOrigin)
{
	_origin = newOrigin;
	_prevOrigin = _origin;

	updateModelview();
	queueDraw();
	CameraManager::GetInstanceInternal().onCameraViewChanged();
}

const Vector3& Camera::getCameraAngles() const
{
	return _angles;
}

void Camera::setCameraAngles(const Vector3& newAngles)
{
	_angles = newAngles;
	_prevAngles = _angles;

	updateModelview();
	freemoveUpdateAxes();
	queueDraw();
	CameraManager::GetInstanceInternal().onCameraViewChanged();
}

const Vector3& Camera::getRightVector() const
{
	return _vright;
}

const Vector3& Camera::getUpVector() const
{
	return _vup;
}

const Vector3& Camera::getForwardVector() const
{
	return _vpn;
}

const Matrix4& Camera::getModelView() const
{
	return _modelview;
}

const Matrix4& Camera::getProjection() const
{
	return _projection;
}

int Camera::getDeviceWidth() const
{
	return _width;
}

int Camera::getDeviceHeight() const
{
	return _height;
}

void Camera::setDeviceDimensions(int width, int height)
{
	_width = width;
	_height = height;
	updateProjection();
}

SelectionTestPtr Camera::createSelectionTestForPoint(const Vector2& point)
{
	float selectEpsilon = registry::getValue<float>(RKEY_SELECT_EPSILON);

	// Get the mouse position
	Vector2 deviceEpsilon(selectEpsilon / getDeviceWidth(), selectEpsilon / getDeviceHeight());

	// Copy the current view and constrain it to a small rectangle
	render::View scissored(_view);

	auto rect = selection::Rectangle::ConstructFromPoint(point, deviceEpsilon);
	scissored.EnableScissor(rect.min[0], rect.max[0], rect.min[1], rect.max[1]);

	return SelectionTestPtr(new SelectionVolume(scissored));
}

const VolumeTest& Camera::getVolumeTest() const
{
	return _view;
}

void Camera::queueDraw()
{
	_queueDraw();
}

void Camera::forceRedraw()
{
	_forceRedraw();
}

void Camera::moveUpdateAxes()
{
	double ya = degrees_to_radians(_angles[camera::CAMERA_YAW]);

	// the movement matrix is kept 2d
	_forward[0] = static_cast<float>(cos(ya));
	_forward[1] = static_cast<float>(sin(ya));
	_forward[2] = 0;
	_right[0] = _forward[1];
	_right[1] = -_forward[0];
}

float Camera::getFarClipPlaneDistance() const
{
	return _farClipPlane;
}

void Camera::setFarClipPlaneDistance(float distance)
{
	_farClipPlane = distance;
	updateProjection();
}

void Camera::updateProjection()
{
	auto farClip = getFarClipPlaneDistance();
	_projection = projection_for_camera(farClip / 4096.0f, farClip, _fieldOfView, _width, _height);

	_view.construct(_projection, _modelview, _width, _height);
}

} // namespace