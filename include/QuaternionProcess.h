#ifndef _QUATERNION_PROCESS_H_
#define _QUATERNION_PROCESS_H_
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
using namespace std;
namespace QuaternionProcess
{
	typedef struct Vector3Obj
	{
		float x_;
		float y_;
		float z_;
		Vector3Obj(float x, float y, float z)
		{
			x_ = x;
			y_ = y;
			z_ = z;
		}
		void normalise()
		{
			float length_ = sqrt(x_*x_ + y_*y_ + z_*z_);
			x_ = x_ / length_;
			y_ = y_ / length_;
			z_ = z_ / length_;
		}
	}Vector3f;

	class Quaternion
	{
	public:
		Quaternion();
		~Quaternion();
		Quaternion(float w_, float x_, float y_, float z_);

		//Basic Operation
		Quaternion operator+ (const Quaternion &rq) const;
		Quaternion operator- (const Quaternion &rq) const;
		Quaternion operator* (const Quaternion &rq) const;
		Vector3f operator* (const Vector3f &vec) const;
		Quaternion& operator= (Quaternion &rq);
		//Quaternion normalise
		void normalise();
		//Get the conjugate
		Quaternion& getConjugate();
		// Convert from Axis Angle
		void FromAxis(const Vector3f &v, float angle);
		// Convert from Euler Angles
		void FromEuler(float yaw, float roll, float pitch, int radian_flag);
		// Convert to Axis/Angles
		void getAxisAngle(Vector3f *axis, float *angle);
		// Get each angle around xyz axis by the calced Quaternion
		void GetXYZAxisAngleByQuaternion(float &xangle, float &yangle, float &zangle);
		// GetRotationResult in the form of Quaternion
		Quaternion GetQRotationResult(const Quaternion &rq2);
		// Set the Quaternion element
		void SetQuaternionW(float w);
		void SetQuaternionX(float x);
		void SetQuaternionY(float y);
		void SetQuaternionZ(float z);
		void SetQuaternionElement(float w, float x, float y, float z);
		// Get the Quaternion Result
		float GetQuaternionW();
		float GetQuaternionX();
		float GetQuaternionY();
		float GetQuaternionZ();
	private:
		float w_;
		float x_;
		float y_;
		float z_;
	};
};
#endif // !_QUATERNION_PROCESS_H_
