#pragma once
#include "Frontend/Lexer/Lexer.h"
#include "Frontend/Lexer/LexicalError.h"
#include "Frontend/Parser/Parser.h"
#include "Frontend/Parser/SyntaxError.h"
#include "Frontend/Parser/Typer.h"
#include "Backend/Generation/Python/Generator.h"
#include "Frontend/Parser/SemanticError.h"
#include "UtilFunctions.h"


namespace Spliwaca
{
	struct TranspilerState
	{
		std::vector<LexicalError> LexerErrors;
		std::vector<SyntaxError> SyntaxErrors;
		//std::vector<TypeError> TypeErrors;
	};

	class Transpiler
	{
	public:
		Transpiler(std::string filename, std::string output, std::shared_ptr<TranspilerState> state, bool printTokenList)
			: m_Filename(filename), m_Output(output), m_State(state), m_PrintTokenList(printTokenList)
		{
		}

		std::string Run();

	private:
		std::string GetSyntaxErrorMessage(SyntaxErrorType type)
		{
			switch (type)
			{
			case SyntaxErrorType::expNewline:   // 0
				return "Expected newline, got {0}";
			case SyntaxErrorType::expIdent:         // 1
				return "Expected identifier, got {0}";
			case SyntaxErrorType::expStatement:     // 2
				return "Expected statement beginning, got incompatible token type {0}";
			case SyntaxErrorType::expAtom:          // 3
				return "Expected atom (value/list), got incompatible token type {0}";
			case SyntaxErrorType::expType:          // 4
				return "Expected type, got {0}";
			case SyntaxErrorType::expComma:         // 5
				return "Expected a comma, got {0}";
			case SyntaxErrorType::expRParen:        // 6
				return "Expected a right parenthesis, got {0}";
			case SyntaxErrorType::expRSquareParen: // 7
				return "Expected right square parenthesis, got {0}";
			case SyntaxErrorType::expDo:         // 8
				return "Expected DO, got {0}";
			case SyntaxErrorType::expTo:	          // 9
				return "Expected TO, got {0}";
			case SyntaxErrorType::expOf:	          // 10
				return "Expected OF, got {0}";
			case SyntaxErrorType::expWith:          // 11
				return "Expected WITH, got {0}";
			case SyntaxErrorType::expTakes:        // 12
				return "Expected TAKES, got {0}. This error shouldn't happen, since TAKES is optional.";
			case SyntaxErrorType::expReturns:       // 13
				return "Expected RETURNS, got {0}. Functions must specify a return type and return a value.";
			case SyntaxErrorType::expAs:	          // 14
				return "Expected AS, got {0}";
			case SyntaxErrorType::expRaw:          // 15
				return "Expected a raw token i.e. a string. This error should not occur. Got {0}";
			case SyntaxErrorType::expEndIf:         // 16
				return "Expected END IF, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expEndFor:       // 17
				return "Expected END FOR, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expEndWhile:      // 18
				return "Expected END WHILE, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expEndFunc:	      // 19
				return "Expected END FUNC, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expEndProc:	      // 20
				return "Expected END PROC, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expEndStruct:     // 21
				return "Expected END STRUCT, got {0}. Reached the end of a statement block, but there was an incorrect END statement.";
			case SyntaxErrorType::expTypeMod:	      // 22
				return "Expected type or type modifier, got {0}. Input type must be specified and must be a primitive";
			case SyntaxErrorType::tooManyElse:      // 23
				return "Got a second ELSE in the if tree. There can only be one ELSE, and it must be last. Got {0}";
			case SyntaxErrorType::unexpEndFunc:     // 24
			case SyntaxErrorType::unexpEndProc:    // 25
			case SyntaxErrorType::unexpEndIf:	      // 26
			case SyntaxErrorType::unexpEndFor:      // 27
			case SyntaxErrorType::unexpEndWhile:  // 28
			case SyntaxErrorType::unexpEndStruct:  // 29
			case SyntaxErrorType::unexpElseIf:      // 30
				return "Unexpected end statement. This error is not implemented so please submit a bug report with the code causing it if you get it.";
			case SyntaxErrorType::inconsistentDict:
				return "Inconsistend dictionary: All items must be dictionary pairs, or all must be single atoms. It is not permissible to mix them. Got {0}";
			default:
				return "Unrecognised or unimplemented error code.";
			}
		}

	private:
		std::string m_Filename;
		std::string m_Output;
		std::shared_ptr<TranspilerState> m_State;
		bool m_PrintTokenList;
	};
}
