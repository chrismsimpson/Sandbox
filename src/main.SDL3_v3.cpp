
#include <print>
#include <optional>
#include <vector>
#include <variant>
#include <istream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

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

    Vec4(float x, float y, float z, float w)
        : m_x(x), m_y(y), m_z(z), m_w(w) {}

    Vec4(
        const Vec4 &other)
        : m_x(other.m_x), m_y(other.m_y), m_z(other.m_z), m_w(other.m_w) {}

    Vec4(
        Vec4 &&other)
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
            lhs.m_z + rhs.m_z,
            lhs.m_w + rhs.m_w);
    }

    static Vec4 subtract(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return Vec4(
            lhs.m_x - rhs.m_x,
            lhs.m_y - rhs.m_y,
            lhs.m_z - rhs.m_z,
            lhs.m_w - rhs.m_w);
    }

    static Vec4 multiply(
        const Vec4 &lhs,
        float k)
    {
        return Vec4(
            lhs.m_x * k,
            lhs.m_y * k,
            lhs.m_z * k,
            lhs.m_w * k);
    }

    static Vec4 divide(
        const Vec4 &lhs,
        float k)
    {
        return Vec4(
            lhs.m_x / k,
            lhs.m_y / k,
            lhs.m_z / k,
            lhs.m_w / k);
    }

    static float dot_product(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return lhs.m_x * rhs.m_x +
               lhs.m_y * rhs.m_y +
               lhs.m_z * rhs.m_z +
               lhs.m_w * rhs.m_w;
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
            vec.m_z / len,
            vec.m_w / len);
    }

    static Vec4 cross_product(
        const Vec4 &lhs,
        const Vec4 &rhs)
    {
        return Vec4(
            lhs.m_y * rhs.m_z - lhs.m_z * rhs.m_y,
            lhs.m_z * rhs.m_x - lhs.m_x * rhs.m_z,
            lhs.m_x * rhs.m_y - lhs.m_y * rhs.m_x,
            1.0f);
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
        : m_points{Vec4(0.0f, 0.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f)}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
        : m_points{Vec4(x1, y1, z1, 1.0f), Vec4(x2, y2, z2, 1.0f), Vec4(x3, y3, z3, 1.0f)}, m_color{0xff, 0xff, 0xff, 0xff}
    {
    }

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, const SDL_Color &color)
        : m_points{Vec4(x1, y1, z1, 1.0f), Vec4(x2, y2, z2, 1.0f), Vec4(x3, y3, z3, 1.0f)}, m_color(color)
    {
    }

    Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, SDL_Color &&color)
        : m_points{Vec4(x1, y1, z1, 1.0f), Vec4(x2, y2, z2, 1.0f), Vec4(x3, y3, z3, 1.0f)}, m_color(std::move(color))
    {
    }

    Triangle(const Vec4 &p1, const Vec4 &p2, const Vec4 &p3)
        : m_points{p1, p2, p3}, m_color{0xff, 0xff, 0xff, 0xff} {}

    Triangle(const Vec4 &p1, const Vec4 &p2, const Vec4 &p3, const SDL_Color &color)
        : m_points{p1, p2, p3}, m_color(color) {}

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

    static float clip_against_plane(
        const Vec4 &plane_p,
        const Vec4 &plane_n,
        Triangle &in_tri,
        Triangle *out_tri1,
        Triangle *out_tri2)
    {
        // Make sure plane normal is indeed normal

        const auto _plane_n = Vec4::normalize(plane_n);

        // Create two temporary storage arrays to classify points either side of plane
        // If distance sign is positive, point lies on "inside" of plane

        Vec4 inside_points[3];
        auto inside_point_count = 0;

        Vec4 outside_points[3];
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

            return 0.0f; // No returned triangles are valid
        }

        if (inside_point_count == 3)
        {
            // All points lie on the inside of plane, so do nothing
            // and allow the triangle to simply pass through

            Triangle out_tri1(in_tri);

            return 1.0f; // Just the one returned original triangle is valid
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

            return 1.0f; // Return the newly formed single triangle
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

            return 2.0f; // Return two newly formed triangles which form a quad
        }

        return 0.0f; // No returned triangles are valid
    }

private:
    Vec4 m_points[3];
    SDL_Color m_color;
};

class Mesh
{
public:
    Mesh(
        const std::vector<Triangle> &triangles)
        : m_triangles(triangles) {}

    Mesh(
        std::vector<Triangle> &&triangles)
        : m_triangles(std::move(triangles)) {}

    Mesh(
        const Mesh &other)
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
        std::vector<Triangle> triangles;

        const auto stride = std::sqrt(height_map.size()) - 1;

