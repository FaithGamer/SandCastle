#include "pch.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/SpriteRender.h"
namespace SandCastle
{
	entt::registry Entity::registry = entt::registry();

	Entity::Entity(EntityId entityId) : m_id(entityId)
	{
		m_valid = true;
	}

	Entity Entity::Create()
	{
		Entity entity;
		entity.m_id = registry.create();
		entity.m_valid = true;
		return entity;
	}

	size_t Entity::Count()
	{
		return registry.storage<EntityId>().free_list();
	}

	void Entity::AddChild(Entity entity)
	{
		if (!entity.Valid())
		{
			LOG_ERROR("Cannot add an invalid entity as a children.");
			return;
		}
		if (entity.GetId() == m_id)
		{
			LOG_ERROR("Entity cannot have itself as a child.");
			return;
		}

		entity.Unparent();
		entity.AddGet<Parent>()->parent = m_id;

		auto children = AddGet<Children>();
		children->children.insert(entity.m_id);

		entity.Add<Transform>();
		auto transformChildren = entity.Get<Transform>();
		transformChildren->SetParent(m_id);

	}

	void Entity::RemoveChild(EntityId entity)
	{
		auto children = Get<Children>();

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

		Entity(*find_it).JustUnparent();
	}

	void Entity::Unparent()
	{
		auto transform = Get<Transform>();
		if (transform != nullptr)
		{
			transform->RemoveParent();
		}
		auto parent = Get<Parent>();
		if (parent != nullptr)
		{
			Entity(parent->parent).JustRemoveChild(m_id);
			Remove<Parent>();
		}
	}

	bool Entity::Valid()
	{
		return m_valid && registry.valid(m_id);
	}

	Transform* Entity::gtr()
	{
		return Get<Transform>();
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

		auto children = Get<Children>();
		if (children != nullptr)
		{
			for (auto& child : children->children)
			{
				Entity(child).DestroyFromParent();
			}
		}
		Unparent();
		m_valid = false;
		registry.destroy(m_id);
	}

	void Entity::DestroyFromParent()
	{
		if (!Valid())
		{
			LOG_WARN("Child of destroying parent invalid.");
			return;
		}

		auto children = Get<Children>();
		if (children != nullptr)
		{
			for (auto& child : children->children)
			{
				Entity(child).DestroyFromParent();
			}
		}

		m_valid = false;
		registry.destroy(m_id);
	}

	void Entity::JustUnparent()
	{
		auto transform = Get<Transform>();
		if (transform != nullptr)
		{
			transform->RemoveParent();
			return;
		}
		Remove<Parent>();
	}

	void Entity::JustRemoveChild(EntityId id)
	{
		auto children = Get<Children>();
		if (children == nullptr)
			return;

		children->children.erase(id);
		if (children->children.empty())
		{
			Remove<Children>();
		}
	}
}