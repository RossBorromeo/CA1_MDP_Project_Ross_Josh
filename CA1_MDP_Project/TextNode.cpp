#include "TextNode.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

TextNode::TextNode(const FontHolder& fonts, const std::string& text)
{
	const sf::Font& font = fonts.Get(FontID::kMain);
	m_text.setFont(font);
	m_text.setCharacterSize(20);
	SetText(text);
}

void TextNode::SetText(const std::string& text)
{
	m_text.setString(text);
	Utility::CentreOrigin(m_text);
}

void TextNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_text, states);
}