        for (auto y = 0; y < stride; y++)
        {
            for (auto x = 0; x < stride; x++)
            {
                const auto &i = y * stride + x;

                const auto h1 = height_map[i];
                const auto h2 = height_map[i + 1];
                const auto h3 = height_map[i + stride];

                const auto p1 = Vec4(x, h1, y, 1.0f);
                const auto p2 = Vec4(x + 1, h2, y, 1.0f);
                const auto p3 = Vec4(x, h3, y + 1, 1.0f);

                const auto c = Mesh::color_given_heights(h1, h2, h3);

                triangles.push_back(Triangle(p1, p2, p3, c));

                const auto h4 = height_map[i + 1];
                const auto h5 = height_map[i + stride];
                const auto h6 = height_map[i + stride + 1];

                const auto p4 = Vec4(x + 1, h4, y, 1.0f);
                const auto p5 = Vec4(x, h5, y + 1, 1.0f);
                const auto p6 = Vec4(x + 1, h6, y + 1, 1.0f);

                const auto c2 = Mesh::color_given_heights(h4, h5, h6);

                triangles.push_back(Triangle(p4, p5, p6, c2));
            }
        }

        return Mesh(triangles);
    }

    const std::vector<Triangle> &triangles() const { return m_triangles; }

private:
    std::vector<Triangle> m_triangles;
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

        m_scale = 2.0f;

        m_render_scale = 1.0f;

        m_width = static_cast<float>(m_screen_width) * m_scale;
        m_height = static_cast<float>(m_screen_height) * m_scale;

        ///

        std::println("creating window");

        m_window = SDL_CreateWindow("Game", m_screen_width, m_screen_height, SDL_WINDOW_HIGH_PIXEL_DENSITY);

        // SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER

        m_renderer = SDL_CreateRenderer(m_window, nullptr);

        // m_renderer = SDL_CreateRendererWithProperties(m_window, -1, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER);

        SDL_SetRenderVSync(m_renderer, SDL_FALSE);

        // SDL_SetRenderScale(m_renderer, 2.0f, 2.0f);

        SDL_RaiseWindow(m_window);

        ///
    }

    ~Game()
    {
        std::println("destroying window");

        if (m_renderer)
        {
            SDL_DestroyRenderer(m_renderer);
        }

        if (m_window)
        {
            SDL_DestroyWindow(m_window);
        }
    }

    void on_update(
        const float &elapsed)
    {
        std::println("elapsed: {}", elapsed);

        ///

        SDL_SetRenderVSync(m_renderer, SDL_FALSE);

        // SDL_SetRenderDrawColor(m_renderer, 0x00, 0x00, 0x00, 0xff); // black
        // SDL_SetRenderDrawColor(m_renderer, 0x47, 0x1e, 0x00, 0xff); // sc2k void
        // SDL_SetRenderDrawColor(m_renderer, 0x23, 0x0f, 0x00, 0xff); // sc2k void * 50%, black * 50%
        // SDL_SetRenderDrawColor(m_renderer, 0x35, 0x37, 0x4c, 0xff); // sc4 void
        // SDL_SetRenderDrawColor(m_renderer, 0x2d, 0x22, 0x24, 0xff); // hybrid
        SDL_SetRenderDrawColor(m_renderer, 0x29, 0x23, 0x2a, 0xff); // hybrid 2

        SDL_RenderClear(m_renderer);

        SDL_SetRenderDrawColor(m_renderer, 0xff, 0x00, 0x00, 0xff); // line color

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

        ///

        // SDL_Vertex vertices[3] = {
        //     {50, 0},
        //     {100, 100},
        //     {0, 100}};

        // SDL_RenderGeometry(m_renderer, nullptr, vertices, 3, nullptr, 0);

        // SDL

        SDL_SetRenderDrawColor(m_renderer, 0xff, 0x00, 0x00, 0xff);

        ///

        const auto padding = 20.0f;

        ///

        const auto x1 = m_width / 2.0f;
        const auto y1 = 0.0f + padding;

        const auto x2 = m_width - padding;
        const auto y2 = m_height - padding;

        const auto x3 = 0.0f + padding;
        const auto y3 = m_height - padding;

        // SDL_RenderLine(m_renderer, 50, 0, 100, 100);
        // SDL_RenderLine(m_renderer, 100, 100, 0, 100);
        // SDL_RenderLine(m_renderer, 0, 100, 50, 0);

        SDL_RenderLine(m_renderer, x1, y1, x2, y2);
        SDL_RenderLine(m_renderer, x2, y2, x3, y3);
        SDL_RenderLine(m_renderer, x3, y3, x1, y1);

        ///

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

        SDL_RenderPresent(m_renderer);

        SDL_SetWindowTitle(m_window, std::format("Game - fps: {}", std::round(1.0f / elapsed)).c_str());
    }

private:
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;
    int m_screen_width;
    int m_screen_height;
    float m_scale;
    float m_render_scale;
    float m_width;
    float m_height;
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

        Game game(800, 600);

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
                    std::println("mouse wheel");

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