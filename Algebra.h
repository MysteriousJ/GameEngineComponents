#ifndef GOBLIN_ALGEBRA_HEADER
#define GOBLIN_ALGEBRA_HEADER

#include <math.h>
// Windows defines these, but we'll use our own for portability
#undef min
#undef max

namespace goblin {

const float pi = 3.14159265f;

float square(float x);
float lerp(float a, float b, float t);
float inverseLerp(float a, float b, float t);
float min(float a, float b);
float max(float a, float b);
float clamp(float x, float min, float max);
float round(float x);
float moveTowards(float current, float target, float maxChange);
float radianDifference(float angle1, float angle2); // UNTESTED

struct Vec2
{
	float x, y;
};

float length(Vec2 v);
float lengthSq(Vec2 v); // Length squared, before square root
Vec2  normalize(Vec2 v);
float dot(Vec2 a, Vec2 b);

bool operator==  (const Vec2& v1, const Vec2& v2);
Vec2 operator+   (const Vec2& v1, const Vec2& v2);
void operator+=  (Vec2& v1, const Vec2& v2);
Vec2 operator-   (const Vec2& v1, const Vec2& v2);
void operator-=  (Vec2& v1, const Vec2& v2);
Vec2 operator*   (const Vec2& v, float s);
void operator*=  (Vec2& v, float s);


struct Vec3
{
	union {
		struct {float x, y, z;};
		struct {Vec2 xy;};
		struct {float _ignored; Vec2 yz;};
		struct {float r, g, b;};
	};
};

float length(Vec3 v);
float lengthSq(Vec3 v); // Length squared, before square root
Vec3  normalize(Vec3 v);
float dot(Vec3 a, Vec3 b);
Vec3  cross(Vec3 a, Vec3 b);
Vec3  lerp(Vec3 a, Vec3 b, float t);
Vec3  project(Vec3 from, Vec3 onto);

// Component-wise operations
bool operator==  (const Vec3& v1, const Vec3& v2);
Vec3 operator+   (const Vec3& v1, const Vec3& v2);
void operator+=  (Vec3& v1, const Vec3& v2);
Vec3 operator-   (const Vec3& v);
Vec3 operator-   (const Vec3& v1, const Vec3& v2);
void operator-=  (Vec3& v1, const Vec3& v2);
Vec3 operator*   (Vec3 a, Vec3 b);
Vec3 operator*   (const Vec3& v, float s);
Vec3 operator*   (float s, const Vec3& v);
void operator*=  (Vec3& v, float s);
Vec3 operator/   (const Vec3& v, float s);


struct Vec4
{
	union {
		struct {float x, y, z, w;};
		struct {Vec3 xyz;};
		struct {float _ignored0; Vec3 yzw;};
		struct {Vec2 xy, zw;};
		struct {float _ignored1; Vec2 yz;};
		struct {float r, g, b, a;};
		struct {Vec3 rgb;};
	};

	float& operator[](int index);
};

Vec4 operator*   (const Vec4& v, float s);
void operator*=  (Vec4& v, float s);


// Row major order
// Multiplies with column vetors, so the order is mat3*mat2*mat1*vec
struct Matrix4x4
{
	float c[4][4]; // 'c' for 'cell'

	static const Matrix4x4 identity;
	const float* operator[](int index) const;
	float* operator[](int index);
};

Matrix4x4 transpose(Matrix4x4 m);
// Inverts any marix
Matrix4x4 inverse(Matrix4x4 m);
// Inverts only rotation-translation matrices. Fast, but does not work with a scaling factor.
Matrix4x4 inversePosRot(Matrix4x4 m);

Matrix4x4 makePerspectiveProjectionMatrix(float fieldOfViewRadians, float width, float height, float nearClip, float farClip);
Matrix4x4 makeOrthographicProjectionMatrix(float zoom, float width, float height, float nearClip, float farClip);

Matrix4x4 operator* (const Matrix4x4& a, const Matrix4x4& b);
Vec4 operator* (const Matrix4x4& m, const Vec3& v); // w is set to 1
Vec4 operator* (const Matrix4x4& m, const Vec4& v);


// Multiplication order goes quat3*quat2*quat1*vec
struct Quaternion
{
	union {
		struct {float w, x, y, z;};
		struct {float _ignored; Vec3 xyz;};
	};

