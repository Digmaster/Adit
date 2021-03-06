#include "PlayerCamera.h"

#include <osg/ObserverNodePath>

#include "Engine/GameEngine.h"
#include "Physics/PhysicsEngine.h"

#include "Character/Player.h"

float PlayerCamera::_minDistance = 2;
float PlayerCamera::_maxDistance = 20;

PlayerCamera::PlayerCamera(GameEngine* eng) : eng(eng), _distance(15.0), _firstPerson(false)
{
}

PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::update()
{

	osg::Vec3d eye;
	osg::Vec3d lookAt;

	//First Person: Eye is eye node, look at is a vector from the eye node
	//Third Person: Look at is eye node, eye is vector behind eye node

	if (_firstPerson)
	{
		eye = GameEngine::inst().getPlayer()->getEyePosition();
		osg::Vec3d rot = GameEngine::inst().getPlayer()->getRotation();

		double yaw = -rot.x() - osg::PI_2;
		double pitch = -rot.y();
		double x = -cos(yaw)*cos(pitch);
		double y = -sin(yaw)*cos(pitch);
		double z = -sin(pitch);

		lookAt = osg::Vec3d(eye.x() + x, eye.y() + y, eye.z() + z);
	}
	else
	{
		lookAt = osg::computeLocalToWorld(GameEngine::inst().getPlayer()->getEyeNode()->getParentalNodePaths().at(0)).getTrans();
		osg::Vec3d rot = GameEngine::inst().getPlayer()->getRotation();

		double yaw = -rot.x() - osg::PI_2;
		double pitch = -rot.y();
		double x = cos(yaw)*cos(pitch);
		double y = sin(yaw)*cos(pitch);
		double z = sin(pitch);

		eye = osg::Vec3d(lookAt.x() + x*_distance, lookAt.y() + y*_distance, lookAt.z() + z*_distance);

		eye = getBestCameraPosition(lookAt, eye);
	}

	GameEngine::inst().getViewer()->getCamera()->setViewMatrixAsLookAt(eye, lookAt, osg::Vec3d(0, 0, 1));
}

void PlayerCamera::setDistance(float dist)
{
	if (dist < _minDistance)
		dist = _minDistance;

	if (dist > _maxDistance)
		dist = _maxDistance;

	_distance = dist;

	if (dist <= _minDistance)
	{
		setFirstPerson(true);
	}
	else
	{
		setFirstPerson(false);
	}

}

void PlayerCamera::setFirstPerson(bool mode)
{
	if (mode == _firstPerson)
		return; //no need to do anything if already set up
	
	_firstPerson = mode;

	if (mode)
	{
		GameEngine::inst().getPlayer()->setFirstPerson(true);
		_distance = _minDistance; //move distance to min distance
	}
	else
	{
		GameEngine::inst().getPlayer()->setFirstPerson(false);
		_distance = _minDistance + 1.0;
	}
}

osg::Vec3d PlayerCamera::getBestCameraPosition(osg::Vec3d start, osg::Vec3d end)
{
	btVector3 btFrom = osgbCollision::asBtVector3(start);
	btVector3 btTo = osgbCollision::asBtVector3(end);
	btCollisionWorld::ClosestConvexResultCallback res(btFrom, btTo);
	res.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;

	btConvexShape* shape = new btSphereShape(0.25);

	btTransform startTransform(btMatrix3x3(btQuaternion(0, 0, 0, 1)), btFrom);
	btTransform endTransform(btMatrix3x3(btQuaternion(0, 0, 0, 1)), btTo);

	GameEngine::inst().getPhysics()->getWorld()->convexSweepTest(shape, startTransform, endTransform, res);

	if (res.hasHit())
	{
		return osgbCollision::asOsgVec3(res.m_hitPointWorld);
	}

	return end;
}
