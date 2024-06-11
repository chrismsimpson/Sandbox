
#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <print>
#include <sstream>
#include <variant>
#include <vector>

#include <SDL3/SDL.h>

class Error
{
public:
    Error(
        const std::string &type,
        const std::optional<std::string> &message = std::nullopt)
        : m_type(type), m_message(message) {}

    Error(
        std::string &&type,
        std::optional<std::string> &&message)
        : m_type(std::move(type)), m_message(std::move(message)) {}

    Error(
        const Error &other)
        : m_type(other.m_type), m_message(other.m_message) {}

    Error &operator=(
        const Error &other)
    {
        if (this != &other)
        {
            m_type = other.m_type;
            m_message = other.m_message;
        }

        return *this;
    }

    ~Error() {}

    const std::string &type() const { return m_type; }

    const std::optional<std::string> &message() const { return m_message; }

private:
    std::string m_type;
    std::optional<std::string> m_message;
};

class Vec4
{
public:
    Vec4()
        : m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(1.0f) {}

    Vec4(float x, float y, float z)
        : m_x(x), m_y(y), m_z(z), m_w(1.0f) {}

    Vec4(float x, float y, float z, float w)
        : m_x(x), m_y(y), m_z(z), m_w(w) {}

    Vec4(const Vec4 &other)
        : m_x(other.m_x), m_y(other.m_y), m_z(other.m_z), m_w(other.m_w) {}

    Vec4(Vec4 &&other)
        : m_x(std::move(other.m_x)), m_y(std::move(other.m_y)), m_z(std::move(other.m_z)), m_w(std::move(other.m_w)) {}

    Vec4 &operator=(
        const Vec4 &other)
    {
        if (this != &other)
        {
            m_x = other.m_x;
            m_y = other.m_y;
            m_z = other.m_z;
            m_w = other.m_w;
        }

        return *this;
    }

    const float &x() const { return m_x; }

    const float &y() const { return m_y; }

    const float &z() const { return m_z; }

    const float &w() const { return m_w; }

    void x(float x)
    {
        m_x = x;
    }

    void y(float y)
    {
        m_y = y;
    }

    void z(float z)
    {
        m_z = z;
    }

    void w(float w)
    {
        m_w = w;
    }

    static Vec4 add(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return Vec4(
            lhs.m_x + rhs.m_x,
            lhs.m_y + rhs.m_y,
            lhs.m_z + rhs.m_z);
    }

    static Vec4 subtract(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return Vec4(
            lhs.m_x - rhs.m_x,
            lhs.m_y - rhs.m_y,
            lhs.m_z - rhs.m_z);
    }

    static Vec4 multiply(
        const Vec4 &lhs,
        float k)
    {
        return Vec4(
            lhs.m_x * k,
            lhs.m_y * k,
            lhs.m_z * k);
    }

    static Vec4 divide(
        const Vec4 &lhs,
        float k)
    {
        return Vec4(
            lhs.m_x / k,
            lhs.m_y / k,
            lhs.m_z / k);
    }

    static float dot_product(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return lhs.m_x * rhs.m_x +
               lhs.m_y * rhs.m_y +
               lhs.m_z * rhs.m_z;
    }

    static float length(
        const Vec4 &vec)
    {
        return std::sqrt(Vec4::dot_product(vec, vec));
    }