	static const Quaternion identity;
};

float length(Quaternion q);
Quaternion inverse(Quaternion q);
Quaternion normalize(Quaternion q);
Quaternion lerp(Quaternion a, Quaternion b, float t);
float dot(Quaternion a, Quaternion b);
Quaternion rotateTowards(Quaternion a, Quaternion b, float maxRadians);
float radianDifference(Quaternion angle1, Quaternion angle2);

Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
Quaternion operator*(const Quaternion& q, float s);
Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
Vec3 operator*(const Quaternion& q, const Vec3& v);


struct Transform
{
	Vec3 position;
	Quaternion rotation;
	Vec3 scale;

	static const Transform identity;
};

Transform lerp(Transform a, Transform b, float t);
Transform concatenateTransforms(Transform parent, Transform child);

// Conversions
Vec3 vec3ToEulerXZ(Vec3 orientedVector);
Quaternion eulerZXYToQuaternion(Vec3 eulerAngles);
Quaternion axisAngleToQuaternion(Vec3 axis, float radians);
Quaternion vec3ToQuaternion(Vec3 lookRotation, Vec3 forward, Vec3 up); // TODO: Not yet implemented
Matrix4x4 eulerZXYToMatrix4x4(Vec3 eulerAngles);
Matrix4x4 quaternionToMatrix4x4(const Quaternion& q);
Matrix4x4 transformToMatrix4x4(const Transform& t);
// Same result as inverse(transformToMatrix4x4(t)), but skips computing a matrix inverse
Matrix4x4 transformToMatrix4x4Inverse(const Transform& t);


/* Implementation */

float square(float x)
{return x*x;}

float lerp(float a, float b, float t)
{return (1-t)*a + t*b;}

float inverseLerp(float a, float b, float t)
{return (t-a)/(b-a);}

float min(float a, float b)
{return a<b? a : b;}

float max(float a, float b)
{return a>b? a : b;}

float clamp(float x, float min, float max)
{return x<min? (min) : (x>max? max : x);}

float round(float x)
{return x>0.0f ? floorf(x+0.5f) : ceilf(x-0.5f);}

float moveTowards(float current, float target, float maxChange)
{return current<target ? min(target, current+maxChange) : max(target, current-maxChange);}

float radianDifference(float angle1, float angle2)
{return fmod(angle2, 2*pi) - fmod(angle1, 2*pi);}

// Vec2 =======================================================================

float length(Vec2 v)
{
	return sqrtf(lengthSq(v));
}

float lengthSq(Vec2 v)
{
	return v.x*v.x + v.y*v.y;
}

Vec2 normalize(Vec2 v)
{
	Vec2 result = {0};
	float r = length(v);
	if (r != 0.0) {
		result.x = v.x/r;
		result.y = v.y/r;
	}
	return result;
}

float dot(Vec2 a, Vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

// Vec 2 and Vec 2
bool operator==(const Vec2& v1, const Vec2& v2)
{return (v1.x == v2.x) && (v1.y == v2.y);}

Vec2 operator+(const Vec2& v1, const Vec2& v2)
{Vec2 result = {v1.x + v2.x, v1.y + v2.y}; return result;}

void operator+=(Vec2& v1, const Vec2& v2)
{v1 = v1 + v2;}

Vec2 operator-(const Vec2& v1, const Vec2& v2)
{Vec2 result = {v1.x - v2.x, v1.y - v2.y}; return result;}

void operator-=(Vec2& v1, const Vec2& v2)
{v1 = v1 - v2;}

// Vec 2 and scalar
Vec2 operator*(const Vec2& v, float s) 
{Vec2 result = {v.x*s, v.y*s}; return result;}

void operator*=(Vec2& v, float s)
{v = v*s;}



// Vec3 =======================================================================

float length(Vec3 v)
{
	return sqrtf(lengthSq(v));
}

float lengthSq(Vec3 v)
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

Vec3 normalize(Vec3 v)
{
	float r = length(v);
	if (r != 0) {
		Vec3 result = {v.x/r, v.y/r, v.z/r};
		return result;
	}
	Vec3 result = {0};
	return result;
}

float dot(Vec3 a, Vec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross(Vec3 a, Vec3 b)
{
	Vec3 result = {
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	};
	return result;
}

Vec3 lerp(Vec3 a, Vec3 b, float t)
{
	return a*(1-t) + b*t;
}

Vec3 project(Vec3 from, Vec3 onto)
{
	return dot(from, onto) / dot(onto, onto) * onto;
}

// Vec3 and Vec3
bool operator==(const Vec3& v1, const Vec3& v2)
{return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);}

Vec3 operator+(const Vec3& v1, const Vec3& v2)
{Vec3 result = {v1.x+v2.x, v1.y+v2.y, v1.z+v2.z}; return result;}

void operator+=(Vec3& v1, const Vec3& v2)
{v1 = v1+v2;}

Vec3 operator-(const Vec3& v)
{ return -1*v; }

Vec3 operator-(const Vec3& v1, const Vec3& v2)
{Vec3 result = {v1.x-v2.x, v1.y-v2.y, v1.z-v2.z}; return result;}

void operator-=(Vec3& v1, const Vec3& v2)
{v1 = v1-v2;}

Vec3 operator*(Vec3 a, Vec3 b)
{Vec3 result = {a.x*b.x, a.y*b.y, a.z*b.z}; return result;}

// Vec3 and scalar
Vec3 operator*(const Vec3& v, float s)
{Vec3 result = {v.x*s, v.y*s, v.z*s}; return result;}

Vec3 operator*(float s, const Vec3& v)
{return v*s;}

void operator*=(Vec3& v, float s)
{v = v*s;}

Vec3 operator/(const Vec3& v, float s)
{Vec3 result = {v.x/s, v.y/s, v.z/s}; return result;}

// Vec4 =======================================================================

float& Vec4::operator[](int index)
{
	assert(index >= 0 || index <= 3);
	return *(&x+index);
}

// Vec4 and scaler
Vec4 operator* (const Vec4& v, float s)
{Vec4 result = {v.x*s, v.y*s, v.z*s, v.w*s}; return result;}

void operator*= (Vec4& v, float s)
{v = v*s;}

// Matrix4x4 ==================================================================
const Matrix4x4 Matrix4x4::identity = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

// C is a 2-dimensional array, so we return a pointer to the column and the user dereferences it again, giving [a][b] syntax.
const float* Matrix4x4::operator[](int index) const
{return c[index];}

float* Matrix4x4::operator[](int index)
{return c[index];}

Matrix4x4 inverse(Matrix4x4 m)
{
	// From http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
	Matrix4x4 r;
	r[0][0] = m[1][2]*m[2][3]*m[3][1] - m[1][3]*m[2][2]*m[3][1] + m[1][3]*m[2][1]*m[3][2]
	        - m[1][1]*m[2][3]*m[3][2] - m[1][2]*m[2][1]*m[3][3] + m[1][1]*m[2][2]*m[3][3];
	r[0][1] = m[0][3]*m[2][2]*m[3][1] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2]
	        + m[0][1]*m[2][3]*m[3][2] + m[0][2]*m[2][1]*m[3][3] - m[0][1]*m[2][2]*m[3][3];
	r[0][2] = m[0][2]*m[1][3]*m[3][1] - m[0][3]*m[1][2]*m[3][1] + m[0][3]*m[1][1]*m[3][2]
	        - m[0][1]*m[1][3]*m[3][2] - m[0][2]*m[1][1]*m[3][3] + m[0][1]*m[1][2]*m[3][3];
	r[0][3] = m[0][3]*m[1][2]*m[2][1] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2] 
	        + m[0][1]*m[1][3]*m[2][2] + m[0][2]*m[1][1]*m[2][3] - m[0][1]*m[1][2]*m[2][3];
	r[1][0] = m[1][3]*m[2][2]*m[3][0] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2]
	        + m[1][0]*m[2][3]*m[3][2] + m[1][2]*m[2][0]*m[3][3] - m[1][0]*m[2][2]*m[3][3];
	r[1][1] = m[0][2]*m[2][3]*m[3][0] - m[0][3]*m[2][2]*m[3][0] + m[0][3]*m[2][0]*m[3][2]
	        - m[0][0]*m[2][3]*m[3][2] - m[0][2]*m[2][0]*m[3][3] + m[0][0]*m[2][2]*m[3][3];
	r[1][2] = m[0][3]*m[1][2]*m[3][0] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2]
	        + m[0][0]*m[1][3]*m[3][2] + m[0][2]*m[1][0]*m[3][3] - m[0][0]*m[1][2]*m[3][3];
	r[1][3] = m[0][2]*m[1][3]*m[2][0] - m[0][3]*m[1][2]*m[2][0] + m[0][3]*m[1][0]*m[2][2]
	        - m[0][0]*m[1][3]*m[2][2] - m[0][2]*m[1][0]*m[2][3] + m[0][0]*m[1][2]*m[2][3];
	r[2][0] = m[1][1]*m[2][3]*m[3][0] - m[1][3]*m[2][1]*m[3][0] + m[1][3]*m[2][0]*m[3][1]
	        - m[1][0]*m[2][3]*m[3][1] - m[1][1]*m[2][0]*m[3][3] + m[1][0]*m[2][1]*m[3][3];
	r[2][1] = m[0][3]*m[2][1]*m[3][0] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1]
	        + m[0][0]*m[2][3]*m[3][1] + m[0][1]*m[2][0]*m[3][3] - m[0][0]*m[2][1]*m[3][3];
	r[2][2] = m[0][1]*m[1][3]*m[3][0] - m[0][3]*m[1][1]*m[3][0] + m[0][3]*m[1][0]*m[3][1]
	        - m[0][0]*m[1][3]*m[3][1] - m[0][1]*m[1][0]*m[3][3] + m[0][0]*m[1][1]*m[3][3];
	r[2][3] = m[0][3]*m[1][1]*m[2][0] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1]
	        + m[0][0]*m[1][3]*m[2][1] + m[0][1]*m[1][0]*m[2][3] - m[0][0]*m[1][1]*m[2][3];
	r[3][0] = m[1][2]*m[2][1]*m[3][0] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1]
	        + m[1][0]*m[2][2]*m[3][1] + m[1][1]*m[2][0]*m[3][2] - m[1][0]*m[2][1]*m[3][2];
	r[3][1] = m[0][1]*m[2][2]*m[3][0] - m[0][2]*m[2][1]*m[3][0] + m[0][2]*m[2][0]*m[3][1]
	        - m[0][0]*m[2][2]*m[3][1] - m[0][1]*m[2][0]*m[3][2] + m[0][0]*m[2][1]*m[3][2];
	r[3][2] = m[0][2]*m[1][1]*m[3][0] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1]
	        + m[0][0]*m[1][2]*m[3][1] + m[0][1]*m[1][0]*m[3][2] - m[0][0]*m[1][1]*m[3][2];
	r[3][3] = m[0][1]*m[1][2]*m[2][0] - m[0][2]*m[1][1]*m[2][0] + m[0][2]*m[1][0]*m[2][1]
	        - m[0][0]*m[1][2]*m[2][1] - m[0][1]*m[1][0]*m[2][2] + m[0][0]*m[1][1]*m[2][2];

	float determinant 
		= m[0][3]*m[1][2]*m[2][1]*m[3][0] - m[0][2]*m[1][3]*m[2][1]*m[3][0]
		- m[0][3]*m[1][1]*m[2][2]*m[3][0] + m[0][1]*m[1][3]*m[2][2]*m[3][0]
		+ m[0][2]*m[1][1]*m[2][3]*m[3][0] - m[0][1]*m[1][2]*m[2][3]*m[3][0]
		- m[0][3]*m[1][2]*m[2][0]*m[3][1] + m[0][2]*m[1][3]*m[2][0]*m[3][1]
		+ m[0][3]*m[1][0]*m[2][2]*m[3][1] - m[0][0]*m[1][3]*m[2][2]*m[3][1]
		- m[0][2]*m[1][0]*m[2][3]*m[3][1] + m[0][0]*m[1][2]*m[2][3]*m[3][1]
		+ m[0][3]*m[1][1]*m[2][0]*m[3][2] - m[0][1]*m[1][3]*m[2][0]*m[3][2]
		- m[0][3]*m[1][0]*m[2][1]*m[3][2] + m[0][0]*m[1][3]*m[2][1]*m[3][2]
		+ m[0][1]*m[1][0]*m[2][3]*m[3][2] - m[0][0]*m[1][1]*m[2][3]*m[3][2]
		- m[0][2]*m[1][1]*m[2][0]*m[3][3] + m[0][1]*m[1][2]*m[2][0]*m[3][3]
		+ m[0][2]*m[1][0]*m[2][1]*m[3][3] - m[0][0]*m[1][2]*m[2][1]*m[3][3]
		- m[0][1]*m[1][0]*m[2][2]*m[3][3] + m[0][0]*m[1][1]*m[2][2]*m[3][3];

	r[0][0] *= 1/determinant;
	r[1][1] *= 1/determinant;
	r[2][2] *= 1/determinant;
	r[3][3] *= 1/determinant;

	return r;
}

