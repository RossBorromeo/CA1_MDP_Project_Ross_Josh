#pragma once
#include "TextureID.hpp"
#include "FontID.hpp"
#include "ShaderTypes.hpp"
#include "SoundEffect.hpp"

namespace sf
{
	class Texture;
	class Font;
	class Shader;
	class SoundBuffer;
}

template<typename Identifier, typename Resource>
class ResourceHolder;

typedef ResourceHolder<TextureID, sf::Texture> TextureHolder;
typedef ResourceHolder <FontID, sf::Font > FontHolder;
typedef ResourceHolder<ShaderTypes, sf::Shader> ShaderHolder;
typedef ResourceHolder<SoundEffect, sf::SoundBuffer> SoundBufferHolder;