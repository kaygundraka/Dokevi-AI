#pragma once
#include <map>
#include <fstream>
#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <Windows.h>

#include "ISingleton.h"
#include "BehaviourTree.h"
#include "Agent.h"

#include "./rapidjson/document.h"
#include "./rapidjson/istreamwrapper.h"

using namespace rapidjson;

namespace DokeviAI
{
	template <typename UUID = int>
	class AIScriptDB : public ISingleton<AIScriptDB<UUID>>
	{
	private:
		std::map<std::string, std::shared_ptr<BehaviourTree<UUID>>> _behaviourTreeMap;

		std::map<std::string, DecoratorIf<UUID>> _decoratorIfMap;
		std::map<std::string, DecoratorWhile<UUID>> _decoratorWhileMap;
		std::map<std::string, ExecutionNode<UUID>> _executionNodeMap;

	public:
		AIScriptDB() = default;

		virtual ~AIScriptDB()
		{
			_behaviourTreeMap.clear();
			_decoratorIfMap.clear();
			_executionNodeMap.clear();
		}

		bool LoadData(std::string inFolder);

		bool RegistAIScript(std::string inFileName);
		bool ConvertAIScriptToBehaviourTree(Document& inJsonAiDocument);
		bool JsonObjectToBehaviourTreeNode(std::string inAiName, std::shared_ptr<TreeNode<UUID>> inTreeNode, rapidjson::Value& inJsonObject);

		std::shared_ptr<Agent<UUID>> CreateAgent(std::string inAiName) const;

		bool RegistDecoratorIf(std::string inName, CheckConditionFuncType<UUID> inCheckFunc);
		bool RegistDecoratorWhile(std::string inName, CheckConditionFuncType<UUID> inCheckFunc);
		bool RegistExecutionNode(std::string inName, RunImplFuncType<UUID> inRunImplFunc);

		DecoratorIf<UUID> GetDecoratorIf(std::string inNam);
		DecoratorWhile<UUID> GetDecoratorWhile(std::string inName);
		ExecutionNode<UUID> GetExecutionNode(std::string inNam);
	};

