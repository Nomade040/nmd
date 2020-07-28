#include "nmd_common.h"

bool _nmd_reserve(size_t numNewVertices, size_t numNewIndices)
{
    /* Check vertices */
    size_t futureSize = (_nmd_context.drawList.numVertices + numNewVertices) * sizeof(nmd_vertex);
    if (futureSize > _nmd_context.drawList.verticesCapacity)
    {
        const size_t newCapacity = NMD_MAX(_nmd_context.drawList.verticesCapacity * 2, futureSize);
        void* memoryBlock = NMD_REALLOC(_nmd_context.drawList.vertices, newCapacity);
        if (!memoryBlock)
            return false;

        _nmd_context.drawList.vertices = (nmd_vertex*)memoryBlock;
        _nmd_context.drawList.verticesCapacity = newCapacity;
    }

    /* Check indices */
    futureSize = (_nmd_context.drawList.numIndices + numNewIndices) * sizeof(IndexType);
    if (futureSize > _nmd_context.drawList.indicesCapacity)
    {
        const size_t newCapacity = NMD_MAX(_nmd_context.drawList.indicesCapacity * 2, futureSize);
        void* memoryBlock = NMD_REALLOC(_nmd_context.drawList.indices, newCapacity);
        if (!memoryBlock)
            return false;

        _nmd_context.drawList.indices = (IndexType*)memoryBlock;
        _nmd_context.drawList.indicesCapacity = newCapacity;
    }

    return true;
}

bool _nmd_reserve_points(size_t numNewPoints)
{
    const size_t futureSize = (_nmd_context.drawList.numPoints + numNewPoints) * sizeof(nmd_vec2);
    if (futureSize > _nmd_context.drawList.pathCapacity)
    {
        const size_t newCapacity = NMD_MAX(_nmd_context.drawList.pathCapacity * 2, futureSize);
        void* memoryBlock = NMD_REALLOC(_nmd_context.drawList.path, newCapacity);
        if (!memoryBlock)
            return false;

        _nmd_context.drawList.path = (nmd_vec2*)memoryBlock;
        _nmd_context.drawList.pathCapacity = newCapacity;
    }

    return true;
}

