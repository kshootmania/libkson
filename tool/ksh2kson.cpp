#include <iostream>
#include <fstream>
#include <filesystem>
#include "kson/kson.hpp"

enum ExitCode : int
{
    kExitSuccess = 0,
    kExitNoArgument,
    kExitUncaughtException,
};

void PrintHelp()
{
    std::cerr <<
        "ksh2kson chart converter\n"
        "  Usage: ksh2kson [KSH file(s)...]\n"
        "  KSON file(s) are saved in the same folder with the extension \".kson\".\n";
}

void PrintError(kson::Error error)
{
    std::cerr << "Error: " << kson::GetErrorString(error) << '\n';
}

void DoConvert(const char *szInputFilePath)
{
    // Input
    std::cout << szInputFilePath << '\n';
    std::filesystem::path filePath(szInputFilePath);
    const kson::ChartData chartData = kson::LoadKSHChartData(filePath.string());
    if (chartData.error != kson::Error::None)
    {
        PrintError(chartData.error);
        std::cout << std::endl;
        return;
    }

    // Output
    std::cout << "-> ";
    filePath.replace_extension(".kson");
    const kson::Error error = kson::SaveKSONChartData(filePath.string(), chartData);
    if (error != kson::Error::None)
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
        return kExitUncaughtException;
    }
    catch (...)
    {
        std::cerr << "Error: Uncaught exception (unknown)\n";
        return kExitUncaughtException;
    }

    return kExitSuccess;
}