    static float distance(
        const Vec4 &a,
        const Vec4 &b)
    {
        const auto dx = b.x() - a.x();
        const auto dy = b.y() - a.y();
        const auto dz = b.z() - a.z();

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    static Vec4 normalize(
        const Vec4 &vec)
    {
        const auto len = Vec4::length(vec);

        return Vec4(
            vec.m_x / len,
            vec.m_y / len,
            vec.m_z / len);
    }

    static Vec4 cross_product(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return Vec4(
            lhs.m_y * rhs.m_z - lhs.m_z * rhs.m_y,
            lhs.m_z * rhs.m_x - lhs.m_x * rhs.m_z,
            lhs.m_x * rhs.m_y - lhs.m_y * rhs.m_x);
    }

    static Vec4 intersect_plane(
        const Vec4 &plane_p,
        const Vec4 &plane_n,
        const Vec4 &line_start,
        const Vec4 &line_end)
    {
        const auto plane_n_norm = Vec4::normalize(plane_n);
        const auto plane_d = -Vec4::dot_product(plane_n_norm, plane_p);
        const auto ad = Vec4::dot_product(line_start, plane_n_norm);
        const auto bd = Vec4::dot_product(line_end, plane_n_norm);
        const auto t = (-plane_d - ad) / (bd - ad);
        const auto line_start_to_end = Vec4::subtract(line_end, line_start);
        const auto line_to_intersect = Vec4::multiply(line_start_to_end, t);
        return Vec4::add(line_start, line_to_intersect);
    }

private:
    float m_x;
    float m_y;
    float m_z;
    float m_w;
};

class Triangle
{
public:
    Triangle()
        : m_points{Vec4(0.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f)}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
        : m_points{Vec4(x1, y1, z1), Vec4(x2, y2, z2), Vec4(x3, y3, z3)}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, const SDL_Color &color)
        : m_points{Vec4(x1, y1, z1), Vec4(x2, y2, z2), Vec4(x3, y3, z3)}, m_color(color) {}

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, SDL_Color &&color)
        : m_points{Vec4(x1, y1, z1), Vec4(x2, y2, z2), Vec4(x3, y3, z3)}, m_color(std::move(color)) {}

    Triangle(const Vec4 &p0, const Vec4 &p1, const Vec4 &p2)
        : m_points{p0, p1, p2}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(const Vec4 &p0, const Vec4 &p1, const Vec4 &p2, const SDL_Color &color)
        : m_points{p0, p1, p2}, m_color(color) {}

    Triangle(Vec4 &&p0, Vec4 &&p1, Vec4 &&p2)
        : m_points{std::move(p0), std::move(p1), std::move(p2)}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(Vec4 &&p0, Vec4 &&p1, Vec4 &&p2, SDL_Color &color)
        : m_points{std::move(p0), std::move(p1), std::move(p2)}, m_color(std::move(color)) {}

    Triangle(const Triangle &other)
        : m_points{other.m_points[0], other.m_points[1], other.m_points[2]}, m_color(other.m_color) {}

    Triangle &operator=(
        const Triangle &other)
    {
        if (this != &other)
        {
            m_points[0] = other.m_points[0];
            m_points[1] = other.m_points[1];
            m_points[2] = other.m_points[2];
            m_color = other.m_color;
        }

        return *this;
    }

    const Vec4 &point_at(int index) const { return m_points[index]; }

    void set_point_at(int index, const Vec4 &point)
    {
        m_points[index] = point;
    }

    void set_x_on_point_at(int index, float x)
    {
        m_points[index].x(x);
    }

    void set_y_on_point_at(int index, float y)
    {
        m_points[index].y(y);
    }

    void set_z_on_point_at(int index, float z)
    {
        m_points[index].z(z);
    }

    const SDL_Color &color() const { return m_color; }

    void set_color(const SDL_Color &color)
    {
        m_color = color;
    }

    static float shortest_distance(
        const Vec4 &plane_p,
        const Vec4 &plane_n,
        const Vec4 &p)
    {
        return plane_n.x() * p.x() + plane_n.y() * p.y() + plane_n.z() * p.z() - Vec4::dot_product(plane_n, plane_p);
    }

    static int clip_against_plane(
        const Vec4 &plane_p,
        const Vec4 &plane_n,
        const Triangle &in_tri,
        Triangle *out_tri1,
        Triangle *out_tri2)
    {
        // Make sure plane normal is indeed normal

        const auto _plane_n = Vec4::normalize(plane_n);

        // Create two temporary storage arrays to classify points either side of plane
        // If distance sign is positive, point lies on "inside" of plane

        Vec4 inside_points[3] = {{}, {}, {}};
        auto inside_point_count = 0;

        Vec4 outside_points[3] = {{}, {}, {}};
        auto outside_point_count = 0;

        // Get signed distance of each point in triangle to plane

        const auto d0 = Triangle::shortest_distance(plane_p, _plane_n, in_tri.m_points[0]);
        const auto d1 = Triangle::shortest_distance(plane_p, _plane_n, in_tri.m_points[1]);
        const auto d2 = Triangle::shortest_distance(plane_p, _plane_n, in_tri.m_points[2]);

        if (d0 >= 0)
        {
            inside_points[inside_point_count++] = in_tri.m_points[0];
        }
        else
        {
            outside_points[outside_point_count++] = in_tri.m_points[0];
        }

        if (d1 >= 0)
        {
            inside_points[inside_point_count++] = in_tri.m_points[1];
        }
        else
        {
            outside_points[outside_point_count++] = in_tri.m_points[1];
        }

        if (d2 >= 0)
        {
            inside_points[inside_point_count++] = in_tri.m_points[2];
        }
        else
        {
            outside_points[outside_point_count++] = in_tri.m_points[2];
        }

        // Now classify triangle points, and break the input triangle into
        // smaller output triangles if required. There are four possible
        // outcomes...

        if (inside_point_count == 0)
        {
            // All points lie on the outside of plane, so clip whole triangle
            // It ceases to exist

            return 0; // No returned triangles are valid
        }

        if (inside_point_count == 3)
        {
            // All points lie on the inside of plane, so do nothing
            // and allow the triangle to simply pass through

            // Triangle out_tri1(in_tri);
            out_tri1->m_points[0] = inside_points[0];
            out_tri1->m_points[1] = inside_points[1];
            out_tri1->m_points[2] = inside_points[2];

            return 1; // Just the one returned original triangle is valid
        }

        if (inside_point_count == 1 && outside_point_count == 2)
        {
            // Triangle should be clipped. As two points lie outside
            // the plane, the triangle simply becomes a smaller triangle

            // Copy appearance info to new triangle

            out_tri1->m_color = in_tri.m_color;

            // The inside point is valid, so keep that...

            out_tri1->m_points[0] = inside_points[0];

            // but the two new points are at the locations where the
            // original sides of the triangle (lines) intersect with the plane

            out_tri1->m_points[1] = Vec4::intersect_plane(plane_p, _plane_n, inside_points[0], outside_points[0]);
            out_tri1->m_points[2] = Vec4::intersect_plane(plane_p, _plane_n, inside_points[0], outside_points[1]);

            return 1; // Return the newly formed single triangle
        }

        if (inside_point_count == 2 && outside_point_count == 1)
        {
            // Triangle should be clipped. As two points lie inside the plane,
            // the clipped triangle becomes a "quad". Fortunately, we can
            // represent a quad with two new triangles

            // Copy appearance info to new triangles

            out_tri1->m_color = in_tri.m_color;
            out_tri2->m_color = in_tri.m_color;

            // The first triangle consists of the two inside points and a new
            // point determined by the location where one side of the triangle
            // intersects with the plane

            out_tri1->m_points[0] = inside_points[0];
            out_tri1->m_points[1] = inside_points[1];
            out_tri1->m_points[2] = Vec4::intersect_plane(plane_p, _plane_n, inside_points[0], outside_points[0]);

            // The second triangle is composed of one of he inside points, a
            // new point determined by the intersection of the other side of the
            // triangle and the plane, and the newly created point above

            out_tri2->m_points[0] = inside_points[1];
            out_tri2->m_points[1] = out_tri1->m_points[2];
            out_tri2->m_points[2] = Vec4::intersect_plane(plane_p, _plane_n, inside_points[1], outside_points[0]);

            return 2; // Return two newly formed triangles which form a quad
        }

        return 0; // No returned triangles are valid
    }