Matrix4x4 inversePosRot(Matrix4x4 m)
{
	Matrix4x4 rotationInverse = transpose(m);
	// Clear translation, leaving it a pure rotation matrix
	rotationInverse[3][0] = 0;
	rotationInverse[3][1] = 0;
	rotationInverse[3][2] = 0;

	Vec3 translation = {m[0][3], m[1][3], m[2][3]};
	Vec3 translationInverse = ((rotationInverse)*translation*-1).xyz;
	Matrix4x4 result = rotationInverse;
	result[0][3] = translationInverse.x;
	result[1][3] = translationInverse.y;
	result[2][3] = translationInverse.z;

	return result;
}

Matrix4x4 transpose(Matrix4x4 m)
{
	Matrix4x4 r ={
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	};
	return r;
}

Matrix4x4 makePerspectiveProjectionMatrix(float fieldOfViewRadians, float width, float height, float nearClip, float farClip)
{
	// Based on glFrustum()
	float tangent = tanf(fieldOfViewRadians / 2.0f);
	float heightRatio = nearClip * tangent;
	float widthRatio = heightRatio*width/height;
	float left = -widthRatio;
	float right = widthRatio;
	float bottom = -heightRatio;
	float top = heightRatio;
	Matrix4x4 projection ={
		(2*nearClip)/(right-left), 0, 0, 0,
		0, (2*nearClip)/(top-bottom), 0, 0,
		0, 0, -(farClip+nearClip)/(farClip-nearClip), -(2*farClip*nearClip)/(farClip-nearClip),
		0, 0, -1, 0
	};
	return projection;
}

