#pragma once
#include <map>
#include "BehaviourTree.h"

namespace DokeviAI
{
	template <typename UUID = int>
	class Agent
	{
	public:
		BehaviourTree<UUID> _behaviourTree;

		Agent() = default;
		virtual ~Agent() = default;
	};
}