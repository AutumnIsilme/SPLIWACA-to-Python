#include "Transpiler.h"
#include "Log.h"
#include <iostream>
#include <fstream>

namespace Spliwaca
{
	std::string Transpiler::Run()
	{
		//PROFILE_FUNCTION();
		//Timer lexerTimer = Timer();
		#if ENABLE_TIMERS
		double lexerTime, parserTime, typerTime, generatorTime;
		#endif

		std::shared_ptr<Lexer> lexer = nullptr;
		std::shared_ptr<std::vector<std::shared_ptr<Token>>> tokens = nullptr;
		{
			PROFILE_SCOPE("Lexer - Transpiler::Run");
			#if ENABLE_TIMERS
			Timer timer = Timer();
			#endif
			lexer = Lexer::Create(m_Filename);
			SPLW_INFO("Created lexer.");
			tokens = lexer->MakeTokens();


			for (LexicalError l : m_State->LexerErrors)
			{
				SPLW_ERROR("Lexical Error code {2} at line {0}, column {1}", l.GetLineNumber(), l.GetColumnNumber(), l.GetErrorCode());
				if (l.GetLineNumber() >= lexer->GetSplitFileString().size())
					SPLW_WARN("Line {0} out of range!", l.GetLineNumber());
				else
					SPLW_WARN("{0}", lexer->GetSplitFileString().at(l.GetLineNumber()));
				SPLW_WARN("{0}{1}", mulString(" ", l.GetColumnNumber() - 1), mulString("^", l.GetColumnSpan()));
				std::cout << "\n";
			}
			if (m_State->LexerErrors.size() > 0)
			{
				SPLW_ERROR("Lexical errors present: cannot continue to parsing stage.");
				system("PAUSE");
				return "";
			}
			else
				SPLW_INFO("Finished constructing tokens.");
			#if ENABLE_TIMERS
			lexerTime = timer.elapsed();
			#endif
		}

		//double lexerTime = lexerTimer.elapsed();

		if (m_PrintTokenList)
		{
			int i = 0;
			for (std::shared_ptr<Token> t : *tokens)
			{
				if (t->GetContents() == "\n")
					SPLW_TRACE("Token {0}: {1},{2} type: {3}, contents: {4}", i, t->GetLineNumber(), t->GetCharacterNumber(), TokenTypeName(t->GetType()), "\\n");//, mulString(" ", 3 - std::to_string(i).size()), mulString(" ", numDigits(lineCount) - std::to_string(t->GetLineNumber()).size()), mulString(" ", 3 - std::to_string(t->GetCharacterNumber()).size()), mulString(" ", 16 - TokenTypeName(t->GetType()).size()));
				else if (t->GetContents() == "\t")
					SPLW_TRACE("Token {0}: {1},{2} type: {3}, contents: {4}", i, t->GetLineNumber(), t->GetCharacterNumber(), TokenTypeName(t->GetType()), "\\t");//, mulString(" ", 3 - std::to_string(i).size()), mulString(" ", numDigits(lineCount) - std::to_string(t->GetLineNumber()).size()), mulString(" ", 3 - std::to_string(t->GetCharacterNumber()).size()), mulString(" ", 16 - TokenTypeName(t->GetType()).size()));
				else if (t->GetContents() == "\f")
					SPLW_TRACE("Token {0}: {1},{2} type: {3}, contents: {4}", i, t->GetLineNumber(), t->GetCharacterNumber(), TokenTypeName(t->GetType()), "\\f");//, mulString(" ", 3 - std::to_string(i).size()), mulString(" ", numDigits(lineCount) - std::to_string(t->GetLineNumber()).size()), mulString(" ", 3 - std::to_string(t->GetCharacterNumber()).size()), mulString(" ", 16 - TokenTypeName(t->GetType()).size()));
				else
					SPLW_TRACE("Token {0}: {1},{2} type: {3}, contents: {4}", i, t->GetLineNumber(), t->GetCharacterNumber(), TokenTypeName(t->GetType()), t->GetContents());//, mulString(" ", 3 - std::to_string(i).size()), mulString(" ", numDigits(lineCount) - std::to_string(t->GetLineNumber()).size()), mulString(" ", 3 - std::to_string(t->GetCharacterNumber()).size()), mulString(" ", 16 - TokenTypeName(t->GetType()).size()));
				i++;
			}
		}

		//Timer parseTimer = Timer();
		std::shared_ptr<Spliwaca::EntryPoint> ast;
		{
			#if ENABLE_TIMERS
			Timer timer = Timer();
			#endif
			PROFILE_SCOPE("Parser - Transpiler::Run");
			std::shared_ptr<Parser> parser = Parser::Create(tokens);
			SPLW_INFO("Created Parser.");
			ast = parser->ConstructAST();

			uint32_t prevLineNumber = 0, prevColNumber = 0;
			for (SyntaxError s : m_State->SyntaxErrors)
			{
				if (s.GetLineNumber() == prevLineNumber && s.GetColumnNumber() == prevColNumber)
					continue;
				else
				{
					prevLineNumber = s.GetLineNumber();
					prevColNumber = s.GetColumnNumber();
				}
				SPLW_ERROR("Syntax Error code {2} at line {0}, column {1}", s.GetLineNumber(), s.GetColumnNumber(), s.GetErrorCode());
				SPLW_ERROR(GetSyntaxErrorMessage(s.GetErrorCode()), TokenTypeName(s.GetTokenType()));
				if (s.GetLineNumber() >= lexer->GetSplitFileString().size())
					SPLW_WARN("Line {0} out of range!", s.GetLineNumber());
				else
					SPLW_WARN("{0}", lexer->GetSplitFileString().at(s.GetLineNumber()));
				SPLW_WARN("{0}{1}", mulString(" ", (uint32_t)s.GetColumnNumber() - 1), mulString("^", (uint32_t)s.GetColumnSpan()));
				std::cout << "\n";
			}
			if (m_State->SyntaxErrors.size() > 0)
			{
				SPLW_ERROR("Syntax errors present: cannot continue to next stage.");
				//if (m_State->MissingVariables.size() == 0)
				//{
				//}
				#ifdef SPLW_WINDOWS
				system("PAUSE");
				#else
				//system("read -n 1 -s -p \"Press any key to continue...\n\"");
				#endif
				return "";
			}
			else
				SPLW_INFO("Finished syntax analysis.");
			#if ENABLE_TIMERS
			parserTime = timer.elapsed();
			#endif
		}

		// Typer
		/*
		{
			#if ENABLE_TIMERS
			Timer timer = Timer();
			#endif
			PROFILE_SCOPE("Typer - Transpiler::Run");
			std::shared_ptr<Typer> typer = Typer::Create(tokens);
			SPLW_INFO("Created Typer.");
			ast = typer->ConstructAST();

			uint32_t prevLineNumber = 0, prevColNumber = 0;
			for (SyntaxError s : m_State->SyntaxErrors)
			{
				if (s.GetLineNumber() == prevLineNumber && s.GetColumnNumber() == prevColNumber)
					continue;
				else
				{
					prevLineNumber = s.GetLineNumber();
					prevColNumber = s.GetColumnNumber();
				}
				SPLW_ERROR("Syntax Error code {2} at line {0}, column {1}", s.GetLineNumber(), s.GetColumnNumber(), s.GetErrorCode());
				SPLW_ERROR(GetSyntaxErrorMessage(s.GetErrorCode()), TokenTypeName(s.GetTokenType()));
				if (s.GetLineNumber() >= lexer->GetSplitFileString().size())
					SPLW_WARN("Line {0} out of range!", s.GetLineNumber());
				else
					SPLW_WARN("{0}", lexer->GetSplitFileString().at(s.GetLineNumber()));
				SPLW_WARN("{0}{1}", mulString(" ", (uint32_t)s.GetColumnNumber() - 1), mulString("^", (uint32_t)s.GetColumnSpan()));
				std::cout << "\n";
			}
			if (m_State->SyntaxErrors.size() > 0)
			{
				SPLW_ERROR("Syntax errors present: cannot continue to next stage.");
				//if (m_State->MissingVariables.size() == 0)
				//{
				//}
				#ifdef SPLW_WINDOWS
				system("PAUSE");
				#else
				//system("read -n 1 -s -p \"Press any key to continue...\n\"");
				#endif
				return "";
			}
			else
				SPLW_INFO("Finished syntax analysis.");
			#if ENABLE_TIMERS
			typerTime = timer.elapsed();
			#endif
		}
		*/

		//double parseTime = parseTimer.elapsed();

		//Timer generateTimer = Timer();
		std::string finalCode;
		{
			#if ENABLE_TIMERS
			Timer timer = Timer();
			#endif
			PROFILE_SCOPE("Generator");
			std::shared_ptr<PythonGenerator> codeGenerator = PythonGenerator::Create(ast);
			SPLW_INFO("Created Generator");

			int errorCode = 0;
			finalCode = codeGenerator->GenerateCode(errorCode);
			if (errorCode)
			{
				SPLW_CRITICAL("Errors detected during generation. Aborting.");
				return "";
			}

			if (m_Output != "")
			{
				PROFILE_SCOPE("Generator file output");
				std::ofstream outputFile;
				outputFile.open(m_Output, std::ios::trunc);
				outputFile << finalCode << "\n";
				outputFile.close();
			}
			else
			{
				PROFILE_SCOPE("Generator cout output");
				std::cout << finalCode << std::endl;
			}
			#if ENABLE_TIMERS
			generatorTime = timer.elapsed();
			#endif
		}

		//double generateTime = generateTimer.elapsed();

		//std::cout << finalCode << std::endl;

		SPLW_INFO("Finished code output!");

		#if ENABLE_TIMERS
		std::cout << "Lexer time: " << lexerTime << ", Parser time: " << parserTime << ", Generator time: " << generatorTime << std::endl;
		#endif

		return finalCode;
	}
}