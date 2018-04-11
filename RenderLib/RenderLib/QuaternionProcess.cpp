#include "QuaternionProcess.h"

namespace QuaternionProcess
{
	Quaternion::Quaternion()
	{
		w_ = 0.0f;
		x_ = 0.0f;
		y_ = 0.0f;
		z_ = 0.0f;
	}

	Quaternion::~Quaternion()
	{
	}

	Quaternion::Quaternion(float w, float x, float y, float z)
	{
		w_ = w;
		x_ = x;
		y_ = y;
		z_ = z;
	}

	Quaternion& Quaternion::operator= (Quaternion &rq)
	{
		x_ = rq.x_;
		y_ = rq.y_;
		z_ = rq.z_;
		w_ = rq.w_;
		return *this;
	}

	Quaternion Quaternion::operator+ (const Quaternion &rq) const
	{
		return Quaternion(w_ + rq.w_,
			x_ + rq.x_,
			y_ + rq.y_,
			z_ + rq.z_);
	}

	Quaternion Quaternion::operator- (const Quaternion &rq) const
	{
		return Quaternion(w_ - rq.w_,
			x_ - rq.x_,
			y_ - rq.y_,
			z_ - rq.z_);
	}

	Quaternion Quaternion::operator* (const Quaternion &rq) const
	{
		return Quaternion(w_*rq.w_ - x_*rq.x_ - y_*rq.y_ - z_*rq.z_,
			w_*rq.x_ + rq.w_*x_ + y_*rq.z_ - rq.y_*z_,
			w_*rq.y_ + rq.w_*y_ + z_*rq.x_ - rq.z_*x_,
			w_*rq.z_ + rq.w_*z_ + x_*rq.y_ - rq.x_*y_);
	}

	void Quaternion::normalise()
	{
		// Don't normalize if we don't have to
		float mag2 = w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
		if (mag2 != 0.f && (fabs(mag2 - 1.0f) > TOLERANCE)) {
			float mag = sqrt(mag2);
			w_ /= mag;
			x_ /= mag;
			y_ /= mag;
			z_ /= mag;
		}
	}

	Quaternion& Quaternion::getConjugate()
	{
		Quaternion ConjugateQuat;
		ConjugateQuat = Quaternion(-x_, -y_, -z_, w_);
		return ConjugateQuat;
	}

	Vector3f Quaternion::operator* (const Vector3f &vec) const
	{
		Vector3f vn(vec);
		vn.normalise();

		Quaternion vecQuat;
		Quaternion ConjugateQuat;
		Quaternion resQuat;
		vecQuat.x_ = vn.x_;
		vecQuat.y_ = vn.y_;
		vecQuat.z_ = vn.z_;
		vecQuat.w_ = 0.0f;

		//ConjugateQuat = getConjugate();
		ConjugateQuat = Quaternion(w_, - x_, -y_, -z_);
		resQuat = vecQuat * ConjugateQuat;
		resQuat = *this * resQuat;

		return (Vector3f(resQuat.x_, resQuat.y_, resQuat.z_));
	}

	void Quaternion::FromAxis(const Vector3f &v, float angle)
	{
		float sinAngle;
		angle *= 0.5f;
		Vector3f vn(v);
		vn.normalise();

		sinAngle = sin(angle);

		x_ = (vn.x_ * sinAngle);
		y_ = (vn.y_ * sinAngle);
		z_ = (vn.z_ * sinAngle);
		w_ = cos(angle);
	}

	// Convert from Euler Angles
	void Quaternion::FromEuler(float yaw, float roll, float pitch, int radian_flag)
	{
		// Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
		// and multiply those together.
		// the calculation below does the same, just shorter
		// roll x
		// pitch y
		// yaw  z
		float y = 0.0f;
		float r = 0.0f;
		float p = 0.0f;

		if (radian_flag == 0)
		{
			y = (yaw / 2.0f) * PI / 180.0f;
			r = (roll / 2.0f) * PI / 180.0f;
			p = (pitch / 2.0f) * PI / 180.0f;
		}
		else
		{
			y = yaw / 2.0f;
			r = roll / 2.0f;
			p = pitch / 2.0f;
		}

		float sinp = sin(p);
		float siny = sin(y);
		float sinr = sin(r);
		float cosp = cos(p);
		float cosy = cos(y);
		float cosr = cos(r);

		this->x_ = sinr * cosp * cosy - cosr * sinp * siny;
		this->y_ = cosr * sinp * cosy + sinr * cosp * siny;
		this->z_ = cosr * cosp * siny - sinr * sinp * cosy;
		this->w_ = cosr * cosp * cosy + sinr * sinp * siny;

		normalise();
	}

	// Convert to Axis/Angles
	void Quaternion::getAxisAngle(Vector3f *axis, float *angle)
	{
		float scale = sqrt(x_ * x_ + y_ * y_ + z_ * z_);
		axis->x_ = x_ / scale;
		axis->y_ = y_ / scale;
		axis->z_ = z_ / scale;
		*angle = acos(w_) * 2.0f;
	}

	void Quaternion::GetXYZAxisAngleByQuaternion(float &xangle, float &yangle, float zangle)
	{
		float t11 = w_*w_ + x_*x_ - y_*y_ - z_*z_;
		float t12 = 2 * (x_*y_ - w_*z_);
		float t13 = 2 * (x_*z_ + w_*y_);
		float t21 = 2 * (x_*y_ + w_*z_);
		float t22 = w_*w_ - x_*x_ + y_*y_ - z_*z_;
		float t23 = 2 * (y_*z_ - w_*x_);
		float t31 = 2 * (x_*z_ - w_*y_);
		float t32 = 2 * (y_*z_ + w_*x_);
		float t33 = w_*w_ - x_*x_ - y_*y_ + z_*z_;
		xangle = asin(t32);
		yangle = atan(-1.0f*t31 / t33);
		zangle = atan(t12 / t22);
	}

	Quaternion Quaternion::GetQRotationResult(const Quaternion &rq)
	{
		Quaternion ResultQuat;
		Quaternion TempQuat;
		Quaternion RotateConjugateQuat(w_, -x_, -y_, -z_);
		TempQuat = rq * RotateConjugateQuat;
		ResultQuat = *this * TempQuat;
		return ResultQuat;
	}

	void Quaternion::SetQuaternionW(float w)
	{
		w_ = w;
	}

	void Quaternion::SetQuaternionX(float x)
	{
		x_ = x;
	}

	void Quaternion::SetQuaternionY(float y)
	{
		y_ = y;
	}

	void Quaternion::SetQuaternionZ(float z)
	{
		z_ = z;
	}

	void Quaternion::SetQuaternionElement(float w, float x, float y, float z)
	{
		w_ = w;
		x_ = x;
		y_ = y;
		z_ = z;
	}

	float Quaternion::GetQuaternionW()
	{
		return w_;
	}
	float Quaternion::GetQuaternionX()
	{
		return x_;
	}
	float Quaternion::GetQuaternionY()
	{
		return y_;
	}
	float Quaternion::GetQuaternionZ()
	{
		return z_;
	}
};