	template <typename UUID>
	bool AIScriptDB<UUID>::LoadData(std::string inFolder)
	{
		_finddata_t fd;

		char path[MAX_PATH] = { 0, };

		::GetCurrentDirectoryA(MAX_PATH, path);

		std::string address = inFolder;

		std::string absoluteAddress = path;
		absoluteAddress += address.substr(1, address.size() - 1);

		intptr_t handle = _findfirst(absoluteAddress.c_str(), &fd);

		if (handle == -1)
		{
			return false;
		}

		int result = 1;

		while (result != -1)
		{
			if (strcmp(fd.name, ".") != 0 && strcmp(fd.name, "..") != 0)
			{
				RegistAIScript(address.substr(0, address.size() - 3) + fd.name);
			}
			result = _findnext(handle, &fd);
		}

		_findclose(handle);

		return true;
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::RegistAIScript(std::string inFileName)
	{
		std::ifstream jsonFile(inFileName);

		if (!jsonFile.is_open())
		{
			return false;
		}

		IStreamWrapper isw(jsonFile);

		Document aiDocument;

		aiDocument.ParseStream(isw);

		if (aiDocument.HasParseError())
		{
			return false;
		}

		return ConvertAIScriptToBehaviourTree(aiDocument);
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::ConvertAIScriptToBehaviourTree(Document& inJsonAiDocument)
	{
		std::string aiName = inJsonAiDocument.FindMember("ai")->value.GetString();

		auto& behaviourComponent = inJsonAiDocument.FindMember("behaviourTree")->value;

		std::shared_ptr<BehaviourTree<UUID>> newBehaviourTree(new BehaviourTree<UUID>());

		if (!JsonObjectToBehaviourTreeNode(aiName, newBehaviourTree->_rootNode, behaviourComponent))
		{
			return false;
		}

		_behaviourTreeMap.insert(make_pair(aiName, newBehaviourTree));

		return true;
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::JsonObjectToBehaviourTreeNode(std::string inAiName, std::shared_ptr<TreeNode<UUID>> inTreeNode, rapidjson::Value& inJsonObject)
	{
		std::string name = inJsonObject.FindMember("type")->value.GetString();

		if (name == "sequence" || name == "behaviourTree")
		{
			if (!inJsonObject.HasMember("child"))
			{
				return false;
			}

			auto childArray = inJsonObject.FindMember("child")->value.GetArray();

			std::shared_ptr<Sequence<UUID>> newNode(new Sequence<UUID>());
			inTreeNode->_childs.push_back(newNode);
			newNode->_parentNode = inTreeNode;

			for (unsigned int i = 0; i < childArray.Size(); i++)
			{
				JsonObjectToBehaviourTreeNode(inAiName, newNode, childArray[i]);
			}
		}
		else if (name == "decoratorWhile")
		{
			std::shared_ptr<DecoratorWhile<UUID>> newNode(new DecoratorWhile<UUID>(GetDecoratorWhile(inJsonObject.FindMember("name")->value.GetString())));
			inTreeNode->_childs.push_back(newNode);
			newNode->_parentNode = inTreeNode;

			if (inJsonObject.HasMember("const"))
			{
				auto childArray = inJsonObject.FindMember("const")->value.GetArray();

				for (unsigned int i = 0; i < childArray.Size(); i++)
				{
					if (childArray[i].MemberBegin()->value.IsNumber())
					{
						newNode->_constDoubleMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetDouble()
						)
						);
					}
					else if (childArray[i].MemberBegin()->value.IsString())
					{
						newNode->_constStringMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetString()
						)
						);
					}
				}
			}

			JsonObjectToBehaviourTreeNode(inAiName, newNode, inJsonObject.FindMember("child")->value);
		}
		else if (name == "decoratorIf")
		{
			std::shared_ptr<DecoratorIf<UUID>> newNode(new DecoratorIf<UUID>(GetDecoratorIf(inJsonObject.FindMember("name")->value.GetString())));
			inTreeNode->_childs.push_back(newNode);
			newNode->_parentNode = inTreeNode;

			if (inJsonObject.HasMember("const"))
			{
				auto childArray = inJsonObject.FindMember("const")->value.GetArray();

				for (unsigned int i = 0; i < childArray.Size(); i++)
				{
					if (childArray[i].MemberBegin()->value.IsNumber())
					{
						newNode->_constDoubleMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetDouble()
						)
						);
					}
					else if (childArray[i].MemberBegin()->value.IsString())
					{
						newNode->_constStringMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetString()
						)
						);
					}
				}
			}

			JsonObjectToBehaviourTreeNode(inAiName, newNode, inJsonObject.FindMember("child")->value);
		}
		else if (name == "execution")
		{
			std::shared_ptr<ExecutionNode<UUID>> newNode(new ExecutionNode<UUID>(GetExecutionNode(inJsonObject.FindMember("name")->value.GetString())));
			inTreeNode->_childs.push_back(newNode);
			newNode->_parentNode = inTreeNode;

			if (inJsonObject.HasMember("const"))
			{
				auto childArray = inJsonObject.FindMember("const")->value.GetArray();

				for (unsigned int i = 0; i < childArray.Size(); i++)
				{
					if (childArray[i].MemberBegin()->value.IsNumber())
					{
						newNode->_constDoubleMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetDouble()
						)
						);
					}
					else if (childArray[i].MemberBegin()->value.IsString())
					{
						newNode->_constStringMap.insert(std::make_pair(
							childArray[i].MemberBegin()->name.GetString(),
							childArray[i].MemberBegin()->value.GetString()
						)
						);
					}
				}
			}
		}
		else if (name == "selector")
		{
			if (!inJsonObject.HasMember("child"))
			{
				return false;
			}

			auto childArray = inJsonObject.FindMember("child")->value.GetArray();

			std::shared_ptr<Selector<UUID>> newNode(new Selector<UUID>());
			inTreeNode->_childs.push_back(newNode);
			newNode->_parentNode = inTreeNode;

			for (unsigned int i = 0; i < childArray.Size(); i++)
			{
				JsonObjectToBehaviourTreeNode(inAiName, newNode, childArray[i]);
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	template <typename UUID>
	std::shared_ptr<Agent<UUID>> AIScriptDB<UUID>::CreateAgent(std::string inAiName) const
	{
		auto iter = _behaviourTreeMap.find(inAiName);

		if (iter == _behaviourTreeMap.end())
		{
			return std::shared_ptr<Agent<UUID>>(nullptr);
		}

		std::shared_ptr<Agent<UUID>> newAgent(new Agent<UUID>());

		newAgent->_behaviourTree = *iter->second;

		return newAgent;
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::RegistDecoratorIf(std::string inName, CheckConditionFuncType<UUID> inCheckFunc)
	{
		if (_decoratorIfMap.find(inName) != _decoratorIfMap.end())
		{
			return false;
		}

		DecoratorIf<UUID> decoratorIf;
		decoratorIf._checkConditionFunc = inCheckFunc;

		_decoratorIfMap.insert(std::make_pair(inName, decoratorIf));

		return true;
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::RegistDecoratorWhile(std::string inName, CheckConditionFuncType<UUID> inCheckFunc)
	{
		if (_decoratorWhileMap.find(inName) != _decoratorWhileMap.end())
		{
			return false;
		}

		DecoratorWhile<UUID> decoratorWhile;
		decoratorWhile._checkConditionFunc = inCheckFunc;

		_decoratorWhileMap.insert(std::make_pair(inName, decoratorWhile));

		return true;
	}

	template <typename UUID>
	bool AIScriptDB<UUID>::RegistExecutionNode(std::string inName, RunImplFuncType<UUID> inRunImplFunc)
	{
		if (_executionNodeMap.find(inName) != _executionNodeMap.end())
		{
			return false;
		}

		ExecutionNode<UUID> executionNode;
		executionNode._runImplFunc = inRunImplFunc;

		_executionNodeMap.insert(std::make_pair(inName, executionNode));

		return true;
	}

	template <typename UUID>
	DecoratorIf<UUID> AIScriptDB<UUID>::GetDecoratorIf(std::string inName)
	{
		auto iter = _decoratorIfMap.find(inName);

		if (iter == _decoratorIfMap.end())
		{
			return DecoratorIf<UUID>();
		}

		return iter->second;
	}

	template <typename UUID>
	DecoratorWhile<UUID> AIScriptDB<UUID>::GetDecoratorWhile(std::string inName)
	{
		auto iter = _decoratorWhileMap.find(inName);

		if (iter == _decoratorWhileMap.end())
		{
			return DecoratorWhile<UUID>();
		}

		return iter->second;
	}

	template <typename UUID>
	ExecutionNode<UUID> AIScriptDB<UUID>::GetExecutionNode(std::string inName)
	{
		auto iter = _executionNodeMap.find(inName);

		if (iter == _executionNodeMap.end())
		{
			return ExecutionNode<UUID>();
		}

		return iter->second;
	}
}