Matrix4x4 makeOrthographicProjectionMatrix(float zoom, float width, float height, float nearClip, float farClip)
{
	// Based on glOrtho()
	float aspectRatio = width/height;
	float left = -aspectRatio/zoom;
	float right = aspectRatio/zoom;
	float top = 1/zoom;
	float bottom = -1/zoom;
	Matrix4x4 projection ={
		2/(right-left), 0, 0, -(right+left)/(right-left),
		0, 2/(top-bottom), 0, -(top+bottom)/(top-bottom),
		0, 0, -2/(farClip-nearClip), -(farClip+nearClip)/(farClip-nearClip),
		0, 0, 0, 1
	};
	return projection;
}


Matrix4x4 operator* (const Matrix4x4& a, const Matrix4x4& b)
{
	Matrix4x4 result ={
		a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0] + a[0][3]*b[3][0],
		a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1] + a[0][3]*b[3][1],
		a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2] + a[0][3]*b[3][2],
		a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3]*b[3][3],

		a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0] + a[1][3]*b[3][0],
		a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1] + a[1][3]*b[3][1],
		a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2] + a[1][3]*b[3][2],
		a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3]*b[3][3],

		a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0] + a[2][3]*b[3][0],
		a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1] + a[2][3]*b[3][1],
		a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2] + a[2][3]*b[3][2],
		a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3]*b[3][3],

		a[3][0]*b[0][0] + a[3][1]*b[1][0] + a[3][2]*b[2][0] + a[3][3]*b[3][0],
		a[3][0]*b[0][1] + a[3][1]*b[1][1] + a[3][2]*b[2][1] + a[3][3]*b[3][1],
		a[3][0]*b[0][2] + a[3][1]*b[1][2] + a[3][2]*b[2][2] + a[3][3]*b[3][2],
		a[3][0]*b[0][3] + a[3][1]*b[1][3] + a[3][2]*b[2][3] + a[3][3]*b[3][3],
	};
	return result;
}

