#include "pch.h"

class Vector3D
{
public:
	Vector3D() {};
	Vector3D(float x, float y, float z) : x(x), y(y), z(z) {};

	Vector3D operator-(const Vector3D &v) const
	{
		return Vector3D(x - v.x, y - v.y, z - v.z);
	}

	Vector3D &operator-=(const Vector3D &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3D operator+(const Vector3D &v) const
	{
		return Vector3D(x + v.x, y + v.y, z + v.z);
	}

	Vector3D &operator+=(const Vector3D &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3D operator*(const float f) const
	{
		return Vector3D(x * f, y * f, z * f);
	}

	Vector3D &operator*(const Vector3D &v) const
	{
		return Vector3D(x * v.x, y * v.y, z * v.z);
	}

	Vector3D &operator*=(const Vector3D &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3D &operator*=(const float f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	float magnitude()
	{
		return sqrt(x * x + y * y + z * z);
	}

	Vector3D &normalize()
	{
		float m = magnitude();
		x /= m;
		y /= m;
		z /= m;
		return *this;
	}

	float dotProduct(const Vector3D &v) const
	{
		return x * v.x + y * v.y;
	}

	Vector3D cross(const Vector3D &v) const
	{
		float tempX = y * v.z - z * v.y;
		float tempY = z * v.x - x * v.z;
		float tempZ = x * v.y - y * v.x;

		return Vector3D(tempX, tempY, tempZ);
	}

	void print()
	{
		if (!isnan(x) && !isnan(y) && !isnan(z))
		{
			printf("X - %f, Y - %f, Z - %f\n", x, y, z);
		}
	}
public:
	float x;
	float y;
	float z;
};

class Line
{
public:
	Line() {};
	Line(const Vector3D &p1, const Vector3D &p2) : m_p1(p1), m_p2(p2), m_direction(m_p2 - m_p1) {};

	Vector3D intersect(const Line &l)
	{
		if (isIntersect(l))
		{
			Vector3D a1 = (l.m_p1 - m_p1).cross(l.m_direction);
			Vector3D a2 = (l.m_p1 - m_p1).cross(m_direction);
			Vector3D b = m_direction.cross(l.m_direction);

			float t = 0;
			float u = 0;
			if (b.x != 0)
			{
				t = a1.x / b.x;
				u = a2.x / b.x;
			}
			else if (b.y != 0)
			{
				t = a1.y / b.y;
				u = a2.y / b.y;
			}
			else if (b.z != 0)
			{
				t = a1.z / b.z;
				u = a2.z / b.z;
			}

			if ((t > 1.0f || t < 0.0f) || (u > 1.0f || u < 0.0f))
			{
				return Vector3D(NAN, NAN, NAN);
			}

			Vector3D v = m_p1 + (m_direction * t);

			return v;
		}
		else
		{
			return Vector3D(NAN, NAN, NAN);
		}
	}
	void print()
	{
		printf("<<");
		m_p1.print();
		printf("><");
		m_p2.print();
		printf(">>\n");
	}
public:
	Vector3D m_p1;
	Vector3D m_p2;
	Vector3D m_direction;
private:
	bool isIntersect(const Line &l)
	{
		Vector3D v = m_direction.cross(l.m_direction);

		if (v.x == 0 && v.y == 0 && v.z == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
};

class Circle
{
public:
	Circle() : pos(Vector3D(0.0f, 0.0f, 0.0f)), r(1.0f) {};
	Circle(const Vector3D &pos, float r) : pos(pos), r(r) {};

	void placePoints(int amount)
	{
		float step = 2 * M_PI / (float)amount;
		for (float i = 0; i < 2 * M_PI; i += step)
		{
			Vector3D a = placePoint2D(i);
			Vector3D b = normal2D(a);

			Vector3D direction = b - a;
			b = b + direction * 1024.0f;

			circleLines.push_back(Line(a, b));
		}
	}

	void intersectPoints(const std::vector<Line> &lines)
	{
		for (int i = 0; i < circleLines.size(); i++)
		{
			Line &circleLine = circleLines[i];

			for (int j = 0; j < lines.size(); j++)
			{
				Line l = lines[j];

				Vector3D intersectPoint = circleLine.intersect(l);

				Vector3D a = intersectPoint - circleLine.m_p1;
				Vector3D b = circleLine.m_p2 - circleLine.m_p1;

				if (b.magnitude() > a.magnitude())
				{
					if (!isnan(intersectPoint.x) && !isnan(intersectPoint.y) && !isnan(intersectPoint.z))
					{
						circleLine.m_p2 = intersectPoint;
					}
				}
			}
		}
	}
public:
	Vector3D pos;
	float r;
	std::vector<Line> circleLines;
private:
	Vector3D placePoint2D(float theta) const // 0 -> 2pi
	{
		float tempX = pos.x + r * cos(theta);
		float tempY = pos.y + r * sin(theta);

		return Vector3D(tempX, tempY, 0.0f);
	}

	Vector3D normal2D(const Vector3D &point) const // scale to make infinity line
	{
		float tempX = 2.0f * point.x - pos.x;
		float tempY = 2.0f * point.y - pos.y;

		return Vector3D(tempX, tempY, 0.0f);
	}
};