void nmd_add_polyline(const nmd_vec2* points, size_t numPoints, nmd_color color, bool closed, float thickness)
{
    if (numPoints < 2)
        return;

    const size_t numInterations = closed ? numPoints : numPoints - 1;
    if (!_nmd_reserve(4 * numPoints, 6 * numPoints))
        return;

    const float halfThickness = (thickness * 0.5f);

    for (size_t i = 0; i < numInterations; i++)
    {
        const size_t offset = _nmd_context.drawList.numVertices;

        /* Add indices */
        IndexType* indices = _nmd_context.drawList.indices + _nmd_context.drawList.numIndices;
        indices[0] = offset + 0; indices[1] = offset + 1; indices[2] = offset + 2;
        indices[3] = offset + 0; indices[4] = offset + 2; indices[5] = offset + 3;
        _nmd_context.drawList.numIndices += 6;

        const nmd_vec2* p0_tmp = &points[i];
        const nmd_vec2* p1_tmp = &points[(i + 1) == numPoints ? 0 : i + 1];
        const float dx = p1_tmp->x - p0_tmp->x;
        const float dy = p1_tmp->y - p0_tmp->y;

        /* If we didn't swap the points in this case the triangles would be drawn in the counter clockwise direction, which may cause issues in some rendering APIs. */
        const bool swapPoints = (dx < 0.0f || dy < 0.0f) || (dx > 0.0f && dy > 0.0f);
        const nmd_vec2* p0 = swapPoints ? p1_tmp : p0_tmp;
        const nmd_vec2* p1 = swapPoints ? p0_tmp : p1_tmp;

        nmd_vertex* vertices = _nmd_context.drawList.vertices + _nmd_context.drawList.numVertices;

        if (dy == 0) /* Horizontal line */
        {
            const int factor = dx > 0.0f ? 1 : -1;
            vertices[0].pos.x = p0->x - halfThickness * factor; vertices[0].pos.y = p0->y - halfThickness; vertices[0].color = color;
            vertices[1].pos.x = p1->x + halfThickness * factor; vertices[1].pos.y = p1->y - halfThickness; vertices[1].color = color;
            vertices[2].pos.x = p1->x + halfThickness * factor; vertices[2].pos.y = p1->y + halfThickness; vertices[2].color = color;
            vertices[3].pos.x = p0->x - halfThickness * factor; vertices[3].pos.y = p0->y + halfThickness; vertices[3].color = color;
        }
        else if (dx == 0) /* Vertical line */
        {
            const int factor = dy > 0.0f ? 1 : -1;
            vertices[0].pos.x = p0->x + halfThickness; vertices[0].pos.y = p0->y - halfThickness * factor; vertices[0].color = color;
            vertices[1].pos.x = p1->x + halfThickness; vertices[1].pos.y = p1->y + halfThickness * factor; vertices[1].color = color;
            vertices[2].pos.x = p1->x - halfThickness; vertices[2].pos.y = p1->y + halfThickness * factor; vertices[2].color = color;
            vertices[3].pos.x = p0->x - halfThickness; vertices[3].pos.y = p0->y - halfThickness * factor; vertices[3].color = color;
        }
        else /* Inclined line */
        {
            const float lineWidth = NMD_SQRT(dx * dx + dy * dy);

            const float cosine = dx / lineWidth;
            const float sine = dy / lineWidth;

            const float xFactor = cosine * halfThickness;
            const float yFactor = sine * halfThickness;

            vertices[0].pos.x = p0->x - yFactor; vertices[0].pos.y = p0->y + xFactor; vertices[0].color = color;
            vertices[1].pos.x = p1->x - yFactor; vertices[1].pos.y = p1->y + xFactor; vertices[1].color = color;
            vertices[2].pos.x = p1->x + yFactor; vertices[2].pos.y = p1->y - xFactor; vertices[2].color = color;
            vertices[3].pos.x = p0->x + yFactor; vertices[3].pos.y = p0->y - xFactor; vertices[3].color = color;
        }
        _nmd_context.drawList.numVertices += 4;
    }
}

void nmd_path_to(float x0, float y0)
{
    if (!_nmd_reserve_points(1))
        return;

    _nmd_context.drawList.path[_nmd_context.drawList.numPoints].x = x0;
    _nmd_context.drawList.path[_nmd_context.drawList.numPoints].y = y0;
    _nmd_context.drawList.numPoints++;
}

void nmd_path_fill_convex(nmd_color color)
{
    nmd_add_convex_polygon_filled(_nmd_context.drawList.path, _nmd_context.drawList.numPoints, color);

    /* Clear points in 'path' */
    _nmd_context.drawList.numPoints = 0;
}

void nmd_path_stroke(nmd_color color, bool closed, float thickness)
{
    nmd_add_polyline(_nmd_context.drawList.path, _nmd_context.drawList.numPoints, color, closed, thickness);

    /* Clear points in 'path' */
    _nmd_context.drawList.numPoints = 0;
}

