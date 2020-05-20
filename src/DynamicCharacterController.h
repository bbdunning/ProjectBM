#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "LinearMath/btVector3.h"

#include "BulletDynamics/Character/btCharacterControllerInterface.h"

class btCollisionShape;
class btRigidBody;
class btCollisionWorld;

///DynamicCharacterController is obsolete/unsupported at the moment
class DynamicCharacterController 
{
protected:
	btScalar m_halfHeight;
	btCollisionShape* m_shape;
	btRigidBody* m_rigidBody;

	btVector3 m_raySource[2];
	btVector3 m_rayTarget[2];
	btScalar m_rayLambda[2];
	btVector3 m_rayNormal[2];
	btVector3 forwardDir;

	btScalar m_turnAngle;

	btScalar m_maxLinearVelocity;
	btScalar m_walkVelocity;
	btScalar m_turnVelocity;
public:
	DynamicCharacterController ();
	~DynamicCharacterController ();
	btVector3 getLocation();
	void setup (btScalar height = 2.0, btScalar width = 0.25, btScalar stepHeight = 0.25);
	void destroy ();

		void setWalkDirection(const btVector3& walkDirection);
		void setVelocityForTimeInterval(const btVector3& velocity, btScalar timeInterval);
	void reset ();
	void warp (const btVector3& origin);
	// virtual void registerPairCacheAndDispatcher (btOverlappingPairCache* pairCache, btCollisionDispatcher* dispatcher);

	btCollisionObject* getCollisionObject ();
	btRigidBody* getRigidBody();

	void preStep (const btCollisionWorld* collisionWorld);
	void playerStep (const btCollisionWorld* collisionWorld,btScalar dt,
					 int forward,
					 int backward,
					 int left,
					 int right,
					 int jump);
	bool canJump () const;
	void jump ();

	bool onGround () const;
	void setUpInterpolate(bool value);
	void setForwardDir(const btVector3 &fd);
};

#endif
