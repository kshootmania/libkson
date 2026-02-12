#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	// Evaluates curve function with control points a, b at position x (all in range [0, 1])
	[[nodiscard]]
	double EvaluateCurve(double a, double b, double x);

	// Evaluates the curve function using GraphCurveValue
	// Returns x (linear) if curve.isLinear() is true
	[[nodiscard]]
	double EvaluateCurve(const GraphCurveValue& curve, double x);

	// Expands a graph with curve data into linear segments at specified intervals
	// Used for laser sections and scroll_speed where pre-conversion is required
	// subdivisionInterval: interval in ticks for subdividing curve segments
	// Returns a new graph with curve segments subdivided into linear segments
	[[nodiscard]]
	Graph ExpandCurveSegments(const Graph& graph, Pulse subdivisionInterval);

	// Expands a graph section with curve data into linear segments at specified intervals
	// Used for laser sections where pre-conversion is required
	// subdivisionInterval: interval in ticks for subdividing curve segments
	// Returns a new graph section with curve segments subdivided into linear segments
	[[nodiscard]]
	GraphSection ExpandCurveSegments(const GraphSection& graphSection, RelPulse subdivisionInterval);

	// Forward declaration for LaserSection
	struct LaserSection;

	// Expands a laser section with curve data into linear segments at specified intervals
	// subdivisionInterval: interval in ticks for subdividing curve segments
	// Returns a new laser section with curve segments subdivided into linear segments
	[[nodiscard]]
	LaserSection ExpandCurveSegments(const LaserSection& laserSection, RelPulse subdivisionInterval);
}
