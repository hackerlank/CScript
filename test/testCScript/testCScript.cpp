#include "stdafx.h"
#include "CScriptEng/CScriptEng.h"
#include "CScriptEng/vm.h"
#include <iostream>

int ExecuteCode(const std::string &filePathName, bool saveCodeToFile = false);

int RunTestCase();

int main(int argc, char* argv[])
{
	int r;
	scriptAPI::SimpleCScriptEng::Init();
	if (argc == 1)
	{
		r = RunTestCase();
	}
	else
	{
		r = ExecuteCode(argv[1], true);
	}
	scriptAPI::SimpleCScriptEng::Term();
	return r;
}

int RunTestCase()
{
	std::string appDir;

#if defined(PLATFORM_WINDOWS)
	appDir.resize(MAX_PATH);
	appDir.resize(::GetModuleFileNameA(NULL, &appDir[0], MAX_PATH));
	appDir.resize(appDir.rfind('\\') + 1);

	appDir += "..\\test\\script\\";
#else
	appDir = "/mnt/CScript/test/script/";
#endif

	notstd::CFindIterator fi(appDir);
	for (notstd::CFindResult fr = fi.begin(); fr != fi.end(); fr = fi.next())
	{
		if (!fr.IsDirectory() && fr.GetSuffix() == "c")
		{
			std::cout << "Find file " << fr.GetPath() << std::endl;
			ExecuteCode(fr.GetPath());
		}
	}

	return 0;
}

int ExecuteCode(const std::string &filePathName, bool saveCodeToFile)
{
	std::cout << "Source file: " << filePathName << std::endl;

	std::string fName = filePathName;
	scriptAPI::FileStream fs(fName.c_str());
	scriptAPI::ScriptCompiler compiler;

	std::cout << "Compile file " << filePathName << std::endl;
	HANDLE h = compiler.Compile(&fs, true);
	if (h)
	{
		if (saveCodeToFile)
		{
			// 保存到和被执行的代码相同的路径中，扩展名为.txt
			std::string codePath = filePathName + ".txt";
			FILE *file = NULL;
			file = fopen(codePath.c_str(), "wb");
			do {
				if (!file)
				{
					std::cout << "Open code file file.\n";
					break;
				}
				if (
					compiler.SaveCodeToFile(h, file) < 0
					|| compiler.SaveConstStringTableInResultToFile(h, file) < 0)
				{
					std::cout << "Save code fail.\n";
				}
				::fclose(file);
			} while (0);
		}

		std::cout << "Compile file success. Start to execute. " << std::endl;
		scriptAPI::ScriptRuntimeContext *runtimeContext
			= scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext(1024, 512);
		runtimeContext->Execute(h);
		scriptAPI::ScriptCompiler::ReleaseCompileResult(h);
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(runtimeContext);
		return 0;
	}
	else
		std::cout << "Compile file " << filePathName << " fail. \n\n" << std::endl;

	return -1;
}