//void PathBezierToCasteljau(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, size_t level)
//{
//    const float dx = x4 - x1;
//    const float dy = y4 - y1;
//    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
//    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
//    d2 = (d2 >= 0) ? d2 : -d2;
//    d3 = (d3 >= 0) ? d3 : -d3;
//    if ((d2 + d3) * (d2 + d3) < GetContext().drawList.curveTessellationTolerance * (dx * dx + dy * dy))
//        GetContext().drawList.path.emplace_back(x4, y4);
//    else if (level < 10)
//    {
//        const float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
//        const float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
//        const float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
//        const float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
//        const float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
//        const float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
//        PathBezierToCasteljau(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
//        PathBezierToCasteljau(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
//    }
//}
//
//nmd_vec2 BezierCalc(nmd_vec2 p0, nmd_vec2 p1, nmd_vec2 p2, nmd_vec2 p3, float t)
//{
//    const float u = 1.0f - t;
//    const float w1 = u * u * u;
//    const float w2 = 3 * u * u * t;
//    const float w3 = 3 * u * t * t;
//    const float w4 = t * t * t;
//    return { w1 * p0.x + w2 * p1.x + w3 * p2.x + w4 * p3.x, w1 * p0.y + w2 * p1.y + w3 * p2.y + w4 * p3.y };
//}
//
///* Distribute UV over (a, b) rectangle */
//void ShadeVertsLinearUV(size_t startVertexIndex, nmd_vec2 p1, const nmd_vec2& p2, const nmd_vec2& uv1, const nmd_vec2& uv2, bool clamp)
//{
//    const nmd_vec2 size = p2 - p1;
//    const nmd_vec2 uv_size = uv2 - uv1;
//    const nmd_vec2 scale = nmd_vec2(size.x != 0.0f ? (uv_size.x / size.x) : 0.0f, size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);
//
//    Vertex* const startVertex = GetContext().drawList.vertices.data() + startVertexIndex;
//    const Vertex* const endVertex = &GetContext().drawList.vertices.back();
//    if (clamp)
//    {
//        const nmd_vec2 min = nmd_vec2::Min(uv1, uv2), max = nmd_vec2::Max(uv1, uv2);
//        for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
//            vertex->uv = nmd_vec2::Clamp(uv1 + ((nmd_vec2(vertex->pos.x, vertex->pos.y) - p1) * scale), min, max);
//    }
//    else
//    {
//        for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
//            vertex->uv = uv1 + ((nmd_vec2(vertex->pos.x, vertex->pos.y) - p1) * scale);
//    }
//}
//
//void DrawList::PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId)
//{
//    if (!drawCommands.empty() && drawCommands.back().userTextureId == userTextureId)
//        drawCommands.back().numVertices += static_cast<IndexType>(numVertices), drawCommands.back().numIndices += static_cast<IndexType>(numIndices);
//    else
//        drawCommands.emplace_back(static_cast<IndexType>(numVertices), static_cast<IndexType>(numIndices), userTextureId);
//}
//

void nmd_add_rect(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t cornerFlags, float thickness)
{
    if (!color.a || thickness == 0.0f)
        return;
    
    nmd_path_rect(x0, y0, x1, y1, rounding, cornerFlags);

    nmd_path_stroke(color, true, thickness);
}

void nmd_add_rect_filled(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t cornerFlags)
{
    if (!color.a)
        return;

    if (rounding > 0.0f)
    {
        nmd_path_rect(x0, y0, x1, y1, rounding, cornerFlags);
        nmd_path_fill_convex(color);
    }
    else
    {
        if (!_nmd_reserve(4, 6))
            return;

        const size_t offset = _nmd_context.drawList.numVertices;

        IndexType* indices = _nmd_context.drawList.indices + _nmd_context.drawList.numIndices;
        indices[0] = offset + 0; indices[1] = offset + 1; indices[2] = offset + 2;
        indices[3] = offset + 0; indices[4] = offset + 2; indices[5] = offset + 3;
        _nmd_context.drawList.numIndices += 6;

        nmd_vertex* vertices = _nmd_context.drawList.vertices + _nmd_context.drawList.numVertices;
        vertices[0].pos.x = x0; vertices[0].pos.y = y0; vertices[0].color = color;
        vertices[1].pos.x = x1; vertices[1].pos.y = y0; vertices[1].color = color;
        vertices[2].pos.x = x1; vertices[2].pos.y = y1; vertices[2].color = color;
        vertices[3].pos.x = x0; vertices[3].pos.y = y1; vertices[3].color = color;
        _nmd_context.drawList.numVertices += 4;
    }
}

