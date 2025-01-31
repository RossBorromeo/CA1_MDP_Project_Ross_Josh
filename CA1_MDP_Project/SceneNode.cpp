#include "SceneNode.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

SceneNode::SceneNode()
	: m_is_marked_for_removal(false), m_parent(nullptr)
{
}

void SceneNode::AttachChild(Ptr child)
{
	child->m_parent = this;
	m_children.push_back(std::move(child));
}

SceneNode::Ptr SceneNode::DetachChild(const SceneNode& node)
{
	auto found = std::find_if(m_children.begin(), m_children.end(),
		[&](Ptr& p) { return p.get() == &node; });
	if (found != m_children.end())
	{
		Ptr result = std::move(*found);
		result->m_parent = nullptr;
		m_children.erase(found);
		return result;
	}
	return nullptr;
}

void SceneNode::Update(sf::Time dt, CommandQueue& commands)
{
	UpdateCurrent(dt, commands);
	for (auto& child : m_children)
	{
		child->Update(dt, commands);
	}
	m_children.erase(std::remove_if(m_children.begin(), m_children.end(),
		[](const Ptr& child) { return child->IsMarkedForRemoval(); }), m_children.end());
}

void SceneNode::MarkForRemoval()
{
	m_is_marked_for_removal = true;
}

bool SceneNode::IsMarkedForRemoval() const
{
	return m_is_marked_for_removal;
}

SceneNode* SceneNode::GetParent() const
{
	return m_parent;
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	drawCurrent(target, states);
	for (const auto& child : m_children)
	{
		child->draw(target, states);
	}
}

void SceneNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Default empty implementation to prevent pure virtual errors
}

void SceneNode::OnCommand(const Command& command, sf::Time dt)
{
	if (command.category & GetCategory())
	{
		command.action(*this, dt);
	}
	for (auto& child : m_children)
	{
		child->OnCommand(command, dt);
	}
}

void SceneNode::RemoveWrecks()
{
	m_children.erase(std::remove_if(m_children.begin(), m_children.end(),
		[](const Ptr& child) { return child->IsMarkedForRemoval(); }), m_children.end());
}
