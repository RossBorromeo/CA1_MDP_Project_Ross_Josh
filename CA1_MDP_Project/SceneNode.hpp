#pragma once
#include <vector>
#include <memory>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>

class Command;
class CommandQueue;

class SceneNode : public sf::Transformable, public sf::Drawable
{
public:
	using Ptr = std::unique_ptr<SceneNode>;

	SceneNode();
	virtual ~SceneNode() = default;

	void AttachChild(Ptr child);
	Ptr DetachChild(const SceneNode& node);

	void Update(sf::Time dt, CommandQueue& commands);
	virtual void MarkForRemoval();
	bool IsMarkedForRemoval() const;

	virtual unsigned int GetCategory() const;
	virtual sf::FloatRect GetBoundingRect() const;

	sf::Transform GetWorldTransform() const;
	sf::Vector2f GetWorldPosition() const;
	SceneNode* GetParent() const;
	SceneNode* GetChildren() const;

	void OnCommand(const Command& command, sf::Time dt);
	void RemoveWrecks();

protected:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	bool m_is_marked_for_removal;
	std::vector<Ptr> m_children;
	SceneNode* m_parent;
};