private:
    Vec4 m_points[3];
    SDL_Color m_color;
};

class Mesh
{
public:
    Mesh(const std::vector<Triangle> &triangles)
        : m_triangles(triangles) {}

    Mesh(std::vector<Triangle> &&triangles)
        : m_triangles(std::move(triangles)) {}

    Mesh(const Mesh &other)
        : m_triangles(other.m_triangles) {}

    Mesh(
        Mesh &&other)
        : m_triangles(std::move(other.m_triangles)) {}

    Mesh &operator=(
        const Mesh &other)
    {
        if (this != &other)
        {
            m_triangles = other.m_triangles;
        }

        return *this;
    }

    static std::variant<Mesh, Error> load_from_obj_file(
        const std::string &filename)
    {
        std::ifstream file(filename);

        if (!file.is_open())
        {
            return Error("file", "Could not open file");
        }

        ///

        std::string line;

        std::vector<Vec4> vertices;

        std::vector<Triangle> triangles;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);

            std::string type;

            iss >> type;

            if (type == "v")
            {
                float x, y, z;

                iss >> x >> y >> z;

                vertices.push_back(Vec4(x, y, z, 1.0f));
            }
            else if (type == "f")
            {
                int v1, v2, v3;

                iss >> v1 >> v2 >> v3;

                triangles.push_back(Triangle(vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1]));
            }
        }

        file.close();

        return Mesh(triangles);
    }

    static std::vector<float> generate_height_map(
        int stride)
    {
        const auto stride_plus_one = stride + 1;

        const auto n = stride_plus_one * stride_plus_one;

        std::vector<float> height_map(n);

        for (auto i = 0; i < n; i++)
        {
            height_map[i] = 0.0f;
        }

        return height_map;
    }

    static SDL_Color color_given_heights(float z1, float z2, float z3)
    {
        if (z1 == 0 && z2 == 0 && z3 == 0)
        {
            return SDL_Color{0x00, 0x00, 0xff, 0xcc}; // blue (water)
        }

        if (z1 == 0 || z2 == 0 || z3 == 0)
        {
            return SDL_Color{0xff, 0xff, 0x70, 0x99}; // yellow (sand)
        }

        return SDL_Color{0x00, 0x70, 0x00, 0x99}; // green (grass)
    }

    static Mesh create_from_height_map(
        const std::vector<float> &height_map)
    {
        const auto height_map_size_f = static_cast<float>(height_map.size());

        const auto height_map_length_sqrt = std::sqrt(height_map_size_f);

        const auto stride_plus_one = static_cast<int>(height_map_length_sqrt);

        const auto stride = stride_plus_one - 1;

        const auto stride_f = static_cast<float>(stride);

        ///

        const auto triangles_size = stride * stride * 2;

        Triangle triangles[triangles_size];

        for (auto z = 0.0f; z < stride_f; ++z)
        {
            for (float x = 0; x < stride_f; ++x)
            {
                const auto n = static_cast<int>(z * stride_f + x);

                const auto n1 = n * 2;

                const auto n2 = n1 + 1;

                const auto h0 = static_cast<int>(z * (stride_f + 1.0f) + x);

                const auto h1 = h0;

                const auto h2 = h1 + 1;

                const auto h3 = static_cast<int>((z + 1) * (stride_f + 1.0f) + x);
                const auto h4 = h3 + 1;

                triangles[n1] = Triangle{x, height_map[h1], z, x, height_map[h3], z + 1.0f, x + 1.0f, height_map[h4], z + 1.0f, Mesh::color_given_heights(height_map[h1], height_map[h3], height_map[h4])};
                triangles[n2] = Triangle{x, height_map[h1], z, x + 1.0f, height_map[h4], z + 1.0f, x + 1.0f, height_map[h2], z, Mesh::color_given_heights(height_map[h1], height_map[h4], height_map[h2])};
            }
        }

        ///

        std::vector<Triangle> triangles_vec;

        for (auto i = 0; i < triangles_size; i++)
        {
            triangles_vec.push_back(triangles[i]);
        }

        ///

        return Mesh(triangles_vec);
    }

    const std::vector<Triangle> &triangles() const { return m_triangles; }

private:
    std::vector<Triangle> m_triangles;
};

