#include "stdafx.h"
#include <iostream>
#include "testCallback.h"
#include "svnTool.h"
#include "cRuntimeExt.h"

#define DECL_TEST_VAR	"declContent"
#define ARGV			"ARGV"

typedef std::list<std::string> ParamList;

struct ExecuteParam
{
	// 是否把文件内容作为代码来加载
	bool loadAsCode;
	// 是否在编译源代码后，保存字节码和字符串表到文件
	bool saveCodeToFile;

	// 上面的两个开关loadAsCode的优先级高，如果设置了loadAsCode，则saveCodeToFile就认为是false了

	ExecuteParam()
		: loadAsCode(false)
		, saveCodeToFile(false)
	{
	}

	void Normalize()
	{
		if (loadAsCode)
			saveCodeToFile = false;
	}
};

int ExecuteCode(const std::string &filePathName, const ParamList &pl, const ExecuteParam *ep = nullptr);

int RunTestCase(const ParamList &pl);

//
// testCSCript的参数说明
// testCScript.exe [外部开关1 [外部开关2 ...]] [内部参数1 [内部参数2 ...]]
// 其中，外部开关是给testCScript自身使用的开关；
// 内部参数是给脚本使用的参数，在脚本中可以通过ARGV变量，通过索引来访问它们
// 每个内部参数外都可以用双引号括起来，这时引号中间可以有空格
// 每个外部参数都由‘/’开头，后面是开关名或者是选项名（中间不能有空格）
// 有的外部参数可以有更多的配置信息，这些配置信息和开关之间加空格即可
// 配置信息若存在空格，则可以用双引号括起来
// ‘/’后是开关还是选项，由testCScript决定，目前的选项有
// 1. sf	后面跟待处理的源代码的路径
// 目前的开关有
// 1. bytecode	表示试图直接加载字节码文件进行处理，而不是加载源代码编译后执行
// 2. 如果没有指定sf选项，同时指定了runtest开关，则执行测试用例
// 
// 如果没有指定sf，但是有一个参数放入了pl中，则认为它是待执行的源代码文件
// 这是为了方便在外壳程序中直接执行源代码
//

int ParseCommandLine(int argc, char *argv[], std::string &pathName, ParamList &pl, ExecuteParam &ep);

int main(int argc, char* argv[])
{
	int r;
	scriptAPI::SimpleCScriptEng::Init();

	ParamList pl;
	ExecuteParam ep;
	std::string filePathName;

	if (ParseCommandLine(argc, argv, filePathName, pl, ep) < 0)
		return 1;
	if (filePathName.empty() && pl.size() == 1)
	{
		filePathName = pl.front();
		pl.clear();
	}

	if (filePathName.empty())
		r = RunTestCase(pl);
	else
		r = ExecuteCode(filePathName, pl, &ep);

	scriptAPI::SimpleCScriptEng::Term();
	return r;
}

int OnQuotation(int &i, int argc, const std::string &thisString, std::string &stringInQuotation)
{
	stringInQuotation.clear();
	for (; i < argc; i++)
	{
		bool endWithQuotation = notstd::StringHelper::Right(thisString, 1) == "\"";
		stringInQuotation += endWithQuotation ? thisString.substr(0, thisString.size() - 1) : thisString;
		if (endWithQuotation)
		{
			i++;
			return 0;
		}
		stringInQuotation += " ";
	}
	return -1;
}

int ParseCommandLine(int argc, char *argv[], std::string &pathName, ParamList &pl, ExecuteParam &ep)
{
	bool waitOption = false;
	std::string theOption;
	for (int i = 1; i < argc; i++)
	{
		std::string ss = argv[i];

		if (waitOption)
		{
			waitOption = false;
			if (notstd::StringHelper::Left(ss, 1) == "\"")
			{
				int r = OnQuotation(i, argc, notstd::StringHelper::Mid(ss, 1), ss);
				if (r < 0)
					return r;
			}
			if (theOption == "sf")
				pathName = ss;
		}
		else
		{
			std::string t = notstd::StringHelper::Left(ss, 1);
			if (t == "/")
			{
				theOption = notstd::StringHelper::Mid(ss, 1);
				if (theOption == "sf")
					waitOption = true;
				else if (theOption == "bytecode")
				{
					ep.loadAsCode = true;
					ep.saveCodeToFile = false;
				}
				else if (theOption == "gc")
				{
					ep.loadAsCode = false;
					ep.saveCodeToFile = true;
				}
				else
				{
					std::cout << "Unknown switch " << theOption << ", ignored.\n";
				}

				continue;
			}
			else
			{
				if (t == "\"")
				{
					int r = OnQuotation(i, argc, notstd::StringHelper::Mid(ss, 1), ss);
					if (r < 0)
						return r;
				}
			}

			pl.push_back(ss);
		}
	}

	if (waitOption)
	{
		std::cout << "Expect option value(" << theOption.c_str() << ")\n";
		return -1;
	}
	notstd::StringHelper::Trim(pathName, " ");

	return 0;
}

