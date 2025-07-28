#include "pch.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/World.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	Entity::Entity(EntityId entityId) : m_id(entityId)
	{
		m_registry = &Systems::GetMainWorld()->registry;
		m_valid = true;
	}

	Entity::Entity(EntityId entityId, World* world) : m_id(entityId)
	{
		m_registry = &Systems::GetMainWorld()->registry;
		m_valid = true;
	}

	Entity Entity::Create()
	{
		Entity entity;
		auto world = Systems::GetMainWorld();
		entity.m_id = world->registry.create();
		entity.m_registry = &world->registry;
		entity.m_valid = true;
		return entity;
	}

	Entity Entity::Create(World* world)
	{
		Entity entity;
		entity.m_id = world->registry.create();
		entity.m_registry = &world->registry;
		entity.m_valid = true;
		return entity;
	}

	void Entity::AddChild(Entity entity)
	{
		if (!entity.Valid())
		{
			LOG_ERROR("Cannot add an invalid entity as a children.");
			return;
		}
		if (entity.m_registry != m_registry)
		{
			LOG_ERROR("Cannot add an entity child if it doesn't belong to the same world as parent.");
			return;
		}
		if (entity.GetId() == m_id)
		{
			LOG_ERROR("Entity cannot have itself as a child.");
			return;
		}

		entity.Unparent();
		entity.AddComponent<Parent>()->parent = m_id;

		auto children = AddComponent<Children>();
		children->children.insert(entity.m_id);

		auto transformChildren = entity.AddComponent<Transform>();
		transformChildren->SetParent(m_id);

	}

	void Entity::RemoveChild(EntityId entity)
	{
		auto children = GetComponent<Children>();

		if (children == nullptr)
		{
			LOG_WARN("Trying to remove child on an entity with no children. What are you doing ?");
			return;
		}

		auto find_it = children->children.find(entity);
		if (find_it == children->children.end())
		{
			LOG_WARN("Trying to remove a child entity that is not in the parent. You looser.");
			return;
		}

		Entity(*find_it, m_registry).JustUnparent();
	}

	void Entity::Unparent()
	{
		auto transform = GetComponent<Transform>();
		if (transform != nullptr)
		{
			transform->RemoveParent();
		}
		auto parent = GetComponent<Parent>();
		if (parent != nullptr)
		{
			Entity(parent->parent).JustRemoveChild(m_id);
			RemoveComponent<Parent>();
		}
	}

	bool Entity::Valid()
	{
		return m_valid && m_registry->valid(m_id);
	}

	void Entity::Destroy()
	{
		if (!Valid())
		{
#ifndef SANDCASTLE_DISTRIB
			LOG_WARN("Trying to destroy an invalid entity.");
#endif
			return;
		}

		auto children = GetComponent<Children>();
		if (children != nullptr)
		{
			for (auto& child : children->children)
			{
				Entity(child, m_registry).DestroyFromParent();
			}
		}
		Unparent();
		m_valid = false;
		m_registry->destroy(m_id);
	}

	void Entity::DestroyFromParent()
	{
		if (!Valid())
		{
			LOG_WARN("Child of destroying parent invalid.");
			return;
		}

		auto children = GetComponent<Children>();
		if (children != nullptr)
		{
			for (auto& child : children->children)
			{
				Entity(child, m_registry).DestroyFromParent();
			}
		}

		m_valid = false;
		m_registry->destroy(m_id);
	}

	void Entity::JustUnparent()
	{
		auto transform = GetComponent<Transform>();
		if (transform != nullptr)
		{
			transform->RemoveParent();
			return;
		}
		RemoveComponent<Parent>();
	}

	void Entity::JustRemoveChild(EntityId id)
	{
		auto children = GetComponent<Children>();
		if (children == nullptr)
			return;

		children->children.erase(id);
		if (children->children.empty())
		{
			RemoveComponent<Children>();
		}
	}

	Entity::Entity(EntityId entityId, entt::registry* registry) : m_id(entityId), m_registry(registry), m_valid(true)
	{

	}

}