Vec4 operator* (const Matrix4x4& m, const Vec3& v)
{
	Vec4 v4;
	v4.xyz = v;
	v4.w = 1;
	return m*v4;
}

Vec4 operator* (const Matrix4x4& m, const Vec4& v)
{
	Vec4 result ={
		m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
		m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
		m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
		m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w,
	};
	return result;
}

// Quaternion =================================================================

const Quaternion Quaternion::identity = {1, 0, 0, 0};

float length(Quaternion q)
{
	return sqrtf(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
}

Quaternion inverse(Quaternion q)
{
	Quaternion result = {q.w, -q.x, -q.y, -q.z};
	return result;
}

Quaternion normalize(Quaternion q)
{
	float l = length(q);
	if (l == 0) {
		Quaternion result = {0};
		return result;
	}
	Quaternion result = {q.w/l, q.x/l, q.y/l, q.z/l};
	return result;
}

Quaternion lerp(Quaternion a, Quaternion b, float t)
{
	/* If the quaternions would have to go "the long way around",
		negating one of them will force it to take the shortest path. */
	if (dot(a, b) < 0) {
		b=b*-1;
	}
	return normalize(a*(1-t) + b*t);
}

float dot(Quaternion a, Quaternion b)
{
	return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
}

Quaternion slerp(Quaternion a, Quaternion b, float t) {
	/* If the quaternions would have to go "the long way around",
	negating one of them will force it to take the shortest path. */
	if (dot(a, b) < 0) {
		b=b*-1;
	}
	float halfAngleCosine = dot(a,b);
	float halfAngle = acos(halfAngleCosine);
	float halfAngleSine = sqrt(1.0f - square(halfAngleCosine));

	if (fabs(halfAngleSine) > 0.999f){
		// We can't slerp with a 180 degree angle, so do a simple lerp
		return lerp(a,b,t);
	}
	float ratioA = sin((1 - t) * halfAngle) / halfAngleSine;
	float ratioB = sin(t * halfAngle) / halfAngleSine;
	return (a*ratioA) + (b*ratioB);
}

Quaternion rotateTowards(Quaternion a, Quaternion b, float maxRadians)
{
	/* If the quaternions would have to go "the long way around",
	negating one of them will force it to take the shortest path. */
	if (dot(a, b) < 0) {
		b=b*-1;
	}
	float halfAngleCosine = clamp(dot(a, b), 0, 1);
	float angleDifference = 2*acos(halfAngleCosine);
	if (angleDifference <= maxRadians) {
		return b;
	}
	float t = maxRadians/angleDifference;
	return slerp(a, b, t);
}

float radianDifference(Quaternion angle1, Quaternion angle2)
{
	Vec3 forward = {1,0,0};
	float cosOfAngle = dot(angle1*forward, angle2*forward);
	return acosf(clamp(cosOfAngle, -1, 1));
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
	Quaternion result = {
		q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
		q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
		q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z,
		q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x
	};
	return result;
}

Quaternion operator*(const Quaternion& q, float s)
{ Quaternion result = {q.w*s, q.x*s, q.y*s, q.z*s}; return result; }

Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{ Quaternion result = {q1.w+q2.w, q1.x+q2.x, q1.y+q2.y, q1.z+q2.z}; return result; }

// TODO: this should probably be done without a matrix
Vec3 operator*(const Quaternion& q, const Vec3& v)
{ return (quaternionToMatrix4x4(q)*v).xyz; }

// Transform ==================================================================

const Transform Transform::identity = {
	{0,0,0},
	Quaternion::identity,
	{1,1,1}
};

// Conversions ================================================================

Vec3 vec3ToEulerXZ(Vec3 orientedVector)
{
	Vec3 euler = {0};
	euler.x = atan2f(orientedVector.y, orientedVector.z);
	euler.z = atan2f(orientedVector.x, orientedVector.y);
	return euler;
}

Quaternion eulerZXYToQuaternion(Vec3 eulerAngles)
{
	// Make quaternions that rotate on each axis
	Quaternion
		zAxisRot = {cosf(eulerAngles.z/2), 0, 0, sinf(eulerAngles.z/2)},
		xAxisRot = {cosf(eulerAngles.x/2), sinf(eulerAngles.x/2), 0, 0},
		yAxisRot = {cosf(eulerAngles.y/2), 0, sinf(eulerAngles.y/2), 0};

	// Combine the individual axis rotations
	return yAxisRot*xAxisRot*zAxisRot;
}

Quaternion axisAngleToQuaternion(Vec3 axis, float radians)
{
	Quaternion q;
	q.w = cosf(radians/2);
	q.xyz = sinf(radians/2)*axis;
	return q;
}

Matrix4x4 eulerZXYToMatrix4x4(Vec3 eulerAngles)
{
	Matrix4x4 rotZ ={
		cosf(eulerAngles.z), -sinf(eulerAngles.z), 0, 0,
		sinf(eulerAngles.z), cosf(eulerAngles.z), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	Matrix4x4 rotX ={
		1, 0, 0, 0,
		0, cosf(eulerAngles.x), -sinf(eulerAngles.x), 0,
		0, sinf(eulerAngles.x), cosf(eulerAngles.x), 0,
		0, 0, 0, 1
	};

	Matrix4x4 rotY ={
		cosf(eulerAngles.y), 0, sinf(eulerAngles.y), 0,
		0, 1, 0, 0,
		-sinf(eulerAngles.y), 0, cosf(eulerAngles.y), 0,
		0, 0, 0, 1
	};

	return rotY*rotX*rotZ;
}

Matrix4x4 quaternionToMatrix4x4(const Quaternion& q)
{
	Matrix4x4 result = {
		1 - 2*q.y*q.y - 2*q.z*q.z, 2*q.x*q.y - 2*q.z*q.w,     2*q.x*q.z + 2*q.y*q.w,     0,
		2*q.x*q.y + 2*q.z*q.w,     1 - 2*q.x*q.x - 2*q.z*q.z, 2*q.y*q.z - 2*q.x*q.w,     0,
		2*q.x*q.z - 2*q.y*q.w,     2*q.y*q.z + 2*q.x*q.w,     1 - 2*q.x*q.x - 2*q.y*q.y, 0,
		0,                         0,                         0,                         1
	};
	return result;
}

Matrix4x4 transformToMatrix4x4(const Transform& t)
{
	Matrix4x4 scale = {
		t.scale.x, 0, 0, 0,
		0, t.scale.y, 0, 0,
		0, 0, t.scale.z, 0,
		0, 0, 0, 1
	};

	Matrix4x4 rotation = quaternionToMatrix4x4(t.rotation);

	Matrix4x4 position = {
		1, 0, 0, t.position.x,
		0, 1, 0, t.position.y,
		0, 0, 1, t.position.z,
		0, 0, 0, 1
	};

	return position*rotation*scale;
}

Matrix4x4 transformToMatrix4x4Inverse(const Transform& t)
{
	Matrix4x4 position ={
		1, 0, 0, -t.position.x,
		0, 1, 0, -t.position.y,
		0, 0, 1, -t.position.z,
		0, 0, 0, 1
	};

	Matrix4x4 rotation = quaternionToMatrix4x4(inverse(t.rotation));

	Matrix4x4 scale ={
		1/t.scale.x, 0, 0, 0,
		0, 1/t.scale.y, 0, 0,
		0, 0, 1/t.scale.z, 0,
		0, 0, 0, 1
	};

	return scale*rotation*position;
}

Transform lerp(Transform a, Transform b, float t)
{
	Transform result;
	result.position = lerp(a.position, b.position, t);
	result.rotation = lerp(a.rotation, b.rotation, t);
	result.scale = lerp(a.scale, b.scale, t);
	return result;
}

Transform concatenateTransforms(Transform parent, Transform child)
{
	Transform result;
	result.scale = child.scale * parent.scale;
	result.rotation = parent.rotation * child.rotation;
	result.position = (quaternionToMatrix4x4(parent.rotation) * (child.position*parent.scale)).xyz + parent.position;
	return result;
}

} // namespace
#endif // include guard
