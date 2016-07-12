#include "Common.h"
#include <fstream>
#include <cassert>
#include "Common/Log.h"

using namespace osuCrypto;

static std::fstream* file = nullptr;
std::string testData("../..");

void InitDebugPrinting(std::string filePath)
{
	Log::out << "changing sink" << Log::endl;

	if (file == nullptr)
	{
		file = new std::fstream;
	}
	else
	{
		file->close();
	}

	file->open(filePath, std::ios::trunc | std::ofstream::out);
	if (!file->is_open())
		throw std::runtime_error("");


	//time_t now = time(0);

	Log::SetSink(*file);

	
	//Log::out << "Test - " << ctime(&now) << Log::endl;
}
