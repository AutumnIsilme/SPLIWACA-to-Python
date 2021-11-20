#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>

#ifdef SPLW_WINDOWS
#include <Windows.h>
#endif

//#include "Instrumentor.h"
#include "Transpiler.h"
#include "Log.h"

using namespace Spliwaca;

std::shared_ptr<TranspilerState> state = std::make_shared<TranspilerState>();

//------------------------------------- UtilFunctions utility function definitions -------------------------------
class MissingVariable
{
	uint32_t lineNumber;
	uint32_t columnNumber;

public:
	MissingVariable(uint32_t lineNumber, uint32_t columnNumber)
		: lineNumber(lineNumber), columnNumber(columnNumber)
	{}

	uint32_t GetLineNumber() const { return lineNumber; }
	uint32_t GetColumnNumber() const { return columnNumber; }
	uint32_t GetColumnSpan() const { return 1; }
};

int RegisterLexicalError(uint8_t errorCode, uint32_t lineNumber, uint32_t columnNumber, uint16_t columnSpan)
{
	state->LexerErrors.push_back({ errorCode, lineNumber, columnNumber, columnSpan });
	return 1;
}

int RegisterSyntaxError(SyntaxErrorType type, std::shared_ptr<Token> token)
{
	state->SyntaxErrors.push_back({ type, token });
	return 1;
}

int RegisterSyntaxError(SyntaxErrorType errorCode, uint32_t lineNumber, uint32_t columnNumber, size_t columnSpan, Spliwaca::TokenType type)
{
	state->SyntaxErrors.push_back({ errorCode, lineNumber, columnNumber, columnSpan, type });
	return 1;
}

int RegisterSemanticsError(uint32_t lineNumber, uint32_t columnNumber)
{
	//state->SemanticErrors.push_back(MissingVariable(lineNumber, columnNumber));
	return 1;
}

std::string mulString(std::string s, uint32_t i)
{
	if (i <= 0)
		return "";
	std::string init = s;
	for (size_t j = 0; j < i; j++)
	{
		s.append(init);
	}
	return s;
}

/* Code snippet copied from https://stackoverflow.com/questions/1489830/efficient-way-to-determine-number-of-digits-in-an-integer
   accepted answer */
int numDigits(int32_t x)
{
	if (x >= 10000)
	{
		if (x >= 10000000)
		{
			if (x >= 100000000)
			{
				if (x >= 1000000000)
					return 10;
				return 9;
			}
			return 8;
		}
		if (x >= 100000)
		{
			if (x >= 1000000)
				return 7;
			return 6;
		}
		return 5;
	}
	if (x >= 100)
	{
		if (x >= 1000)
			return 4;
		return 3;
	}
	if (x >= 10)
		return 2;
	return 1;
}

bool charInStr(const std::string& s, char c)
{
	//PROFILE_FUNC();
	for (char ch : s)
	{
		if (ch == c)
		{
			return true;
		}
	}
	return false;
}

template <typename T>
bool itemInVect(const std::vector<T>& v, T t)
{
	for (T e : v)
	{
		if (e == t)
		{
			return true;
		}
	}
	return false;
}

//------------------------------------- End UtilFunctions utility function definitions -------------------------------

enum TranspileTarget {
	NONE = 0,
	PYTHON = 1,
	LLVM = 2,

	TRANSPILE_TARGET_MAX = LLVM
};

struct transpilerOptions
{
	std::string ifile;
	std::string ofile;
	bool recursive_transpile;
	TranspileTarget transpile_target;
};

// @TODO: Expand to give all args
void print_help_text() {
	std::cout << "Usage: transpiler FILE [options]\n" <<
	"Options:\n"
	"    -o <filename>    | Write the compiler output to the filename provided\n"
	"    -target=<target> | Set the compiler target. Options are:\n"
	"                     |  - none\n"
	"                     |  - python\n"
	"                     |  - llvm\n";
}

transpilerOptions* parseCommandLineArgs(int argc, char** argv)
{
	PROFILE_FUNCTION();
	transpilerOptions* options = new transpilerOptions();
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		print_help_text();
		exit(-1);
	}
	options->ifile = argv[1];
	auto arg_index = 2;



	return options;
}

int main(int argc, char** argv)
{
	PROFILE_BEGIN_SESSION("Run", "splw-run.json");
	PROFILE_FUNCTION();
	#if ENABLE_TIMERS
	//Timer timer = Timer();
	#endif
	transpilerOptions* options = parseCommandLineArgs(argc, argv);

	/*
	{
		PROFILE_SCOPE("Set up unicode output");
		#ifdef SPLW_WINDOWS
		SetConsoleOutputCP(CP_UTF8);
		setvbuf(stdout, nullptr, _IOFBF, 1000);
		#endif
	}
	*/

	LOG_INIT();
	bool printTokenList = false;

	Transpiler transpiler = Transpiler(options->ifile, options->ofile, state, printTokenList);
	std::string output = transpiler.Run();

	#if ENABLE_PROFILING
	//timer_180.Stop();
	//std::cout << "#Total time taken: " << timer_192.Elapsed() << "ms" << std::endl;
	#endif
	#if ENABLE_TIMERS
	//std::cout << "#Total time taken: " << timer.elapsed() << "s" << std::endl;
	#endif
	PROFILE_END_SESSION();
	return 0;
}
