#pragma once

#include "SandCastle/Core/Vec.h"
#include <algorithm>

namespace SandCastle
{
    /// @brief Cubic Bezier curve defined by four control points.
    /// Used by ParticleSystem trajectories. Static factories produce ready-made
    /// curves: Straight, CubicIn (bend early), CubicOut (bend late), CubicInOut (symmetric).
    class Beziers
    {
    public:
        Beziers() {}
        Beziers(Vec2f a, Vec2f b, Vec2f c, Vec2f d) : p0(a), p1(b), p2(c), p3(d) {}
        inline static float ClampAbs(float v, float maxAbs)
        {
            if (v > maxAbs) return  maxAbs;
            if (v < -maxAbs) return -maxAbs;
            return v;
        }
        // Factory: straight segment (implemented as a cubic with handles on the line)
        static Beziers Straight(Vec2f start, Vec2f end) {
            Vec2f d = end - start;
            // Handles at 1/3 and 2/3 along the segment -> exactly a straight line
            return Beziers(start, start + d * (1.0f / 3.0f), start + d * (2.0f / 3.0f), end);
        }

        // Factory: "ease-in" style curve (bends early, settles toward the end)
        // 'amount' controls how strong the curve is; 0 -> straight
        static Beziers CubicIn(Vec2f start, Vec2f end, float amount = 0.35f)
        {
            amount = ClampAbs(amount, 1.0f);

            Vec2f d = end - start;
            if (d.x == 0.f && d.y == 0.f)
                return Straight(start, end);

            // Perpendicular (normalized-ish not required for particles)
            Vec2f perp{ -d.y, d.x };

            Vec2f c1 = start + d * 0.20f + perp * (amount);
            Vec2f c2 = start + d * 0.80f + perp * (amount * 0.20f);

            return Beziers(start, c1, c2, end);
        }

        // Factory: "ease-out" style curve (stays straighter early, bends near the end)
        static Beziers CubicOut(Vec2f start, Vec2f end, float amount = 0.35f)
        {
            amount = ClampAbs(amount, 1.0f);

            Vec2f d = end - start;
            if (d.x == 0.f && d.y == 0.f)
                return Straight(start, end);

            Vec2f perp{ -d.y, d.x };

            Vec2f c1 = start + d * 0.20f + perp * (amount * 0.20f);
            Vec2f c2 = start + d * 0.80f + perp * (amount);

            return Beziers(start, c1, c2, end);
        }
        static Beziers CubicInOut(Vec2f start, Vec2f end, float amount = 0.35f)
        {
            amount = ClampAbs(amount, 1.0f);

            Vec2f d = end - start;
            if (d.x == 0.f && d.y == 0.f)
                return Straight(start, end);

            // Perpendicular
            Vec2f perp{ -d.y, d.x };

            // Handles symmetric around the center
            Vec2f c1 = start + d * 0.25f + perp * (amount * 0.5f);
            Vec2f c2 = start + d * 0.75f + perp * (amount * 0.5f);

            return Beziers(start, c1, c2, end);
        }
        /// @brief Evaluate the curve at parameter t in [0,1]. Clamped if outside.
        Vec2f Step(float t) const {
            t = std::clamp(t, 0.0f, 1.0f);

            // Cubic Bezier: (1-t)^3 p0 + 3(1-t)^2 t p1 + 3(1-t) t^2 p2 + t^3 p3
            float u = 1.0f - t;
            float uu = u * u;
            float tt = t * t;

            return p0 * (uu * u)
                + p1 * (3.0f * uu * t)
                + p2 * (3.0f * u * tt)
                + p3 * (tt * t);
        }

        /// @brief Cheap length approximation: average of the chord and the control-net length. Lower-bound clamped to 1.
        float LengthFast() const
        {
            float chord = (p3 - p0).Magnitude();
            float controlNet =
                (p1 - p0).Magnitude() +
                (p2 - p1).Magnitude() +
                (p3 - p2).Magnitude();

            return std::max(1.f, (chord + controlNet) * 0.5f);
        }

        Vec2f p0, p1, p2, p3;

    };
}