void nmd_add_rect_filled_multi_color(float x0, float y0, float x1, float y1, nmd_color colorUpperLeft, nmd_color colorUpperRight, nmd_color colorBottomRight, nmd_color colorBottomLeft)
{
    if (!_nmd_reserve(4, 6))
        return;

    const size_t offset = _nmd_context.drawList.numVertices;

    IndexType* indices = _nmd_context.drawList.indices + _nmd_context.drawList.numIndices;
    indices[0] = offset + 0; indices[1] = offset + 1; indices[2] = offset + 2;
    indices[3] = offset + 0; indices[4] = offset + 2; indices[5] = offset + 3;
    _nmd_context.drawList.numIndices += 6;

    nmd_vertex* vertices = _nmd_context.drawList.vertices + _nmd_context.drawList.numVertices;
    vertices[0].pos.x = x0; vertices[0].pos.y = y0; vertices[0].color = colorUpperLeft;
    vertices[1].pos.x = x1; vertices[1].pos.y = y0; vertices[1].color = colorUpperRight;
    vertices[2].pos.x = x1; vertices[2].pos.y = y1; vertices[2].color = colorBottomRight;
    vertices[3].pos.x = x0; vertices[3].pos.y = y1; vertices[3].color = colorBottomLeft;
    _nmd_context.drawList.numVertices += 4;
}

void nmd_add_quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color, float thickness)
{
    if (!color.a)
        return;

    nmd_path_to(x0, y0);
    nmd_path_to(x1, y1);
    nmd_path_to(x2, y2);
    nmd_path_to(x3, y3);

    nmd_path_stroke(color, true, thickness);
}

void nmd_add_quad_filled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color)
{
    if (!color.a)
        return;

    nmd_path_to(x0, y0);
    nmd_path_to(x1, y1);
    nmd_path_to(x2, y2);
    nmd_path_to(x3, y3);

    nmd_path_fill_convex(color);
}

void nmd_add_triangle(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color, float thickness)
{
    if (!color.a)
        return;
    
    nmd_path_to(x0, y0);
    nmd_path_to(x1, y1);
    nmd_path_to(x2, y2);

    nmd_path_stroke(color, true, thickness);
}

void nmd_add_triangle_filled(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color)
{
    if (!color.a || !_nmd_reserve(3, 3))
        return;

    const size_t offset = _nmd_context.drawList.numVertices;

    IndexType* indices = _nmd_context.drawList.indices + _nmd_context.drawList.numIndices;
    indices[0] = offset + 0;
    indices[1] = offset + 1;
    indices[2] = offset + 2;
    _nmd_context.drawList.numIndices += 6;

    nmd_vertex* vertices = _nmd_context.drawList.vertices + _nmd_context.drawList.numVertices;
    vertices[0].pos.x = x0; vertices[0].pos.y = y0; vertices[0].color = color;
    vertices[1].pos.x = x1; vertices[1].pos.y = y1; vertices[1].color = color;
    vertices[2].pos.x = x2; vertices[2].pos.y = y2; vertices[2].color = color;
    _nmd_context.drawList.numVertices += 4;
}

void nmd_add_circle(float x0, float y0, float radius, nmd_color color, size_t numSegments, float thickness)
{
    if (!color.a || radius <= 0.0f)
        return;

    if (numSegments == 0)
        numSegments = (radius - 1 < 64) ? _nmd_context.drawList.cachedCircleSegmentCounts64[(int)radius - 1] : NMD_CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
    else
        numSegments = NMD_CLAMP(numSegments, 3, NMD_CIRCLE_AUTO_SEGMENT_MAX);

    if (numSegments == 12)
        nmd_path_arc_to_cached(x0, y0, radius - 0.5f, 0, 12, false);
    else
        nmd_path_arc_to(x0, y0, radius - 0.5f, 0.0f, NMD_2PI * ((numSegments - 1) / (float)numSegments), numSegments - 1, false);

    nmd_path_stroke(color, true, thickness);
}

void nmd_add_circle_filled(float x0, float y0, float radius, nmd_color color, size_t numSegments)
{
    if (!color.a || radius <= 0.0f)
        return;

    if (numSegments <= 0)
        numSegments = (radius - 1 < 64) ? _nmd_context.drawList.cachedCircleSegmentCounts64[(int)radius - 1] : NMD_CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
    else
        numSegments = NMD_CLAMP(numSegments, 3, NMD_CIRCLE_AUTO_SEGMENT_MAX);

    if (numSegments == 12)
        nmd_path_arc_to_cached(x0, y0, radius, 0, 12, false);
    else
        nmd_path_arc_to(x0, y0, radius, 0.0f, NMD_2PI * ((numSegments - 1.0f) / (float)numSegments), numSegments - 1, false);
        
    nmd_path_fill_convex(color);
}

