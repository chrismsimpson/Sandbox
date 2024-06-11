
#pragma once

#include <utility>

struct Point
{
    int x;
    int y;
    int z;
};

class Edge
{
public:
    Edge(
        const Point &start,
        const Point &end)
        : m_start(start), m_end(end) {}

    Edge(
        Point &&start,
        Point &&end)
        : m_start(std::move(start)), m_end(std::move(end)) {}

    Edge(
        const Edge &other)
        : m_start(other.m_start), m_end(other.m_end) {}

    Edge &operator=(
        const Edge &other)
    {
        if (this != &other)
        {
            m_start = other.m_start;
            m_end = other.m_end;
        }

        return *this;
    }

    ~Edge() {}

    const Point &start() const { return m_start; }

    const Point &end() const { return m_end; }

private:
    Point m_start;
    Point m_end;
};

class Node
{
public:
    Node(
        const Point &position)
        : m_position(position) {}

    Node(
        Point &&position)
        : m_position(std::move(position)) {}

    Node(
        const Node &other)
        : m_position(other.m_position) {}

    Node &operator=(
        const Node &other)
    {
        if (this != &other)
        {
            m_position = other.m_position;
        }

        return *this;
    }

    ~Node() {}

    const Point &position() const { return m_position; }

private:
    Point m_position;
};