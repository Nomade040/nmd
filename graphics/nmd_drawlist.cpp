#include "nmd_common.hpp"
#include "stb_truetype.h"

namespace nmd
{
    void PathBezierToCasteljau(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, size_t level)
    {
        const float dx = x4 - x1;
        const float dy = y4 - y1;
        float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
        float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
        d2 = (d2 >= 0) ? d2 : -d2;
        d3 = (d3 >= 0) ? d3 : -d3;
        if ((d2 + d3) * (d2 + d3) < GetContext().drawList.curveTessellationTolerance * (dx * dx + dy * dy))
            GetContext().drawList.path.emplace_back(x4, y4);
        else if (level < 10)
        {
            const float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            const float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            const float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
            const float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            const float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
            const float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
            PathBezierToCasteljau(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
            PathBezierToCasteljau(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
        }
    }

    Vec2 BezierCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float t)
    {
        const float u = 1.0f - t;
        const float w1 = u * u * u;
        const float w2 = 3 * u * u * t;
        const float w3 = 3 * u * t * t;
        const float w4 = t * t * t;
        return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
    }

    // Distribute UV over (a, b) rectangle
    void ShadeVertsLinearUV(size_t startVertexIndex, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, bool clamp)
    {
        const Vec2 size = p2 - p1;
        const Vec2 uv_size = uv2 - uv1;
        const Vec2 scale = Vec2(size.x != 0.0f ? (uv_size.x / size.x) : 0.0f, size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

        Vertex* const startVertex = GetContext().drawList.vertices.data() + startVertexIndex;
        const Vertex* const endVertex = &GetContext().drawList.vertices.back();
        if (clamp)
        {
            const Vec2 min = Vec2::Min(uv1, uv2), max = Vec2::Max(uv1, uv2);
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = Vec2::Clamp(uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale), min, max);
        }
        else
        {
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale);
        }
    }

    DrawList::DrawList()
        : curveTessellationTolerance(1.25f)
    {
        for (size_t i = 0; i < 12; i++)
        {
            //const float angle = (i / 12.0f) * GUI_2PI;
            const float angle = (i / 6.0f) * NMD_PI; // Simplified version of the line above.
            cachedCircleVertices12[i] = Vec2(cosf(angle), sinf(angle));
        }

        CalculateCircleSegments(1.6f);
    }

    void DrawList::CalculateCircleSegments(float maxError)
    {
        for (size_t i = 0; i < 64; i++)
        {
            const uint8_t segment_count = static_cast<uint8_t>(CIRCLE_AUTO_SEGMENT_CALC(i + 1.0f, maxError));
            cachedCircleSegmentCounts64[i] = NMD_MIN(segment_count, 255);
        }
    }