void nmd_add_ngon(float x0, float y0, float radius, nmd_color color, size_t numSegments, float thickness)
{
    if (!color.a || numSegments < 3)
        return;

    /* remove one(1) from numSegment because it's a closed shape. */
    nmd_path_arc_to(x0, y0, radius, 0.0f, NMD_2PI * ((numSegments - 1) / (float)numSegments), numSegments - 1, false);
    nmd_path_stroke(color, true, thickness);
}

void nmd_add_ngon_filled(float x0, float y0, float radius, nmd_color color, size_t numSegments)
{
    if (!color.a || numSegments < 3)
        return;

    /* remove one(1) from numSegment because it's a closed shape. */
    nmd_path_arc_to(x0, y0, radius, 0.0f, NMD_2PI * ((numSegments - 1) / (float)numSegments), numSegments - 1, false);
    nmd_path_fill_convex(color);
}

void nmd_path_rect(float x0, float y0, float x1, float y1, float rounding, uint32_t cornerFlags)
{
    if (rounding == 0.0f || cornerFlags == 0)
    {
        nmd_path_to(x0, y0);
        nmd_path_to(x1, y0);
        nmd_path_to(x1, y1);
        nmd_path_to(x0, y1);
    }
    else
    {
        const float roundingTopLeft = (cornerFlags & NMD_CORNER_TOP_LEFT) ? rounding : 0.0f;
        const float roundingTopRight = (cornerFlags & NMD_CORNER_TOP_RIGHT) ? rounding : 0.0f;
        const float roundingBottomRight = (cornerFlags & NMD_CORNER_BOTTOM_RIGHT) ? rounding : 0.0f;
        const float roundingBottomLeft = (cornerFlags & NMD_CORNER_BOTTOM_LEFT) ? rounding : 0.0f;
        nmd_path_arc_to_cached(x0 + roundingTopLeft, y0 + roundingTopLeft, roundingTopLeft, 6, 9, false);
        nmd_path_arc_to_cached(x1 - roundingTopRight, y0 + roundingTopRight, roundingTopRight, 9, 12, false);
        nmd_path_arc_to_cached(x1 - roundingBottomRight, y1 - roundingBottomRight, roundingBottomRight, 0, 3, false);
        nmd_path_arc_to_cached(x0 + roundingBottomLeft, y1 - roundingBottomLeft, roundingBottomLeft, 3, 6, false);
    }
}

void nmd_path_arc_to(float x0, float y0, float radius, float startAngle, float endAngle, size_t numSegments, bool startAtCenter)
{
    if (!_nmd_reserve_points((startAtCenter ? 1 : 0) + numSegments))
        return;

    nmd_vec2* path = _nmd_context.drawList.path + _nmd_context.drawList.numPoints;

    if (startAtCenter)
    {
        path->x = x0, path->y = y0;
        path++;
    }

    for (size_t i = 0; i <= numSegments; i++)
    {
        const float angle = startAngle + (i / (float)numSegments) * (endAngle - startAngle);
        path[i].x = x0 + NMD_COS(angle) * radius;
        path[i].y = y0 + NMD_SIN(angle) * radius;
    }

    _nmd_context.drawList.numPoints = (startAtCenter ? 1 : 0) + numSegments + 1;
}

void nmd_path_arc_to_cached(float x0, float y0, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter)
{
    if (!_nmd_reserve_points((startAtCenter ? 1 : 0) + (endAngleOf12 - startAngleOf12)))
        return;

    nmd_vec2* path = _nmd_context.drawList.path + _nmd_context.drawList.numPoints;

    if (startAtCenter)
    {
        path->x = x0, path->y = y0;
        path++;
    }

    for (size_t angle = startAngleOf12; angle <= endAngleOf12; angle++)
    {
        const nmd_vec2* point = &_nmd_context.drawList.cachedCircleVertices12[angle % 12];
        path->x = x0 + point->x * radius;
        path->y = y0 + point->y * radius;
        path++;
    }

    _nmd_context.drawList.numPoints = path - _nmd_context.drawList.path;
}

