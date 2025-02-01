#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Text.hpp>

class TextNode : public SceneNode
{
public:
	explicit TextNode(const FontHolder& fonts, const std::string& text);
	void SetText(const std::string& text);

private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Text m_text;
};
