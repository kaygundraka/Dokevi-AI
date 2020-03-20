# Dokevi-AI
> this project can use easier to make game ai in c++ server using json script.

![version](https://img.shields.io/badge/alpha-v1.0.0-blue.svg)
![language](https://img.shields.io/badge/language-c%2B%2B-green.svg)
![platform](https://img.shields.io/badge/platform-windows-brightgreen.svg)

Dokevi-AI provides to generate behaviour tree ai agent. you can write ai using json-sciprt.
When first turned on process, ai agent of script is cached on memory.

## Installation

If you want to build this project, just add this porject's source and header to your project.

## Guide and Example

+ AIScriptDB is singleton and script database class. When first turned on process, AIScriptDB must be initialized.
  <pre><code>// include header
  #include <AIScriptDB.h>
  
  // parameter : scrpit folder path string
  AIScriptDB::GetInstance()->LoadData("./sciprts/");
  </code></pre>

+ Adding DecorateIf Node 
  - a decorateIf node needs to name (used in scirpts) and lamda logic function.
  - function type : [](int uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    <pre><code>#define AI_CHECK_CONDITION_LAMBDA [](int uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    </code></pre>
  - you can regist lamda logic by AIScriptDB class.
    <pre><code>void RegistDecoratorIf(std::string inName, CheckConditionFuncType inCheckFunc);
    </code></pre>
  - example
    <pre><code>AIScriptDB::GetInstance()->RegistDecoratorIf("Search", AI_CHECK_CONDITION_LAMBDA{
        // uuid -> custum uuid in your game
        // add logic
        return true/false; -> behaviour tree decorate if result
      }
    );
    </code></pre>
    
+ Adding DecorateWhile Node 
  - a decorateWhile node needs to name (used in scirpts) and lamda logic function.
  - function type : [](int uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    <pre><code>#define AI_CHECK_CONDITION_LAMBDA [](int uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    </code></pre>
  - you can regist lamda logic by AIScriptDB class -> same decorateif

+ Adding ExecutionNode
  - a executionNode node needs to name (used in scirpts) and lamda logic function.
  - function type : [](int uuid, ConstDoubleMapType& constDoubleMap , ConstStringMapType& constStringMap) -> bool
    <pre><code>#define AI_RUNIMPL_LAMBDA [](int uuid, ConstDoubleMapType& constDoubleMap, ConstStringMapType& constStringMap, double timeDelta) -> NODE_RESULT_TYPE
    </code></pre>
  - you can regist lamda logic by AIScriptDB class
    <pre><code>AIScriptDB::GetInstance()->RegistExecutionNode("Follow", AI_RUNIMPL_LAMBDA{
        // uuid -> custum uuid in your game
        // add logic
        return NST_FAILURE/NST_SUCCESS;
      }
    );
    </code></pre>

## Write Script

A few motivating and useful examples of how your product can be used. Spice this up with code blocks and potentially more screenshots.

## Release History

* v1.0.0
    * Alpha: behaviour ai-agent

## Maker

KaygunDraka – 69277660@naver.com (you must write '[Dokevi-AI]' at email title)

Distributed under the MIT license.