//void DrawList::PathBezierCurveTo(const nmd_vec2& p2, const nmd_vec2& p3, const nmd_vec2& p4, size_t numSegments)
//{
//    const nmd_vec2& p1 = path.back();
//    if (numSegments == 0)
//        PathBezierToCasteljau(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, 0);
//    else
//    {
//        const float tStep = 1.0f / static_cast<float>(numSegments);
//        for (size_t iStep = 1; iStep <= numSegments; iStep++)
//            path.push_back(BezierCalc(p1, p2, p3, p4, tStep * iStep));
//    }
//}
//
//void DrawList::PrimRectUV(const nmd_vec2& p1, const nmd_vec2& p2, const nmd_vec2& uv1, const nmd_vec2& uv2, Color color)
//{
//    const IndexType nextIndex = static_cast<IndexType>(vertices.size());
//
//    vertices.emplace_back(p1, color, uv1);
//    vertices.emplace_back(nmd_vec2(p2.x, p1.y), color, nmd_vec2(uv2.x, uv1.y));
//    vertices.emplace_back(p2, color, uv2);
//    vertices.emplace_back(nmd_vec2(p1.x, p2.y), color, nmd_vec2(uv1.x, uv2.y));
//
//    indices.push_back(nextIndex + 0);
//    indices.push_back(nextIndex + 1);
//    indices.push_back(nextIndex + 2);
//
//    indices.push_back(nextIndex + 0);
//    indices.push_back(nextIndex + 2);
//    indices.push_back(nextIndex + 3);
//}
//
//void DrawList::PrimQuadUV(const nmd_vec2& p1, const nmd_vec2& p2, const nmd_vec2& p3, const nmd_vec2& p4, const nmd_vec2& uv1, const nmd_vec2& uv2, const nmd_vec2& uv3, const nmd_vec2& uv4, Color color)
//{
//    const IndexType nextIndex = static_cast<IndexType>(vertices.size());
//
//    vertices.emplace_back(p1, color, uv1);
//    vertices.emplace_back(p2, color, uv2);
//    vertices.emplace_back(p3, color, uv3);
//    vertices.emplace_back(p4, color, uv4);
//
//    indices.push_back(nextIndex + 0);
//    indices.push_back(nextIndex + 1);
//    indices.push_back(nextIndex + 2);
//
//    indices.push_back(nextIndex + 0);
//    indices.push_back(nextIndex + 2);
//    indices.push_back(nextIndex + 3);
//}
//
void nmd_add_line(float x0, float y0, float x1, float y1, nmd_color color, float thickness)
{
    if (!color.a)
        return;

    nmd_path_to(x0, y0);
    nmd_path_to(x1, y1);
    nmd_path_stroke(color, false, thickness);
}

void nmd_add_convex_polygon_filled(const nmd_vec2* points, size_t numPoints, nmd_color color)
{
    if (numPoints < 3 || !_nmd_reserve(numPoints, (numPoints - 2) * 3))
        return;

    const size_t offset = _nmd_context.drawList.numVertices;
    IndexType* indices = _nmd_context.drawList.indices + _nmd_context.drawList.numIndices;
    for (size_t i = 2; i < numPoints; i++)
        indices[(i - 2) * 3 + 0] = offset, indices[(i - 2) * 3 + 1] = offset + (i - 1), indices[(i - 2) * 3 + 2] = offset + i;
    _nmd_context.drawList.numIndices += (numPoints - 2) * 3;

    nmd_vertex* vertices = _nmd_context.drawList.vertices + _nmd_context.drawList.numVertices;
    for (size_t i = 0; i < numPoints; i++)
        vertices[i].pos.x = points[i].x, vertices[i].pos.y = points[i].y, vertices[i].color = color;
    _nmd_context.drawList.numVertices += numPoints;
}