class Matrix4x4
{
public:
    Matrix4x4(
        float matrix[4][4])
        : m_matrix{
              {matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]},
              {matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]},
              {matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]},
              {matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]}} {}

    Matrix4x4()
        : m_matrix{
              {0.0f, 0.0f, 0.0f, 0.0f},
              {0.0f, 0.0f, 0.0f, 0.0f},
              {0.0f, 0.0f, 0.0f, 0.0f},
              {0.0f, 0.0f, 0.0f, 0.0f}} {}

    Matrix4x4(
        float m0, float m1, float m2, float m3,
        float m4, float m5, float m6, float m7,
        float m8, float m9, float ma, float mb,
        float mc, float md, float me, float mf)
        : m_matrix{
              {m0, m1, m2, m3},
              {m4, m5, m6, m7},
              {m8, m9, ma, mb},
              {mc, md, me, mf}} {}

    Matrix4x4(
        const Matrix4x4 &other)
        : m_matrix{
              {other.m_matrix[0][0], other.m_matrix[0][1], other.m_matrix[0][2], other.m_matrix[0][3]},
              {other.m_matrix[1][0], other.m_matrix[1][1], other.m_matrix[1][2], other.m_matrix[1][3]},
              {other.m_matrix[2][0], other.m_matrix[2][1], other.m_matrix[2][2], other.m_matrix[2][3]},
              {other.m_matrix[3][0], other.m_matrix[3][1], other.m_matrix[3][2], other.m_matrix[3][3]}} {}

    Matrix4x4(
        Matrix4x4 &&other)
        : m_matrix{
              {std::move(other.m_matrix[0][0]), std::move(other.m_matrix[0][1]), std::move(other.m_matrix[0][2]), std::move(other.m_matrix[0][3])},
              {std::move(other.m_matrix[1][0]), std::move(other.m_matrix[1][1]), std::move(other.m_matrix[1][2]), std::move(other.m_matrix[1][3])},
              {std::move(other.m_matrix[2][0]), std::move(other.m_matrix[2][1]), std::move(other.m_matrix[2][2]), std::move(other.m_matrix[2][3])},
              {std::move(other.m_matrix[3][0]), std::move(other.m_matrix[3][1]), std::move(other.m_matrix[3][2]), std::move(other.m_matrix[3][3])}} {}

    Matrix4x4 &operator=(
        const Matrix4x4 &other)
    {
        if (this != &other)
        {
            m_matrix[0][0] = other.m_matrix[0][0];
            m_matrix[0][1] = other.m_matrix[0][1];
            m_matrix[0][2] = other.m_matrix[0][2];
            m_matrix[0][3] = other.m_matrix[0][3];

            m_matrix[1][0] = other.m_matrix[1][0];
            m_matrix[1][1] = other.m_matrix[1][1];
            m_matrix[1][2] = other.m_matrix[1][2];
            m_matrix[1][3] = other.m_matrix[1][3];

            m_matrix[2][0] = other.m_matrix[2][0];
            m_matrix[2][1] = other.m_matrix[2][1];
            m_matrix[2][2] = other.m_matrix[2][2];
            m_matrix[2][3] = other.m_matrix[2][3];

            m_matrix[3][0] = other.m_matrix[3][0];
            m_matrix[3][1] = other.m_matrix[3][1];
            m_matrix[3][2] = other.m_matrix[3][2];
            m_matrix[3][3] = other.m_matrix[3][3];
        }

        return *this;
    }

    static Vec4 multiply_vector(
        const Matrix4x4 &m,
        const Vec4 &i)
    {
        return Vec4(
            i.x() * m.m_matrix[0][0] + i.y() * m.m_matrix[1][0] + i.z() * m.m_matrix[2][0] + m.m_matrix[3][0],
            i.x() * m.m_matrix[0][1] + i.y() * m.m_matrix[1][1] + i.z() * m.m_matrix[2][1] + m.m_matrix[3][1],
            i.x() * m.m_matrix[0][2] + i.y() * m.m_matrix[1][2] + i.z() * m.m_matrix[2][2] + m.m_matrix[3][2],
            i.x() * m.m_matrix[0][3] + i.y() * m.m_matrix[1][3] + i.z() * m.m_matrix[2][3] + m.m_matrix[3][3]);
    }

    static Matrix4x4 make_identity()
    {
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 make_rotation_x(
        float angle_rad)
    {
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, std::cos(angle_rad), std::sin(angle_rad), 0.0f,
            0.0f, -std::sin(angle_rad), std::cos(angle_rad), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 make_rotation_y(
        float angle_rad)
    {
        return Matrix4x4(
            std::cos(angle_rad), 0.0f, -std::sin(angle_rad), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            std::sin(angle_rad), 0.0f, std::cos(angle_rad), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 make_rotation_z(
        float angle_rad)
    {
        return Matrix4x4(
            std::cos(angle_rad), std::sin(angle_rad), 0.0f, 0.0f,
            -std::sin(angle_rad), std::cos(angle_rad), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 make_translation(
        float x,
        float y,
        float z)
    {
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f);
    }

    static Matrix4x4 make_projection(
        float fov_degrees,
        float aspect_ratio,
        float near,
        float far)
    {
        const auto fov_rad = 1.0f / std::tan(fov_degrees * 0.5f / 180.0f * 3.14159f);

        return Matrix4x4(
            aspect_ratio * fov_rad, 0.0f, 0.0f, 0.0f,
            0.0f, fov_rad, 0.0f, 0.0f,
            0.0f, 0.0f, far / (far - near), 1.0f,
            0.0f, 0.0f, (-far * near) / (far - near), 0.0f);
    }

    static Matrix4x4 multiply(
        const Matrix4x4 &m1,
        const Matrix4x4 &m2)
    {
        Matrix4x4 matrix;

        for (auto c = 0; c < 4; c++)
        {
            for (auto r = 0; r < 4; r++)
            {
                matrix.m_matrix[r][c] = m1.m_matrix[r][0] * m2.m_matrix[0][c] + m1.m_matrix[r][1] * m2.m_matrix[1][c] + m1.m_matrix[r][2] * m2.m_matrix[2][c] + m1.m_matrix[r][3] * m2.m_matrix[3][c];
            }
        }

        return matrix;
    }

    static Matrix4x4 point_at(
        const Vec4 &pos,
        const Vec4 &target,
        const Vec4 &up)
    {
        // Calculate new forward direction

        auto new_forward = Vec4::subtract(target, pos);

        new_forward = Vec4::normalize(new_forward);

        // Calculate new up direction

        const auto a = Vec4::multiply(new_forward, Vec4::dot_product(up, new_forward));

        auto new_up = Vec4::subtract(up, a);

        new_up = Vec4::normalize(new_up);

        // New right direction is easy, its just cross product

        const auto new_right = Vec4::cross_product(new_up, new_forward);

        // Construct Dimensioning and Translation Matrix

        return Matrix4x4(
            new_right.x(), new_right.y(), new_right.z(), 0.0f,
            new_up.x(), new_up.y(), new_up.z(), 0.0f,
            new_forward.x(), new_forward.y(), new_forward.z(), 0.0f,
            pos.x(), pos.y(), pos.z(), 1.0f);
    }

    static Matrix4x4 quick_inverse(
        const Matrix4x4 &m)
    {
        // return Matrix4x4(
        //     m.m_matrix[0][0], m.m_matrix[1][0], m.m_matrix[2][0], 0.0f,
        //     m.m_matrix[0][1], m.m_matrix[1][1], m.m_matrix[2][1], 0.0f,
        //     m.m_matrix[0][2], m.m_matrix[1][2], m.m_matrix[2][2], 0.0f,
        //     -(m.m_matrix[3][0] * m.m_matrix[0][0] + m.m_matrix[3][1] * m.m_matrix[1][0] + m.m_matrix[3][2] * m.m_matrix[2][0]),
        //     -(m.m_matrix[3][0] * m.m_matrix[0][1] + m.m_matrix[3][1] * m.m_matrix[1][1] + m.m_matrix[3][2] * m.m_matrix[2][1]),
        //     -(m.m_matrix[3][0] * m.m_matrix[0][2] + m.m_matrix[3][1] * m.m_matrix[1][2] + m.m_matrix[3][2] * m.m_matrix[2][2]),
        //     1.0f);

        Matrix4x4 matrix;
        matrix.m_matrix[0][0] = m.m_matrix[0][0];
        matrix.m_matrix[0][1] = m.m_matrix[1][0];
        matrix.m_matrix[0][2] = m.m_matrix[2][0];
        matrix.m_matrix[0][3] = 0.0f;
        matrix.m_matrix[1][0] = m.m_matrix[0][1];
        matrix.m_matrix[1][1] = m.m_matrix[1][1];
        matrix.m_matrix[1][2] = m.m_matrix[2][1];
        matrix.m_matrix[1][3] = 0.0f;
        matrix.m_matrix[2][0] = m.m_matrix[0][2];
        matrix.m_matrix[2][1] = m.m_matrix[1][2];
        matrix.m_matrix[2][2] = m.m_matrix[2][2];
        matrix.m_matrix[2][3] = 0.0f;
        matrix.m_matrix[3][0] = -(m.m_matrix[3][0] * matrix.m_matrix[0][0] + m.m_matrix[3][1] * matrix.m_matrix[1][0] + m.m_matrix[3][2] * matrix.m_matrix[2][0]);
        matrix.m_matrix[3][1] = -(m.m_matrix[3][0] * matrix.m_matrix[0][1] + m.m_matrix[3][1] * matrix.m_matrix[1][1] + m.m_matrix[3][2] * matrix.m_matrix[2][1]);
        matrix.m_matrix[3][2] = -(m.m_matrix[3][0] * matrix.m_matrix[0][2] + m.m_matrix[3][1] * matrix.m_matrix[1][2] + m.m_matrix[3][2] * matrix.m_matrix[2][2]);
        matrix.m_matrix[3][3] = 1.0f;
        return matrix;
    }

private:
    float m_matrix[4][4];
};

class Game
{
public:
    Game(
        int screen_width,
        int screen_height)
    {
        m_screen_width = screen_width;
        m_screen_height = screen_height;

        std::println("creating window");

        m_window = SDL_CreateWindow(
            "Game",
            m_screen_width,
            m_screen_height,
            SDL_WINDOW_HIGH_PIXEL_DENSITY);

        ///

        int window_size_width, window_size_height, window_size_pixels_width, window_size_pixels_height;

        SDL_GetWindowSize(m_window, &window_size_width, &window_size_height);

        SDL_GetWindowSizeInPixels(m_window, &window_size_pixels_width, &window_size_pixels_height);

        ///

        m_scale = static_cast<float>(window_size_pixels_width) / static_cast<float>(window_size_width);

        ///

        m_width = static_cast<float>(m_screen_width) * m_scale;
        m_height = static_cast<float>(m_screen_height) * m_scale;

        ///

        m_renderer = SDL_CreateRenderer(m_window, nullptr);

        SDL_SetRenderVSync(m_renderer, SDL_FALSE);

        SDL_RaiseWindow(m_window);

        ///

        m_projection_matrix = {};

        m_camera = {};
    }

    ~Game()
    {
        if (m_renderer)
        {
            SDL_DestroyRenderer(m_renderer);
        }

        if (m_window)
        {
            SDL_DestroyWindow(m_window);
        }
    }

    void on_create()
    {
        m_mesh = Mesh::create_from_height_map(Mesh::generate_height_map(128));

        m_projection_matrix = Matrix4x4::make_projection(90.0f, m_height / m_width, 0.1f, 1000.0f);

        ///

        m_camera = Vec4(8.0f, 9.51649f, 2.4497957f, 1.0f);
        m_yaw = 0.0f;
        m_pitch = 0.7708009f;
        m_roll = 0.0f;
    }

    void on_update(
        float elapsed)
    {
        const auto c1 = m_camera;

        const auto yaw_start = m_yaw;

        const auto pitch_start = m_pitch;

        const auto roll_start = m_roll;

        ///

        const auto keyboard_state = SDL_GetKeyboardState(nullptr);

        if (keyboard_state[SDL_SCANCODE_UP])
        {
            m_camera = Vec4(m_camera.x(), m_camera.y() + (8.0f * elapsed), m_camera.z(), m_camera.w());
        }

        if (keyboard_state[SDL_SCANCODE_DOWN])
        {
            m_camera = Vec4(m_camera.x(), m_camera.y() - (8.0f * elapsed), m_camera.z(), m_camera.w());
        }

        ///

        if (keyboard_state[SDL_SCANCODE_LEFT])
        {
            m_camera = Vec4(m_camera.x() + (8.0f * elapsed), m_camera.y(), m_camera.z(), m_camera.w());
        }

        if (keyboard_state[SDL_SCANCODE_RIGHT])
        {
            m_camera = Vec4(m_camera.x() - (8.0f * elapsed), m_camera.y(), m_camera.z(), m_camera.w());
        }

        ///

        const auto forward = Vec4::multiply(m_look_direction, std::exp(std::exp(m_pitch) - 1.0f) * 8.0f * elapsed);

        if (keyboard_state[SDL_SCANCODE_W])
        {
            const auto w = Vec4::add(m_camera, forward);

            m_camera = Vec4(w.x(), m_camera.y(), w.z(), w.w());
        }

        if (keyboard_state[SDL_SCANCODE_S])
        {
            const auto s = Vec4::subtract(m_camera, forward);

            m_camera = Vec4(s.x(), m_camera.y(), s.z(), s.w());
        }

        ///

        if (keyboard_state[SDL_SCANCODE_Q])
        {
            m_yaw -= 2.0f * elapsed;
        }

        if (keyboard_state[SDL_SCANCODE_E])
        {
            m_yaw += 2.0f * elapsed;
        }

        ///

        const auto up = Vec4(0.0f, 1.0f, 0.0f, 1.0f);

        const auto direction = Vec4::cross_product(up, forward);

        ///

        if (keyboard_state[SDL_SCANCODE_A])
        {
            m_camera = Vec4(m_camera.x() + direction.x(), m_camera.y(), m_camera.z() + direction.z(), m_camera.w());
        }

        if (keyboard_state[SDL_SCANCODE_D])
        {
            m_camera = Vec4(m_camera.x() - direction.x(), m_camera.y(), m_camera.z() - direction.z(), m_camera.w());
        }

        ///

        if (keyboard_state[SDL_SCANCODE_T])
        {
            m_camera = Vec4::add(m_camera, forward);
            m_camera = Vec4(m_camera.x(), m_camera.y() - (8.0f * elapsed), m_camera.z(), m_camera.w());
        }

        if (keyboard_state[SDL_SCANCODE_G])
        {
            m_camera = Vec4::subtract(m_camera, forward);
            m_camera = Vec4(m_camera.x(), m_camera.y() + (8.0f * elapsed), m_camera.z(), m_camera.w());
        }

        ///

        if (keyboard_state[SDL_SCANCODE_U])
        {
            m_pitch -= 2.0f * elapsed;
        }

        if (keyboard_state[SDL_SCANCODE_J])
        {
            m_pitch += 2.0f * elapsed;
        }

        m_pitch = std::min(std::max(0.0f, m_pitch), 1.5f);

        ///

        if (m_camera.y() < 0.5f)
        {
            m_camera = Vec4(m_camera.x(), 0.5f, m_camera.z(), m_camera.w());
        }

        ///

        if (c1.x() != m_camera.x() || c1.y() != m_camera.y() || c1.z() != m_camera.z() || c1.w() != m_camera.w() || yaw_start != m_yaw || pitch_start != m_pitch || roll_start != m_roll)
        {
            std::println("        camera = Vec4(x: {0}f, y: {1}f, z: {2}f, w: {3}f);", m_camera.x(), m_camera.y(), m_camera.z(), m_camera.w());
            std::println("        yaw = {0}f;", m_yaw);
            std::println("        pitch = {0}f;", m_pitch);
            std::println("        roll = {0}f;\n", m_roll);
        }

        ///

        const auto rotation_z = Matrix4x4::make_rotation_z(m_theta * 0.5f);

        const auto rotation_x = Matrix4x4::make_rotation_x(m_theta);

        ///

        const auto translation = Matrix4x4::make_translation(0.0f, 0.0f, 10.0f); // TODO: explore diff values?

        auto world = Matrix4x4::make_identity();
        world = Matrix4x4::multiply(rotation_z, rotation_x);
        world = Matrix4x4::multiply(world, translation);

        ///

        auto target = Vec4(0.0f, 0.0f, 1.0f, 1.0f);

        const auto camera_rotation = Matrix4x4::multiply(Matrix4x4::multiply(Matrix4x4::make_rotation_x(m_pitch), Matrix4x4::make_rotation_y(m_yaw)), Matrix4x4::make_rotation_z(m_roll));

        m_look_direction = Matrix4x4::multiply_vector(camera_rotation, target);

        target = Vec4::add(m_camera, m_look_direction);

        const auto camera = Matrix4x4::point_at(m_camera, target, up);

        const auto view = Matrix4x4::quick_inverse(camera);

        std::vector<Triangle> triangles;

        if (m_mesh.has_value())
        {
            for (auto i = 0; i < m_mesh->triangles().size(); i++)
            {
                auto tri = m_mesh->triangles()[i];

                // World Matrix Transform

                auto tri_transformed = Triangle{};

                tri_transformed.set_point_at(0, Matrix4x4::multiply_vector(world, tri.point_at(0)));

                // if transformed is not a certain radius of camera and it's pitch, skip (continue)
                if (Vec4::distance(tri_transformed.point_at(0), m_camera) > 50.0f || 
                    Vec4::dot_product(Vec4::normalize(Vec4::subtract(tri_transformed.point_at(0), m_camera)), m_look_direction) < 0.0f)
                {
                    continue;
                }

                ///

                auto tri_projected = Triangle{};

                auto tri_viewed = Triangle{};

                // World Matrix Transform (cont.)

                tri_transformed.set_point_at(1, Matrix4x4::multiply_vector(world, tri.point_at(1)));
                tri_transformed.set_point_at(2, Matrix4x4::multiply_vector(world, tri.point_at(2)));
                tri_transformed.set_color(tri.color());

                // Calculate triangle Normal
                // Get lines either side of triangle

                const auto line1 = Vec4::subtract(tri_transformed.point_at(1), tri_transformed.point_at(0));
                const auto line2 = Vec4::subtract(tri_transformed.point_at(2), tri_transformed.point_at(0));

                // Take cross product of lines to get normal to triangle surface

                auto normal = Vec4::cross_product(line1, line2);

                // You normally need to normalise a normal!

                normal = Vec4::normalize(normal);

                // Get Ray from triangle to camera

                const auto camera_ray = Vec4::subtract(tri_transformed.point_at(0), m_camera);

                // If ray is aligned with normal, then triangle is visible

                if (Vec4::dot_product(normal, camera_ray) < 0.0f)
                {
                    // Illumination

                    auto light_direction = Vec4(0.0f, 1.0f, -1.0f, 1.0f);

                    light_direction = Vec4::normalize(light_direction);

                    // How "aligned" are light direction and triangle surface normal?

                    const auto dp = std::max(0.1f, Vec4::dot_product(normal, light_direction));

                    tri_transformed.set_color({static_cast<uint8_t>(tri_transformed.color().r * dp),
                                               static_cast<uint8_t>(tri_transformed.color().g * dp),
                                               static_cast<uint8_t>(tri_transformed.color().b * dp),
                                               tri_transformed.color().a});

                    // Convert World Space --> View space

                    tri_viewed.set_point_at(0, Matrix4x4::multiply_vector(view, tri_transformed.point_at(0)));
                    tri_viewed.set_point_at(1, Matrix4x4::multiply_vector(view, tri_transformed.point_at(1)));
                    tri_viewed.set_point_at(2, Matrix4x4::multiply_vector(view, tri_transformed.point_at(2)));
                    tri_viewed.set_color(tri_transformed.color());

                    // Clip Viewed Triangle against near plane, this could form two additional
                    // additional triangles.

                    auto clipped_triangles = 0;

                    Triangle clipped[2] = {Triangle{}, Triangle{}};

                    clipped_triangles = Triangle::clip_against_plane({0.0f, 0.0f, 0.1f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, tri_viewed, &clipped[0], &clipped[1]);

                    for (auto n = 0; n < clipped_triangles; n++)
                    {
                        // Project triangles from 3D --> 2D

                        tri_projected.set_point_at(0, Matrix4x4::multiply_vector(m_projection_matrix, clipped[n].point_at(0)));
                        tri_projected.set_point_at(1, Matrix4x4::multiply_vector(m_projection_matrix, clipped[n].point_at(1)));
                        tri_projected.set_point_at(2, Matrix4x4::multiply_vector(m_projection_matrix, clipped[n].point_at(2)));
                        tri_projected.set_color(clipped[n].color());

                        // Scale into view, we moved the normalising into cartesian space
                        // out of the matrix.vector function from the previous videos, so
                        // do this manually

                        tri_projected.set_point_at(0, Vec4::divide(tri_projected.point_at(0), tri_projected.point_at(0).w()));
                        tri_projected.set_point_at(1, Vec4::divide(tri_projected.point_at(1), tri_projected.point_at(1).w()));
                        tri_projected.set_point_at(2, Vec4::divide(tri_projected.point_at(2), tri_projected.point_at(2).w()));

                        // X/Y are inverted so put them back

                        tri_projected.set_x_on_point_at(0, tri_projected.point_at(0).x() * -1.0f);
                        tri_projected.set_x_on_point_at(1, tri_projected.point_at(1).x() * -1.0f);
                        tri_projected.set_x_on_point_at(2, tri_projected.point_at(2).x() * -1.0f);
                        tri_projected.set_y_on_point_at(0, tri_projected.point_at(0).y() * -1.0f);
                        tri_projected.set_y_on_point_at(1, tri_projected.point_at(1).y() * -1.0f);
                        tri_projected.set_y_on_point_at(2, tri_projected.point_at(2).y() * -1.0f);

                        // Offset verts into visible normalised space

                        const auto offset_view = Vec4(1.0f, 1.0f, 0.0f, 1.0f);

                        tri_projected.set_point_at(0, Vec4::add(tri_projected.point_at(0), offset_view));
                        tri_projected.set_point_at(1, Vec4::add(tri_projected.point_at(1), offset_view));
                        tri_projected.set_point_at(2, Vec4::add(tri_projected.point_at(2), offset_view));

                        tri_projected.set_x_on_point_at(0, tri_projected.point_at(0).x() * 0.5f * m_width);
                        tri_projected.set_y_on_point_at(0, tri_projected.point_at(0).y() * 0.5f * m_height);
                        tri_projected.set_x_on_point_at(1, tri_projected.point_at(1).x() * 0.5f * m_width);
                        tri_projected.set_y_on_point_at(1, tri_projected.point_at(1).y() * 0.5f * m_height);
                        tri_projected.set_x_on_point_at(2, tri_projected.point_at(2).x() * 0.5f * m_width);
                        tri_projected.set_y_on_point_at(2, tri_projected.point_at(2).y() * 0.5f * m_height);

                        // Store triangle for sorting

                        triangles.push_back(Triangle(tri_projected));
                    }
                }
            }
        }

        // Sort triangles from back to front

        std::sort(triangles.begin(), triangles.end(), [](const Triangle &a, const Triangle &b)
                  {
            const auto z1 = (a.point_at(0).z() + a.point_at(1).z() + a.point_at(2).z()) / 3.0f;
            const auto z2 = (b.point_at(0).z() + b.point_at(1).z() + b.point_at(2).z()) / 3.0f;

            return z1 > z2; });

        ///

        SDL_FPoint zero_text_coord = {};

        ///

        SDL_SetRenderDrawColor(m_renderer, 0x29, 0x23, 0x2a, 0xff); // hybrid 2

        SDL_RenderClear(m_renderer);

        SDL_SetRenderDrawColor(m_renderer, 0xff, 0x00, 0x00, 0xff); // line color

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

        ///

        for (const auto triangle_to_raster : triangles)
        {
            Triangle clipped[2] = {Triangle{}, Triangle{}};

            ///

            std::vector<Triangle> inner_triangles;

            inner_triangles.push_back(triangle_to_raster);

            ///

            auto new_triangles = 1;

            for (auto p = 0; p < 4; p++)
            {
                auto tris_to_add = 0;

                while (new_triangles > 0)
                {
                    const auto test = inner_triangles.front();

                    inner_triangles.erase(inner_triangles.begin());

                    new_triangles--;

                    switch (p)
                    {
                    case 0:
                    {
                        tris_to_add = Triangle::clip_against_plane({0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, test, &clipped[0], &clipped[1]);

                        break;
                    }

                    case 1:
                    {
                        tris_to_add = Triangle::clip_against_plane({0.0f, m_height - 1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f}, test, &clipped[0], &clipped[1]);

                        break;
                    }

                    case 2:
                    {
                        tris_to_add = Triangle::clip_against_plane({0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, test, &clipped[0], &clipped[1]);

                        break;
                    }

                    case 3:
                    {
                        tris_to_add = Triangle::clip_against_plane({m_width - 1.0f, 0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, test, &clipped[0], &clipped[1]);

                        break;
                    }
                    }

                    for (auto w = 0; w < tris_to_add; w++)
                    {
                        inner_triangles.push_back(Triangle{clipped[w].point_at(0), clipped[w].point_at(1), clipped[w].point_at(2), clipped[w].color()});
                    }
                }

                new_triangles = inner_triangles.size();
            }

            for (const auto it : inner_triangles)
            {
                SDL_SetRenderDrawColor(m_renderer, 0xff, 0x00, 0x00, 0xff);

                if (m_render_wireframes)
                {
                    SDL_RenderLine(m_renderer, it.point_at(0).x(), it.point_at(0).y(), it.point_at(1).x(), it.point_at(1).y());
                    SDL_RenderLine(m_renderer, it.point_at(1).x(), it.point_at(1).y(), it.point_at(2).x(), it.point_at(2).y());
                    SDL_RenderLine(m_renderer, it.point_at(2).x(), it.point_at(2).y(), it.point_at(0).x(), it.point_at(0).y());
                }
            }
        }

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

        SDL_RenderPresent(m_renderer);

        const auto s = std::format("GameEngine - fps: {}", std::round(1.0f / elapsed));

        SDL_SetWindowTitle(m_window, s.c_str());
    }

    const Vec4 &camera() const
    {
        return m_camera;
    }

    const Vec4 &look_direction() const
    {
        return m_look_direction;
    }

    Vec4 set_camera(
        const Vec4 &camera)
    {
        m_camera = camera;
    }

    Vec4 set_look_direction(
        const Vec4 &look_direction)
    {
        m_look_direction = look_direction;
    }

private:
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;
    int m_screen_width;
    int m_screen_height;
    float m_scale;
    float m_width;
    float m_height;
    Matrix4x4 m_projection_matrix;
    Vec4 m_camera;
    Vec4 m_look_direction;
    float m_yaw;
    float m_pitch;
    float m_roll;
    float m_theta = 0.0f;
    bool m_render_wireframes = true;
    std::optional<Mesh> m_mesh;
};

int main()
{
    std::println("sdl version: {}", SDL_GetVersion());

    ///

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::println("SDL_Init Error: {}", SDL_GetError());

        return 1;
    }

    ///

    {
        // main loop

        Game game(1280, 832);

        game.on_create();

        ///

        auto quit = false;

        ///

        auto elapsed = 0.0f;

        ///

        auto t1 = SDL_GetTicks();

        auto t2 = t1;

        ///

        while (!quit)
        {
            t2 = SDL_GetTicks();

            auto elapsed = static_cast<float>(t2 - t1) / 1000.0f;

            t1 = t2;

            ///

            SDL_Event event;

            while (SDL_PollEvent(&event) && !quit)
            {
                switch (event.type)
                {
                case SDL_EVENT_MOUSE_WHEEL:
                {
                    const auto forward = Vec4::multiply(game.look_direction(), 8.0f * elapsed);

                    if (event.wheel.y > 0)
                    {
                        std::println("scroll up");

                        const auto c = Vec4::subtract(game.camera(), forward);

                        game.set_camera(Vec4(c.x(), c.y() + (8.0f * elapsed), c.z(), c.w()));
                    }
                    else if (event.wheel.y < 0)
                    {
                        std::println("scroll down");

                        const auto c = Vec4::add(game.camera(), forward);

                        game.set_camera(Vec4(c.x(), c.y() - (8.0f * elapsed), c.z(), c.w()));
                    }

                    break;
                }

                case SDL_EVENT_QUIT:
                {
                    quit = true;

                    break;
                }

                case SDL_EVENT_KEY_UP:
                {
                    // DO THING?

                    break;
                }

                default:
                {
                    break;
                }
                }
            }

            ///

            game.on_update(elapsed);
        }
    }

    ///

    SDL_Quit();

    ///

    return 0;
}
