# Dokevi-AI
> The Dokevi-AI helps you make game ai easier on c++ client/server using json script.

![version](https://img.shields.io/badge/alpha-v1.0.0-blue.svg)
![language](https://img.shields.io/badge/language-c%2B%2B-green.svg)
![platform](https://img.shields.io/badge/platform-windows-brightgreen.svg)

Dokevi-AI provides to generate behaviour tree ai agent. you can write ai using json-sciprt.
When first turned on process, ai agent of script is cached on memory.

This project include rapid-json library. (BSD License Open Source Library)
https://github.com/Tencent/rapidjson

## Installation

If you want to build this project, just add this porject's header to your project.
> Only used header file type.

## Guide and Example

+ AIScriptDB is singleton and script database class. When first turned on process, AIScriptDB must be initialized.
  ```cpp
  // include header
  #include <AIScriptDB.h>
  
  // parameter : scrpit folder path string
  AIScriptDB<UUID>::GetInstance()->LoadData(".\scripts\ai\*.*");
  ```
  - The template-type means game object id-type. The UUID is used to identify the ai game object. The default option is int-type.

+ Adding DecorateIf Node 
  - a decorateIf node needs to name (used in scirpts) and lamda logic function.
  - function type : [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    ```cpp
    #define AI_CHECK_CONDITION_LAMBDA(UUID) [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    ```
  - you can regist lamda logic by AIScriptDB class.
    ```cpp
    void RegistDecoratorIf<UUID>(std::string inName, CheckConditionFuncType<UUID> inCheckFunc);
    ```
  - example
    ```cpp
    AIScriptDB<UUID>::GetInstance()->RegistDecoratorIf<UUID>("Search", AI_CHECK_CONDITION_LAMBDA(UUID){
        // uuid -> custum uuid in your game
        // add logic
        return true/false; -> behaviour tree decorate if result
      }
    );
    ```
    
+ Adding DecorateWhile Node 
  - a decorateWhile node needs to name (used in scirpts) and lamda logic function.
  - function type : [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    ```cpp
    #define AI_CHECK_CONDITION_LAMBDA(UUID) [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    ```
  - you can regist lamda logic by AIScriptDB class -> same decorateif

+ Adding ExecutionNode
  - a executionNode node needs to name (used in scirpts) and lamda logic function.
  - function type : [](UUID uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    ```cpp
    #define AI_RUNIMPL_LAMBDA(UUID) [](UUID uuid, ConstDoubleMapType& constDoubleMap, ConstStringMapType& constStringMap, double timeDelta) -> NODE_RESULT_TYPE
    ```
  - you can regist lamda logic by AIScriptDB class
    ```cpp
    AIScriptDB<UUID>::GetInstance()->RegistExecutionNode("Follow", AI_RUNIMPL_LAMBDA(UUID) {
        // uuid -> custum uuid in your game
        // add logic
        return NST_FAILURE/NST_SUCCESS;
      }
    );
    ```
    
+ Generate Agent
  ```cpp
  std::shared_ptr<Agent<UUID>> AIScriptDB<UUID>::CreateAgent(std::string inAiName)
  ```

+ Update Agent
  - The agent update at game update time using uuid and elapsed time.
  ```cpp
  std::shared_ptr<Agent<UUID>> aiAgent;
  aiAgent->_behaviourTree.Run(uuid, timeDelta);
  ```

## Write Script

The ai-agent script type is json-file. A script file is mapped onle one ai. The ai-element must write the same name as the file.

You should write root node type as sequence node. If you add new node by code, you can use by name-element in script. const-array element is delivered cpp lamda parameter. (ConstDoubleMapType& constDoubleMap, ConstStringMapType& constStringMap) The root node is looping own child array.

+ example
  - The example show ai-agent behaviours. The agent search targets between 2 and 10 distances. And then if the agent confirm target, they follow target. If the agent can attack to target, they start attacking.
  - Control Flow Node : Sequence, Selector
  - Decorator Node : DecoratorIf, DecoratorWhile (user lamda define)
  - Execution Node : Execution (user lamda define)
  ```json
  {
    "ai": "baseAI",
    "behaviourTree": {
      "type": "sequence",
      "child": [
        {
          "type": "decoratorIf",
          "name": "Search",
          "const": [ { "nearDist": 2 }, { "farDist": 10 } ],
          "child": {
            "type": "execution",
            "name": "Follow",
            "const": []
          }
        },
        {
          "type": "decoratorIf",
          "name": "CanAttack",
          "const": [ { "nearDist": 0 }, { "farDist": 2 } ],
          "child": {
            "type": "execution",
            "name": "Attack",
            "const": []
          }
        }
      ]
    }
  }
  ```
  
## Release History

* v1.0.0
    * Alpha: behaviour ai-agent

## Maker

KaygunDraka – 69277660@naver.com (you must write '[Dokevi-AI]' at email title)

Distributed under the MIT license.
