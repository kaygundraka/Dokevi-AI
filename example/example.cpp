#include <iostream>
#include "../AIScriptDB.h"

using namespace DokeviAI;

const int MAX_CHAR = 5;
int pos[MAX_CHAR] = { 0, 4, 2, 7, 10 };
int targetId = -1;

int main()
{
	////////////////////////////////////////////////////////////////////
	// Regist AI Nodes
	////////////////////////////////////////////////////////////////////

    std::cout << "[Regist AI Nodes]" << std::endl;

	AIScriptDB<int>::GetInstance()->RegistDecoratorIf("Search", AI_CHECK_CONDITION_LAMBDA(int){
		const auto id = uuid;
		const double distance = constDoubleMap.find("dist")->second;

		for (int i = 0; i < MAX_CHAR; i++)
		{
			if (i == id)
			{
				continue;
			}

			const int temp = pos[i] - pos[id];

			if (temp <= 0)
				continue;

			if (temp < distance)
			{
				std::cout << "search : target = " << i << ", pos = " << pos[i] << std::endl;
				targetId = i;
				return true;
			}
		}

		std::cout << "search : cannot found any targets" << std::endl;

		return false;
	});

	AIScriptDB<int>::GetInstance()->RegistExecutionNode("Follow", AI_RUNIMPL_LAMBDA(int){
		const auto id = uuid;

		if (targetId == -1)
			return DokeviAI::NODE_RESULT_TYPE::NST_FAILURE;

		if (pos[id] < pos[targetId])
		{
			pos[id]++;
			std::cout << "follow : cur pos =" << pos[id] << std::endl;
			return DokeviAI::NODE_RESULT_TYPE::NST_RUNNING;
		}
		else
		{
			return DokeviAI::NODE_RESULT_TYPE::NST_SUCCESS;
		}
	});

	////////////////////////////////////////////////////////////////////
	// Load Scripts
	////////////////////////////////////////////////////////////////////

	std::cout << "[Load AI Scripts]" << std::endl;

	const bool isLoaded = AIScriptDB<int>::GetInstance()->LoadData(".\\scripts\\*.*");

	if (!isLoaded)
	{
		std::cout << "cannot load ai script datas" << std::endl;
		return 0;
	}

	////////////////////////////////////////////////////////////////////
	// Run Game Loop
	////////////////////////////////////////////////////////////////////

	auto agent = AIScriptDB<int>::GetInstance()->CreateAgent("BaseAI");

	while (true)
	{
		agent->_behaviourTree.Run(0, 1000);
		Sleep(1000);
	}
}