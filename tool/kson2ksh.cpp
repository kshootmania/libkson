#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "kson/kson.hpp"

enum ExitCode : int
{
	kExitSuccess = 0,
	kExitNoArgument,
	kExitError,
};

void PrintHelp()
{
	std::cerr <<
		"kson2ksh chart converter\n"
		"  Usage:\n"
		"    kson2ksh <input.kson>         Convert file and output to stdout\n"
		"    kson2ksh < input.kson         Read from stdin and output to stdout\n"
		"    cat input.kson | kson2ksh     Read from pipe and output to stdout\n";
}

void PrintError(kson::ErrorType errorType)
{
	std::cerr << "Error: " << kson::GetErrorString(errorType) << '\n';
}

int DoConvert(std::istream& input)
{
	const kson::ChartData chartData = kson::LoadKsonChartData(input);
	if (chartData.error != kson::ErrorType::None)
	{
		PrintError(chartData.error);
		return kExitError;
	}

	const kson::ErrorType error = kson::SaveKshChartData(std::cout, chartData);
	if (error != kson::ErrorType::None)
	{
		PrintError(error);
		return kExitError;
	}

	return kExitSuccess;
}

int main(int argc, char *argv[])
{
	try
	{
		if (argc == 1)
		{
			// Read from stdin
			return DoConvert(std::cin);
		}
		else if (argc == 2)
		{
			// Read from file
			std::ifstream ifs{ argv[1] };
			if (!ifs)
			{
				std::cerr << "Error: Cannot open file: " << argv[1] << '\n';
				return kExitError;
			}
			return DoConvert(ifs);
		}
		else
		{
			PrintHelp();
			return kExitNoArgument;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: Uncaught exception '" << e.what() << "'\n";
		return kExitError;
	}
	catch (...)
	{
		std::cerr << "Error: Uncaught exception (unknown)\n";
		return kExitError;
	}
}