//void DrawList::AddBezierCurve(const nmd_vec2& p1, const nmd_vec2& p2, const nmd_vec2& p3, const nmd_vec2& p4, Color color, float thickness, size_t numSegments)
//{
//    if (!color.a)
//        return;
//
//    PathLineTo(p1);
//    PathBezierCurveTo(p2, p3, p4, numSegments);
//    PathStroke(color, false, thickness);
//}
//
//void DrawList::AddText(const nmd_vec2& pos, Color color, const char* text, size_t textLength)
//{
//    AddText(NULL, 0.0f, pos, color, text, textLength);
//}
//
//void DrawList::AddText(const void* font, float fontSize, const nmd_vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth)
//{
//    if (!color.a)
//        return;
//    
//    const char* const textEnd = text + textLength;
//    
//    float x = pos.x;
//    float y = pos.y;
//
//    while (text < textEnd)
//    {
//        //const Glyph* glyph = font->FindGlyph(*text);
//    
//        //stbtt_aligned_quad q;
//        //stbtt_GetBakedQuad(bdata, 512, 512, *text - 32, &x, &y, &q, 0);
//        //
//        //const size_t nextIndex = vertices.size();
//        //vertices.emplace_back(nmd_vec2(glyph->x0, glyph->y0), color, nmd_vec2(glyph->u0, glyph->v0));
//        //vertices.emplace_back(nmd_vec2(glyph->x1, glyph->y0), color, nmd_vec2(glyph->u1, glyph->v0));
//        //vertices.emplace_back(nmd_vec2(glyph->x1, glyph->y1), color, nmd_vec2(glyph->u1, glyph->v1));
//        //vertices.emplace_back(nmd_vec2(glyph->x0, glyph->y1), color, nmd_vec2(glyph->u0, glyph->v1));
//    
//        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
//
//        indices.push_back(nextIndex + 0);
//        indices.push_back(nextIndex + 1);
//        indices.push_back(nextIndex + 2);
//    
//        indices.push_back(nextIndex + 0);
//        indices.push_back(nextIndex + 2);
//        indices.push_back(nextIndex + 3);
//    
//        text++;
//    }
//}
//
//void DrawList::AddImage(TextureId userTextureId, const nmd_vec2& p1, const nmd_vec2& p2, const nmd_vec2& uv1, const nmd_vec2& uv2, Color color)
//{
//    if (!color.a)
//        return;
//
//    PushRemainingDrawCommands();
//
//    PrimRectUV(p1, p2, uv1, uv2, color);
//
//    PushTextureDrawCommand(4, 6, userTextureId);
//}
//
//void DrawList::AddImageQuad(TextureId userTextureId, const nmd_vec2& p1, const nmd_vec2& p2, const nmd_vec2& p3, const nmd_vec2& p4, const nmd_vec2& uv1, const nmd_vec2& uv2, const nmd_vec2& uv3, const nmd_vec2& uv4, Color color)
//{
//    if (!color.a)
//        return;
//
//    PushRemainingDrawCommands();
//
//    PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, color);
//
//    PushTextureDrawCommand(4, 6, userTextureId);
//}
//
//void DrawList::AddImageRounded(TextureId userTextureId, const nmd_vec2& p1, const nmd_vec2& p2, float rounding, uint32_t cornerFlags, const nmd_vec2& uv1, const nmd_vec2& uv2, Color color)
//{
//    if (!color.a)
//        return;
//
//    if (rounding <= 0.0f || !cornerFlags)
//        AddImage(userTextureId, p1, p2, uv1, uv2, color);
//    else
//    {
//        PushRemainingDrawCommands();
//
//        PathRect(p1, p2, rounding, cornerFlags);
//
//        const size_t v0 = vertices.size(), i0 = indices.size();
//        PathFillConvex(color);
//        ShadeVertsLinearUV(v0, p1, p2, uv1, uv2, true);
//
//        PushTextureDrawCommand(vertices.size() - v0, indices.size() - i0, userTextureId);
//    }
//}