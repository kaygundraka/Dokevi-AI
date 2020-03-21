#pragma once
#include <functional>
#include <chrono>
#include <vector>
#include <random>
#include <bitset>
#include <map>

#define AI_RUNIMPL_LAMBDA(UUID) [](UUID uuid, ConstDoubleMapType& constDoubleMap, ConstStringMapType& constStringMap, double timeDelta) -> NODE_RESULT_TYPE
#define AI_CHECK_CONDITION_LAMBDA(UUID) [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool

namespace DokeviAI
{
	enum NODE_RESULT_TYPE {
		NST_SUCCESS,
		NST_RUNNING,
		NST_FAILURE,
		NST_MAX
	};

	using ConstDoubleMapType = std::map<std::string, double>;
	using ConstStringMapType = std::map<std::string, std::string>;

	template <typename UUID = int>
	using CheckConditionFuncType = std::function<bool(UUID, ConstDoubleMapType&, ConstStringMapType&)>;

	template <typename UUID = int>
	using RunImplFuncType = std::function<NODE_RESULT_TYPE(UUID, ConstDoubleMapType&, ConstStringMapType&, double)>;

	//////////////////////////////////////////////////////////////////////////////////////
	// TreeNode
	//////////////////////////////////////////////////////////////////////////////////////

	template <typename UUID = int>
	class TreeNode
	{
	public:
		std::shared_ptr<TreeNode<UUID>> _parentNode;
		std::vector<std::shared_ptr<TreeNode<UUID>>> _childs;

		virtual ~TreeNode() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) { return NST_SUCCESS; };
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// ExecutionNode
	//////////////////////////////////////////////////////////////////////////////////////

	template <typename UUID = int>
	class ExecutionNode final : public TreeNode<UUID>
	{
	public:
		ConstDoubleMapType _constDoubleMap;
		ConstStringMapType _constStringMap;

		virtual ~ExecutionNode() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID,  double inTimeDelta) final
		{
			return _runImplFunc(inUUID, _constDoubleMap, _constStringMap, inTimeDelta);
		}

		RunImplFuncType<UUID> _runImplFunc;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Decorator
	//////////////////////////////////////////////////////////////////////////////////////

	template <typename UUID = int>
	class Decorator abstract : public TreeNode<UUID> {
	public:
		virtual ~Decorator() = default;

		ConstDoubleMapType _constDoubleMap;
		ConstStringMapType _constStringMap;

		CheckConditionFuncType<UUID> _checkConditionFunc;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) = 0;
	};

	template <typename UUID = int>
	class DecoratorWhile final : public Decorator<UUID> {
	public:
		virtual ~DecoratorWhile() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) final
		{
			if (Decorator<UUID>::_checkConditionFunc(inUUID, Decorator<UUID>::_constDoubleMap, Decorator<UUID>::_constStringMap) == true)
			{
				TreeNode<UUID>::_childs[0]->Run(inUUID, inTimeDelta);

				return NST_RUNNING;
			}

			return NST_FAILURE;
		}
	};

	template <typename UUID = int>
	class DecoratorIf : public Decorator<UUID> {
	public:
		virtual ~DecoratorIf() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) final
		{
			if (Decorator<UUID>::_checkConditionFunc(inUUID, Decorator<UUID>::_constDoubleMap, Decorator<UUID>::_constStringMap) == true)
			{
				return TreeNode<UUID>::_childs[0]->Run(inUUID, inTimeDelta);
			}

			return NST_FAILURE;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// ControlFlowNode
	//////////////////////////////////////////////////////////////////////////////////////
	template <typename UUID = int>
	class ControlFlowNode abstract : public TreeNode<UUID>
	{
	public:
		virtual ~ControlFlowNode() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) = 0;
	};

	template <typename UUID = int>
	class Selector final : public ControlFlowNode<UUID>
	{
	public:
		virtual ~Selector() = default;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) final
		{
			long long seed = std::chrono::steady_clock::now().time_since_epoch().count();
			std::default_random_engine e(seed);

			std::bitset<32> childBitSet;

			while (true)
			{
				int selectRandomNode;
				NODE_RESULT_TYPE currentChildNodeResult;

				while (true)
				{
					if (!childBitSet.any())
					{
						return NST_FAILURE;
					}

					std::uniform_int_distribution<int> dis(0, TreeNode<UUID>::_childs.size());
					selectRandomNode = dis(e);

					if (!childBitSet.test(selectRandomNode))
					{
						currentChildNodeResult = TreeNode<UUID>::_childs[selectRandomNode]->Run(inUUID, inTimeDelta);
						break;
					}
				}

				switch (currentChildNodeResult)
				{
				case NST_SUCCESS:
				case NST_RUNNING:
					return currentChildNodeResult;
				case NST_FAILURE:
					childBitSet.set(selectRandomNode, true);
					break;
				}
			}
		}
	};

	template <typename UUID = int>
	class Sequence final : public ControlFlowNode<UUID>
	{
	public:
		Sequence() = default;
		virtual ~Sequence() = default;
		
		std::map<UUID, int> _childNodeIndexMap;

		virtual NODE_RESULT_TYPE Run(UUID inUUID, double inTimeDelta) final
		{
			if (_childNodeIndexMap.find(inUUID) == _childNodeIndexMap.end())
			{
				_childNodeIndexMap.insert(std::make_pair(inUUID, 0));
			}

			auto iter = _childNodeIndexMap.find(inUUID);

			while (iter->second < TreeNode<UUID>::_childs.size())
			{
				NODE_RESULT_TYPE currentChildNodeResult = TreeNode<UUID>::_childs[iter->second++]->Run(inUUID, inTimeDelta);

				switch (currentChildNodeResult)
				{
				case NST_SUCCESS:
					continue;

				case NST_RUNNING:
				case NST_FAILURE:
					return currentChildNodeResult;
				}
			}

			iter->second = 0;

			return NST_SUCCESS;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// BehaviourTree
	//////////////////////////////////////////////////////////////////////////////////////
	template <typename UUID = int>
	class BehaviourTree final
	{
	public:
		BehaviourTree() { _rootNode = std::make_shared<Sequence<UUID>>(); }

		std::shared_ptr<Sequence<UUID>> _rootNode;

		void Run(UUID inUUID, double inTimeDelta)
		{
			_rootNode->Run(inUUID, inTimeDelta);
		}
	};
}