int RunTestCase(const ParamList &pl)
{
	std::string appDir = notstd::AppHelper::GetAppPath();
	if (appDir.empty())
		return -1;
	appDir.resize(appDir.rfind(notstd::AppHelper::splash[0]) + 1);
	
#define SPLASH notstd::AppHelper::splash
#if defined(PLATFORM_WINDOWS)
	appDir.append("..").append(SPLASH);
#endif
	appDir.append("..").append(SPLASH).append("test").
		append(SPLASH).append("script").append(SPLASH);

	notstd::CFindIterator fi(appDir);
	for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
	{
		if (!fr.IsDirectory() && fr.GetSuffix() == "c")
		{
			std::cout << "Find file " << fr.GetPath() << std::endl;
			if (ExecuteCode(fr.GetPath(), pl) < 0)
				break;
		}
	}

	return 0;
}

int ExecuteCode(const std::string &filePathName, const ParamList &pl, const ExecuteParam *ep)
{
	ExecuteParam executeParam;
	if (ep)
	{
		executeParam = *ep;
		executeParam.Normalize();
	}
	std::cout << "Source file: " << filePathName << std::endl;

	std::string fName = filePathName;
	scriptAPI::FileStream fs(fName.c_str());
	scriptAPI::ScriptCompiler compiler;

	std::cout << "Compile file " << filePathName << std::endl;

	runtime::arrayObject *argvArray = new runtime::ObjectModule<runtime::arrayObject>;
	runtime::stringObject *singleArgv;
	for (auto iter = pl.begin(); iter != pl.end(); iter++)
	{
		singleArgv = new runtime::ObjectModule<runtime::stringObject>;
		*singleArgv->mVal = *iter;
		argvArray->AddSub(singleArgv);
	}
	runtime::stringObject *declContent = new runtime::ObjectModule<runtime::stringObject>;
	*declContent->mVal = "testCScript v1.0";

	scriptAPI::ScriptLibReg les[] =
	{
		{ ARGV, argvArray, },
		{ DECL_TEST_VAR, declContent, },
		{ TESTCALLBACKNAME, new runtime::ObjectModule<TestCallback>, },
		{ "SvnTool", new runtime::ObjectModule<tools::svnTools>, },
		{ "cruntime", new runtime::ObjectModule<tools::CRuntimeExtObj>, },
	};

	compiler.GetLibRegister().RegistLib("testCScriptRuntime", les, sizeof(les) / sizeof(les[0]));

	int r = 0;
	HANDLE h = nullptr;
	if (!executeParam.loadAsCode)
	{
		h = compiler.Compile(&fs, true);
		do {
			if (!h)
			{
				std::cout << "Compile file " << filePathName << " fail. \n\n" << std::endl;
				r = -1;
				break;
			}
			if (executeParam.saveCodeToFile)
			{
				std::string codePath = filePathName + ".txt";
				Handle<STDCFILEHandle> file = fopen(codePath.c_str(), "wb");
				if (!file) {
					std::cout << "Open code file file.\n";
				}
				if (compiler.SaveCodeToFile(h, file) < 0
					|| compiler.SaveConstStringTableInResultToFile(h, file) < 0)
				{
					std::cout << "Save code to file fail\n";
				}
			}
			std::cout << "Compile file success. Start to execute. \n";
		} while (0);
	}
	else
	{
		// 先用命令行中指定的文件名加载字节码，如果加载不上，再在文件名尾部加上
		// .txt后缀后尝试加载一下
		Handle<STDCFILEHandle> file = file = ::fopen(fName.c_str(), "rb");
		do {
			if (!file)
				file = ::fopen((fName + ".txt").c_str(), "rb");
			if (file)
			{
				h = scriptAPI::ScriptCompiler::CreateCompileResult();
				if (h)
				{
					if (compiler.LoadCodeFromFile(h, file) >= 0)
					{
						if (compiler.LoadConstStringTableToResultFromFile(h, file) >= 0)
						{
							break;
						}
					}
				}
			}
			std::cout << "Load byte code from file fail\n";
			if (h)
			{
				scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
				h = nullptr;
			}
			r = -1;
		} while (0);
	}

	do {
		if (!h) {
			// 运行到这里说明编译或者加载字节码失败
			break;
		}

		scriptAPI::ScriptRuntimeContext *runtimeContext
			= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(1024, 512);
		runtimeContext->PushRuntimeObjectInLibs(&compiler.GetLibRegister());
		int exitCode = 0;
		int er = runtimeContext->Execute(h, true, &exitCode);
		scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
		if (er != runtime::EC_Normal)
		{
			std::cerr << "Execute return fail: " << er << std::endl;
			r = -1;
		}
		else
		{
			std::cout << "Execute finished, return value is " << exitCode << std::endl;
			r = 0;
		}
	} while (0);

#if defined(PLATFORM_WINDOWS)
	if (r < 0)
	{
		system("pause");
	}
#endif

	return r;
}
