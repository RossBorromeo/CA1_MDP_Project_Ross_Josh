//Ross - D00241095 | Josh - D00238448
#pragma once
enum class SceneLayers
{
	kBackground,
	kLowerAir,
	kUpperAir,
	kLayerCount
};


constexpr std::size_t NumSceneLayers = static_cast<std::size_t>(SceneLayers::kLayerCount);
