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

	bool operator==(const Vector3D &v) const
	{
		if (!isNan() && !v.isNan())
		{
			if (x == v.x && y == v.y && z == v.z)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool operator!=(const Vector3D &v) const
	{
		if (x != v.x || y != v.y || z != v.z)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool isNan() const
	{
		if (isnan(x) || isnan(y) || isnan(z))
		{
			return true;
		}
		else
		{
			return false;
		}
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

	bool operator==(const Line &l) const
	{
		if (m_p1 == l.m_p1 && m_p2 == l.m_p2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator!=(const Line &l) const
	{
		if (m_p1 != l.m_p1 && m_p2 != l.m_p2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	Vector3D intersect(const Line &l) const
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
public:
	Vector3D m_p1;
	Vector3D m_p2;
	Vector3D m_direction;
private:
	bool isIntersect(const Line &l) const
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

	float pointMinRayMagnitude(const Line &ray, const std::vector<Line> &lines) const
	{
		float resultDistance = INFINITY;

		float minRayDistance = (ray.m_p2 - ray.m_p1).magnitude();
		for (int j = 0; j < lines.size(); j++)
		{
			Line l = lines[j];

			Vector3D intersectPoint = ray.intersect(l);

			if (!intersectPoint.isNan())
			{
				float rayDistance = (intersectPoint - ray.m_p1).magnitude();

				if (rayDistance < minRayDistance)
				{
					minRayDistance = rayDistance;

					resultDistance = rayDistance;
				}
			}
		}

		return resultDistance;
	}

	void intersectPoints(const std::vector<Line> &lines, std::vector<Line> &linesToDraw)
	{
		for (int i = 0; i < lines.size(); i++)
		{
			Line l = lines[i];
			Line newL = l;

			float minDistanceA = (l.m_p2 - l.m_p1).magnitude();
			float minDistanceB = (l.m_p1 - l.m_p2).magnitude();
			for (int j = 0; j < circleLines.size(); j++)
			{
				Line &circleLine = circleLines[j];

				Vector3D intersectPoint = circleLine.intersect(l);

				if (!intersectPoint.isNan())
				{
					float rayMagnitude = (intersectPoint - circleLine.m_p1).magnitude();
					float minRayMagnitude = pointMinRayMagnitude(circleLine, lines);

					if (rayMagnitude == minRayMagnitude)
					{
						circleLine.m_p2 = intersectPoint;

						float vectorMagnitudeA = (intersectPoint - l.m_p1).magnitude();
						float vectorMagnitudeB = (intersectPoint - l.m_p2).magnitude();

						if (vectorMagnitudeA < vectorMagnitudeB)
						{
							if (vectorMagnitudeA < minDistanceA)
							{
								newL.m_p1 = intersectPoint;

								minDistanceA = vectorMagnitudeA;
							}
						}
						else
						{
							if (vectorMagnitudeB < minDistanceB)
							{
								newL.m_p2 = intersectPoint;

								minDistanceB = vectorMagnitudeB;
							}
						}
					}
				}
			}
			if (l != newL)
			{
				linesToDraw.push_back(newL);
			}
		}
	}
public:
	Vector3D pos;
	float r;
	std::vector<Line> circleLines;
private:
	Vector3D placePoint2D(float theta) const
	{
		float tempX = pos.x + r * cos(theta);
		float tempY = pos.y + r * sin(theta);

		return Vector3D(tempX, tempY, 0.0f);
	}

	Vector3D normal2D(const Vector3D &point) const
	{
		float tempX = 2.0f * point.x - pos.x;
		float tempY = 2.0f * point.y - pos.y;

		return Vector3D(tempX, tempY, 0.0f);
	}
};