    void DrawList::PushRemainingDrawCommands()
    {
        size_t numAccountedVertices = 0, numAccountedIndices = 0;
        for (auto& drawCommand : drawCommands)
            numAccountedVertices += drawCommand.numVertices, numAccountedIndices += drawCommand.numIndices;

        size_t numUnaccountedIndices = indices.size() - numAccountedIndices;

        while (numUnaccountedIndices > 0)
        {
            if (numUnaccountedIndices <= (2 << (8 * sizeof(IndexType) - 1)))
            {
                drawCommands.emplace_back(static_cast<IndexType>(vertices.size() - numAccountedVertices), static_cast<IndexType>(numUnaccountedIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices = 0;
                return;
            }
            else
            {
                size_t numIndices = (2 << (8 * sizeof(IndexType) - 1)) - 1;
                IndexType lastIndex = indices[numIndices - 1];

                bool isLastIndexReferenced = false;
                do
                {
                    for (size_t i = numIndices; i < numUnaccountedIndices; i++)
                    {
                        if (indices[i] == lastIndex)
                        {
                            isLastIndexReferenced = true;
                            numIndices -= 3;
                            lastIndex = indices[numIndices - 1];
                            break;
                        }
                    }
                } while (isLastIndexReferenced);

                drawCommands.emplace_back(static_cast<IndexType>(lastIndex + 1), static_cast<IndexType>(numIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices -= numIndices;
            }
        }
    }

    void DrawList::PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId)
    {
        if (!drawCommands.empty() && drawCommands.back().userTextureId == userTextureId)
            drawCommands.back().numVertices += static_cast<IndexType>(numVertices), drawCommands.back().numIndices += static_cast<IndexType>(numIndices);
        else
            drawCommands.emplace_back(static_cast<IndexType>(numVertices), static_cast<IndexType>(numIndices), userTextureId);
    }

    void DrawList::AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags, float thickness)
    {
        if (!color.a || thickness == 0.0f)
            return;
        
        PathRect(p1 + Vec2(0.5f, 0.5f), p2 - Vec2(0.5f, 0.5f), rounding, cornerFlags);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags)
    {
        if (!color.a)
            return;

        if (rounding > 0.0f)
        {
            PathRect(p1, p2, rounding, cornerFlags);
            PathFillConvex(color);
        }
        else
            PrimRect(p1, p2, color);
    }

    void DrawList::AddRectFilledMultiColor(const Vec2& p1, const Vec2& p2, Color colorUpperLeft, Color colorUpperRight, Color colorBottomRight, Color colorBottomLeft)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, colorUpperLeft, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), colorUpperRight, Vec2());
        vertices.emplace_back(p2, colorBottomRight, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), colorBottomLeft, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathFillConvex(color);
    }

    void DrawList::AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color)
    {
        if (!color.a)
            return;

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(p3, color, Vec2());
        for (size_t i = 0; i < 3; i++)
            indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddCircle(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments == 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius - 0.5f, 0, 12);
        else
            PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * (static_cast<float>(numSegments - 1) / static_cast<float>(numSegments)), numSegments - 1);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddCircleFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments <= 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius, 0, 12);
        else
            PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);

        PathFillConvex(color);
    }

    void DrawList::AddNgon(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddNgonFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathFillConvex(color);
    }

    void DrawList::PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags)
    {
        if (rounding <= 0.0f || cornerFlags == 0)
        {
            PathLineTo(p1);
            PathLineTo(Vec2(p2.x, p1.y));
            PathLineTo(p2);
            PathLineTo(Vec2(p1.x, p2.y));
        }
        else
        {
            const float roundingTopLeft = (cornerFlags & CORNER_FLAGS_TOP_LEFT) ? rounding : 0.0f;
            const float roundingTopRight = (cornerFlags & CORNER_FLAGS_TOP_RIGHT) ? rounding : 0.0f;
            const float roundingBottomRight = (cornerFlags & CORNER_FLAGS_BOTTOM_RIGHT) ? rounding : 0.0f;
            const float roundingBottomLeft = (cornerFlags & CORNER_FLAGS_BOTTOM_LEFT) ? rounding : 0.0f;
            PathArcToCached(Vec2(p1.x + roundingTopLeft, p1.y + roundingTopLeft), roundingTopLeft, 6, 9);
            PathArcToCached(Vec2(p2.x - roundingTopRight, p1.y + roundingTopRight), roundingTopRight, 9, 12);
            PathArcToCached(Vec2(p2.x - roundingBottomRight, p2.y - roundingBottomRight), roundingBottomRight, 0, 3);
            PathArcToCached(Vec2(p1.x + roundingBottomLeft, p2.y - roundingBottomLeft), roundingBottomLeft, 3, 6);
        }
    }

    void DrawList::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, size_t numSegments, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t i = 0; i <= numSegments; i++)
        {
            const float angle = startAngle + (static_cast<float>(i) / static_cast<float>(numSegments)) * (endAngle - startAngle);
            path.emplace_back(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        }
    }

    void DrawList::PathArcToCached(const Vec2& center, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t angle = startAngleOf12; angle <= endAngleOf12; angle++)
        {
            const Vec2& point = cachedCircleVertices12[angle % 12];
            path.emplace_back(center.x + point.x * radius, center.y + point.y * radius);
        }
    }

    void DrawList::PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments)
    {
        const Vec2& p1 = path.back();
        if (numSegments == 0)
            PathBezierToCasteljau(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, 0);
        else
        {
            const float tStep = 1.0f / static_cast<float>(numSegments);
            for (size_t iStep = 1; iStep <= numSegments; iStep++)
                path.push_back(BezierCalc(p1, p2, p3, p4, tStep * iStep));
        }
    }

    void DrawList::PrimRect(const Vec2& p1, const Vec2& p2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2(uv2.x, uv1.y));
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2(uv1.x, uv2.y));

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(p3, color, uv3);
        vertices.emplace_back(p4, color, uv4);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1 + Vec2(0.5f, 0.5f));
        PathLineTo(p2 + Vec2(0.5f, 0.5f));
        PathStroke(color, false, thickness);
    }

    void DrawList::AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed, float thickness)
    {
        if (numPoints < 2)
            return;

        IndexType nextIndex = static_cast<IndexType>(vertices.size());
        const float halfThickness = (thickness * 0.5f);
        for (size_t i = 0; i < (closed ? numPoints : numPoints - 1); i++, nextIndex += 4)
        {
            const Vec2& p1_tmp = points[i], & p2_tmp = points[(i + 1) == numPoints ? 0 : i + 1];
            const float dx = p2_tmp.x - p1_tmp.x;
            const float dy = p2_tmp.y - p1_tmp.y;

            //If we didn't swap the points in the cases the triangles would be drawn in the counter clockwise direction, which can cause problems in some rendering APIs.
            const bool swapPoints = (dx < 0.0f || dy < 0.0f) || (dx > 0.0f && dy > 0.0f);
            const Vec2& p1 = swapPoints ? p2_tmp : p1_tmp, & p2 = swapPoints ? p1_tmp : p2_tmp;

            if (dy == 0) // Horizontal line
            {
                int factor = dx > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y + halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y + halfThickness), color, Vec2());
            }
            else if (dx == 0) // Vertical line
            {
                int factor = dy > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x + halfThickness, p1.y - halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness, p1.y - halfThickness * factor), color, Vec2());
            }
            else // Inclined line
            {
                const float lineWidth = sqrtf(dx * dx + dy * dy);

                const float cosine = dx / lineWidth;
                const float sine = dy / lineWidth;

                const float xFactor = cosine * halfThickness;
                const float yFactor = sine * halfThickness;

                vertices.emplace_back(Vec2(p1.x - yFactor, p1.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - yFactor, p2.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + yFactor, p2.y - xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x + yFactor, p1.y - xFactor), color, Vec2());
            }

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 1);
            indices.push_back(nextIndex + 2);

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 2);
            indices.push_back(nextIndex + 3);
        }
    }

    void DrawList::AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color)
    {
        if (numPoints < 3)
            return;

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        for (size_t i = 0; i < numPoints; i++)
            vertices.emplace_back(points[i], color, Vec2());

        for (size_t i = 2; i < numPoints; i++)
            indices.push_back(nextIndex), indices.push_back(nextIndex + static_cast<IndexType>(i - 1)), indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddBezierCurve(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness, size_t numSegments)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathBezierCurveTo(p2, p3, p4, numSegments);
        PathStroke(color, false, thickness);
    }

    void DrawList::AddText(const Vec2& pos, Color color, const char* text, size_t textLength)
    {
        AddText(NULL, 0.0f, pos, color, text, textLength);
    }
    
    void DrawList::AddText(const void* font, float fontSize, const Vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth)
    {
        if (!color.a)
            return;
        
        const char* const textEnd = text + textLength;
        
        float x = pos.x;
        float y = pos.y;

        while (text < textEnd)
        {
            //const Glyph* glyph = font->FindGlyph(*text);
        
            //stbtt_aligned_quad q;
            //stbtt_GetBakedQuad(bdata, 512, 512, *text - 32, &x, &y, &q, 0);
            //
            //const size_t nextIndex = vertices.size();
            //vertices.emplace_back(Vec2(glyph->x0, glyph->y0), color, Vec2(glyph->u0, glyph->v0));
            //vertices.emplace_back(Vec2(glyph->x1, glyph->y0), color, Vec2(glyph->u1, glyph->v0));
            //vertices.emplace_back(Vec2(glyph->x1, glyph->y1), color, Vec2(glyph->u1, glyph->v1));
            //vertices.emplace_back(Vec2(glyph->x0, glyph->y1), color, Vec2(glyph->u0, glyph->v1));
        
            const IndexType nextIndex = static_cast<IndexType>(vertices.size());

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 1);
            indices.push_back(nextIndex + 2);
        
            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 2);
            indices.push_back(nextIndex + 3);
        
            text++;
        }
    }

    void DrawList::AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimRectUV(p1, p2, uv1, uv2, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        if (rounding <= 0.0f || !cornerFlags)
            AddImage(userTextureId, p1, p2, uv1, uv2, color);
        else
        {
            PushRemainingDrawCommands();

            PathRect(p1, p2, rounding, cornerFlags);

            const size_t v0 = vertices.size(), i0 = indices.size();
            PathFillConvex(color);
            ShadeVertsLinearUV(v0, p1, p2, uv1, uv2, true);

            PushTextureDrawCommand(vertices.size() - v0, indices.size() - i0, userTextureId);
        }
    }

} // namespace nmd