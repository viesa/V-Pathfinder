#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <cstring>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <jcv/jc_voronoi.h>

#include "Camera.h"
#include "IException.h"
#include "Random.h"

class Voronoi : public sf::Drawable
{
public:
    class Polygon : public sf::ConvexShape
    {
    public:
        Polygon(const sf::Vector2f &voronoiPoint, size_t pointCount = 0)
            : sf::ConvexShape(0),
              m_voronoiPoint(voronoiPoint)
        {
        }
        Polygon(const sf::ConvexShape &shape, const sf::Vector2f &voronoiPoint)
            : sf::ConvexShape(shape),
              m_voronoiPoint(voronoiPoint)
        {
        }

        void setVoronoiPoint(const sf::Vector2f &voronoiPoint) noexcept { m_voronoiPoint = voronoiPoint; }
        const sf::Vector2f &getVoronoiPoint() const noexcept { return m_voronoiPoint; }

    private:
        sf::Vector2f m_voronoiPoint;
    };

public:
    Voronoi();
    Voronoi(const sf::FloatRect &boundingBox, const std::vector<sf::Vector2f> &points);
    Voronoi(const sf::FloatRect &boundingBox, int nRandomPoints);
    ~Voronoi();

    void SetPoints(const std::vector<sf::Vector2f> &points);
    void SetBoundingBox(const sf::FloatRect &boundingBox);
    void SetFillColors(const std::vector<sf::Color> &fillColors) noexcept { m_fillColors = fillColors; }
    void SetOutlineColor(const sf::Color &color) noexcept { m_outlineColor = m_outlineColor; }
    void SetOutlineThickness(float thickness) noexcept;

    void Relax(int iterations = 1);

    const std::vector<Voronoi::Polygon> &GetPolygons() const noexcept { return m_polygons; }
    sf::ConvexShape &GetPolygon(const sf::Vector2f &position);

protected:
    void GenerateVoronoi();
    static jcv_rect ConvertBoundingBox(const sf::FloatRect &boundingBox);

private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    std::optional<jcv_diagram> m_diagram;
    sf::FloatRect m_boundingBox;
    std::vector<sf::Vector2f> m_points;
    std::vector<Voronoi::Polygon> m_polygons;

    std::vector<sf::Color> m_fillColors;
    sf::Color m_outlineColor;

public:
    class Exception : public IException
    {
    public:
        Exception(int line, const char *file, const char *errorString) noexcept;
        const char *what() const noexcept override;
        virtual const char *GetType() const noexcept override;
        const char *GetErrorString() const noexcept;

    private:
        std::string errorString;
    };
};