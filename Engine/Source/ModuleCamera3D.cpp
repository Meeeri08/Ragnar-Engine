#include "Application.h"
#include "Globals.h"
#include "ModuleCamera3D.h"

#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "GameObject.h"
#include "ModuleScene.h"

#include "SDL.h"
#include "Profiling.h"


ModuleCamera3D::ModuleCamera3D(bool startEnabled) : horizontalFov(DegToRad(90.0f)), verticalFov(0.0f), nearPlane(0.1f), farPlane(100.0f), Module(startEnabled)
{
	name = "Camera3D";

	cameraFrustum.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	cameraFrustum.SetViewPlaneDistances(nearPlane, farPlane);
	CalculateVerticalFov(horizontalFov, SCREEN_WIDTH, SCREEN_HEIGHT);
	cameraFrustum.SetPerspective(horizontalFov, verticalFov);
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

void ModuleCamera3D::CalculateVerticalFov(float horizontalFovRadians, float width, float height)
{
	verticalFov = 2 * Atan((Tan(horizontalFovRadians / 2)) * (height / width));
	cameraFrustum.SetVerticalFovAndAspectRatio(verticalFov, (width / height));
	currentScreenHeight = height;
	currentScreenWidth = width;
}

void ModuleCamera3D::UpdateFovAndScreen(float width, float height)
{
	verticalFov = 2 * Atan((Tan(horizontalFov / 2)) * (height / width));
	cameraFrustum.SetVerticalFovAndAspectRatio(verticalFov, (width / height));
	currentScreenHeight = height;
	currentScreenWidth = width;
}

void ModuleCamera3D::UpdateFov()
{
	verticalFov = 2 * Atan((Tan(horizontalFov / 2)) * (currentScreenHeight / currentScreenWidth));
	cameraFrustum.SetVerticalFovAndAspectRatio(verticalFov, (currentScreenWidth / currentScreenHeight));
}

bool ModuleCamera3D::LoadConfig(JsonParsing& node)
{
	//float3 pos;
	//float3 front;
	//float3 up;

	//pos.x = node.GetJsonNumber("PosX");
	//pos.y = node.GetJsonNumber("PosY");
	//pos.z = node.GetJsonNumber("PosZ");

	//front.x = node.GetJsonNumber("FrontX");
	//front.y = node.GetJsonNumber("FrontY");
	//front.z = node.GetJsonNumber("FrontZ");

	//up.x = node.GetJsonNumber("UpX");
	//up.y = node.GetJsonNumber("UpY");
	//up.z = node.GetJsonNumber("UpZ");

	//cameraFrustum.SetFrame(pos, front, up);
	//matrixViewFrustum = cameraFrustum.ViewMatrix();
	//matrixProjectionFrustum = cameraFrustum.ProjectionMatrix();

	return true;
}

bool ModuleCamera3D::SaveConfig(JsonParsing& node) const
{
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "PosX", cameraFrustum.Pos().x);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "PosY", cameraFrustum.Pos().y);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "PosZ", cameraFrustum.Pos().z);

	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "FrontX", cameraFrustum.Front().x);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "FrontY", cameraFrustum.Front().y);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "FrontZ", cameraFrustum.Front().z);

	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "UpX", cameraFrustum.Up().x);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "UpY", cameraFrustum.Up().y);
	//node.SetNewJsonNumber(node.ValueToObject(node.GetRootValue()), "UpZ", cameraFrustum.Up().z);

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
	if (app->input->GetKey(SDL_SCANCODE_T) == KeyState::KEY_UP)
	{
		newUp = float3::unitY;
		newFront = -float3::unitZ;
	}
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
			rotateHorizontal = rotateHorizontal.RotateY(dX * dt);
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
			float3 targetPos = {};
			Quat targetRot = {};
			float3 targetSize = {};
			target->GetComponent<TransformComponent>()->GetGlobalTransform().Decompose(targetPos, targetRot, targetSize);
			float3 distanceTarget = cameraFrustum.Pos() - targetPos;

			Quat rotateOrbitY;
			rotateOrbitY = rotateOrbitY.RotateY(-dX * dt);
			rotateOrbitY.Normalize();
			//distanceTarget = rotateOrbitY * distanceTarget;

			Quat rotateOrbitX;
			rotateOrbitX = rotateOrbitX.RotateAxisAngle(cameraFrustum.WorldRight().Normalized(), -dY * dt);
			rotateOrbitX.Normalize();
			newUp =  rotateOrbitX * newUp;
			newUp.Normalize();
			newUp = rotateOrbitY * newUp;
			newUp.Normalize();
			distanceTarget = rotateOrbitX * rotateOrbitY * distanceTarget;
			newFront = distanceTarget.Normalized().Neg();
			newPos = distanceTarget + targetPos;
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
	float4 size = app->editor->GetViewport()->GetBounds();
	float2 pos(app->input->GetMouseX(), app->input->GetMouseY());
	if (pos.x > size.x && pos.x < size.x + size.z && pos.y > size.y && pos.y < size.y + size.w)
	{
		if (app->input->GetMouseZ() == 1) newPos += newFront * speed;
		if (app->input->GetMouseZ() == -1) newPos -= newFront * speed;

		if (app->input->GetMouseButton(SDL_BUTTON_LEFT) == KeyState::KEY_UP)
		{
			float2 mousePos = { (float)app->input->GetMouseX(), (float)app->input->GetMouseY() };
			
			mousePos.x -= size.x;
			mousePos.y -= size.y;
			mousePos.x = -1.0f + 2.0f * mousePos.x / size.z;
			mousePos.y = 1.0f - 2.0f * mousePos.y / size.w;
			LineSegment picking = cameraFrustum.UnProjectLineSegment(mousePos.x, mousePos.y);
			Ray rayCast = picking.ToRay();
			DEBUG_LOG("POSITION X %f, POSITION Y %f", mousePos.x, mousePos.y);
			bool hit = false;

			std::vector<GameObject*> gameObjects;
			app->scene->GetQuadtree().CollectGo(gameObjects);

			std::vector<GameObject*>::iterator it = gameObjects.begin();
			for (; it < gameObjects.end(); ++it)
			{
				TransformComponent* transform = (*it)->GetComponent<TransformComponent>();
				if ((*it)->GetAABB().IsFinite() && transform)
				{

					rayCast.Transform(transform->GetGlobalTransform().Transposed());
					//OBB asObb = (*it)->GetAABB().Transform((*it)->GetComponent<TransformComponent>()->GetTransform());
					hit = rayCast.Intersects((*it)->GetAABB());
					
					if (hit)
					{
						app->editor->SetSelected((*it));
						DEBUG_LOG("HITTED IS %s", (*it)->GetName());
					}
				}
			}
		}
	}
	cameraFrustum.SetFrame(newPos, newFront, newUp);
	
	//matrixViewFrustum = cameraFrustum.ViewMatrix();
	matrixProjectionFrustum = cameraFrustum.ComputeProjectionMatrix();
	matrixViewFrustum = cameraFrustum.ComputeViewMatrix();

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

int ModuleCamera3D::ContainsAaBox(const AABB& boundingBox)
{
	
	if (boundingBox.IsFinite())
	{
		if (cameraFrustum.Contains(boundingBox))
		{
			return 1;

		}
		else if (cameraFrustum.Intersects(boundingBox))
		{
			return 2;
		}
		return 0;
	}
	return -1;
}
