#pragma once
#include <optional>
#include "kson/Common/Common.hpp"
#include "kson/Note/NoteInfo.hpp"

namespace kson
{
	[[nodiscard]]
	double GraphValueAt(const Graph& graph, Pulse pulse);

	[[nodiscard]]
	Graph BakeStopIntoScrollSpeed(const Graph& scrollSpeed, const ByPulse<RelPulse>& stop);

	template <class GS>
	[[nodiscard]]
	typename ByPulse<GS>::const_iterator GraphSectionAt(const ByPulse<GS>& graphSections, Pulse pulse)
#ifdef __cpp_concepts
		requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
#endif
	{
		assert(!graphSections.empty());

		auto itr = graphSections.upper_bound(pulse);
		if (itr != graphSections.begin())
		{
			--itr;
		}

		return itr;
	}

	template <class GS>
	[[nodiscard]]
	std::optional<double> GraphSectionValueAt(const ByPulse<GS>& graphSections, Pulse pulse)
#ifdef __cpp_concepts
		requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
#endif
	{
		if (graphSections.empty())
		{
			return std::nullopt;
		}

		const auto itr = GraphSectionAt(graphSections, pulse);
		if (itr == graphSections.end())
        {
            return std::nullopt;
        }

		const auto& [y, graphSection] = *itr;
		const RelPulse ry = pulse - y;

		if (graphSection.v.size() <= 1)
		{
			return std::nullopt;
		}

		{
			const auto& [firstRy, _] = *graphSection.v.begin();
			if (ry < firstRy)
			{
				return std::nullopt;
			}
		}

		{
			const auto& [lastRy, _] = *graphSection.v.rbegin();
			if (ry >= lastRy)
			{
				return std::nullopt;
			}
		}

		return std::make_optional(GraphValueAt(graphSection.v, ry));
	}

	template <class GS>
	[[nodiscard]]
	double GraphSectionValueAtWithDefault(const ByPulse<GS>& graphSections, Pulse pulse, double defaultValue)
#ifdef __cpp_concepts
		requires std::is_same_v<GS, GraphSection> || std::is_same_v<GS, LaserSection>
#endif
	{
		const std::optional<double> sectionValue = GraphSectionValueAt(graphSections, pulse);
		if (sectionValue.has_value())
		{
			return *sectionValue;
		}
		else
		{
			return defaultValue;
		}
	}

	template <class GS>
	[[nodiscard]]
	std::optional<GraphPoint> GraphPointAt(const ByPulse<GS>& graphSections, Pulse pulse)
	{
		if (graphSections.empty())
		{
			return std::nullopt;
		}

		const auto itr = GraphSectionAt(graphSections, pulse);
		if (itr == graphSections.end())
		{
			return std::nullopt;
		}

		const auto& [y, graphSection] = *itr;
		const RelPulse ry = pulse - y;

		if (!graphSection.v.contains(ry))
		{
			return std::nullopt;
		}

		return graphSection.v.at(ry);
	}
}
