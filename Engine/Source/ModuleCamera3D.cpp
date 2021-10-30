#include "Application.h"
#include "ModuleCamera3D.h"

#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MathGeolib/src/Math/float4.h"

#include "Optick/include/optick.h"

#include "mmgr/mmgr.h"

ModuleCamera3D::ModuleCamera3D(bool startEnabled) : Module(startEnabled)
{
	name = "Camera3D";

	cameraFrustum.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	cameraFrustum.SetViewPlaneDistances(1.0f, 5.0f);
	cameraFrustum.SetPerspective(90.0f, 60.0f);
	cameraFrustum.SetFrame(float3(0.0f, 1.5f, 5.0f), float3(0.0f, 0.0f, -1.0f), float3(0.0f, 1.0f, 0.0f));
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	DEBUG_LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	DEBUG_LOG("Cleaning camera");

	return true;
}

bool ModuleCamera3D::LoadConfig(JsonParsing& node)
{
	return true;
}

bool ModuleCamera3D::SaveConfig(JsonParsing& node) const
{
	return true;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::Update(float dt)
{
	float3 newPos = cameraFrustum.Pos();
	float3 newFront = cameraFrustum.Front();
	float3 newUp = cameraFrustum.Up();
	float speed = 9.0f * dt;

	if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_REPEAT)
		speed *= 2;

	float dX = -app->input->GetMouseXMotion();
	float dY = -app->input->GetMouseYMotion();

	// Inputs for the camera
	if (app->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::KEY_REPEAT)
	{
		if (app->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT) newPos += cameraFrustum.Front() * speed;
		if (app->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT) newPos -= cameraFrustum.Front() * speed;

		if (app->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT) newPos -= cameraFrustum.WorldRight() * speed;
		if (app->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT) newPos += cameraFrustum.WorldRight() * speed;

		if (app->input->GetKey(SDL_SCANCODE_Q) == KeyState::KEY_REPEAT) newPos += cameraFrustum.Up() * speed;
		if (app->input->GetKey(SDL_SCANCODE_E) == KeyState::KEY_REPEAT) newPos -= cameraFrustum.Up() * speed;

		GameObject* target = app->editor->GetSelected();

		if (dY != 0)
		{
			Quat rotateVertical;
			rotateVertical = rotateVertical.RotateAxisAngle(cameraFrustum.WorldRight().Normalized(), -dY * dt);
			newFront = rotateVertical * newFront;
			newUp = rotateVertical * newUp;
			newFront.Normalize();
			newUp.Normalize();
			float3::Orthonormalize(newFront, newUp);
		}
		if (dX != 0)
		{
			Quat rotateHorizontal;
			rotateHorizontal = rotateHorizontal.RotateY(-dX * dt);
			newFront = rotateHorizontal * newFront;
			newUp = rotateHorizontal * newUp;
			newFront.Normalize();
			newUp.Normalize();
			float3::Orthonormalize(newFront, newUp);
		}
	}

	if (app->input->GetMouseButton(SDL_BUTTON_LEFT) == KeyState::KEY_REPEAT && 
		app->input->GetKey(SDL_SCANCODE_LALT) == KeyState::KEY_REPEAT)
	{
		GameObject* target = app->editor->GetSelected();
		if (target != nullptr)
		{
			float3 distanceTarget = cameraFrustum.Pos() - target->GetComponent<TransformComponent>()->GetPosition();

			Quat rotateOrbitY;
			rotateOrbitY = rotateOrbitY.RotateY(-dX * dt);
			//distanceTarget = rotateOrbitY * distanceTarget;

			Quat rotateOrbitX;
			rotateOrbitX = rotateOrbitX.RotateAxisAngle(cameraFrustum.WorldRight().Normalized(), -dY * dt);
			newUp = rotateOrbitY * rotateOrbitX * newUp;
			newUp.Normalize();
			distanceTarget = rotateOrbitY * rotateOrbitX * distanceTarget;
			newFront = distanceTarget.Normalized().Neg();
			newPos = distanceTarget;
			float3::Orthonormalize(newFront, newUp);
		}
	}

	if (app->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_UP)
	{
		GameObject* target = app->editor->GetSelected();
		if (target != nullptr)
		{
			float3 maxPoint = target->GetAABB().maxPoint;
			float3 minPoint = target->GetAABB().minPoint;
			
			float3 h = (maxPoint - minPoint) / 2.0f;

			float angle = DegToRad(cameraFrustum.VerticalFov()) / 2;

			float3 distance = h / Tan(angle);

			distance.x = (distance.x + 2.5f) * cameraFrustum.Front().x;
			distance.y = distance.y * cameraFrustum.Front().y;
			distance.z = (distance.z + 2.5f) * cameraFrustum.Front().z;
			newPos = target->GetAABB().CenterPoint() - distance;
		}
	}
	
	if (app->input->GetMouseZ() == 1) newPos.z -= 9.0f * dt;
	if (app->input->GetMouseZ() == -1) newPos.z += 9.0f * dt;

	cameraFrustum.SetFrame(newPos, newFront, newUp);

	matrixViewFrustum = cameraFrustum.ViewMatrix();
	matrixProjectionFrustum = cameraFrustum.ProjectionMatrix();
	

	return true;
}

void ModuleCamera3D::LookAt(float3& target)
{
	float3 directionFrustum = target - cameraFrustum.Pos();
	directionFrustum.Normalize();

	float3x3 lookAt = float3x3::LookAt(cameraFrustum.Front(), directionFrustum, cameraFrustum.Up(), float3(0.0f, 1.0f, 0.0f));
	cameraFrustum.SetFront(lookAt.MulDir(cameraFrustum.Front()).Normalized());
	cameraFrustum.SetUp(lookAt.MulDir(cameraFrustum.Up()).Normalized());
}