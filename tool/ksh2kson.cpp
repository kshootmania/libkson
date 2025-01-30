#include <iostream>
#include <fstream>
#include <filesystem>
#include "kson/kson.hpp"

enum ExitCode : int
{
	kExitSuccess = 0,
	kExitNoArgument,
};

void PrintHelp()
{
	std::cerr <<
		"ksh2kson chart converter\n"
		"  Usage: ksh2kson [KSH file(s)...]\n"
		"  KSON file(s) are saved in the same folder with the extension \".kson\".\n";
}

void PrintError(kson::ErrorType errorType)
{
	std::cerr << "Error: " << kson::GetErrorString(errorType) << '\n';
}

void DoConvert(const char *szInputFilePath)
{
	// Input
	std::cout << szInputFilePath << '\n';
	std::filesystem::path filePath(szInputFilePath);
	const kson::ChartData chartData = kson::LoadKSHChartData(filePath.string());
	if (chartData.error != kson::ErrorType::None)
	{
		PrintError(chartData.error);
		std::cout << std::endl;
		return;
	}

	// Output
	std::cout << "-> ";
	filePath.replace_extension(".kson");
	const kson::ErrorType error = kson::SaveKSONChartData(filePath.string(), chartData);
	if (error != kson::ErrorType::None)
	{
		PrintError(error);
		std::cout << std::endl;
		return;
	}
	std::cout << "Saved: " << filePath.string() << '\n' << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		PrintHelp();
		return kExitNoArgument;
	}

	try
	{
		for (int i = 1; i < argc; ++i)
		{
			DoConvert(argv[i]);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: Uncaught exception '" << e.what() << "'\n";
	}
	catch (...)
	{
		std::cerr << "Error: Uncaught exception (unknown)\n";
	}

	return kExitSuccess;
}
