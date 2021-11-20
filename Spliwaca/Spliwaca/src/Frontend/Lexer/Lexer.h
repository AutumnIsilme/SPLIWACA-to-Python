#pragma once
#include "Token.h"
#include <memory>
#include <vector>
#include <map>
#include <sstream>

namespace Spliwaca
{
	class Lexer
	{
	public:
		static std::shared_ptr<Lexer> Create(std::string file);
		~Lexer() = default;

		std::shared_ptr<std::vector<std::shared_ptr<Token>>> MakeTokens();
		inline const std::string GetFileString() const { return m_FileString; }
		inline const std::vector<std::string> GetSplitFileString() const { return split(m_FileString, '\n'); }
		inline const bool IsStringInKeywords(std::string string) const { return s_KeywordDict.find(string) != s_KeywordDict.end(); }

	private:
		Lexer(std::string file);

		void makeToken(std::string tokenContents);

		template <typename Out>
		static void split(const std::string& s, char delim, Out result)
		{
			std::istringstream iss(s);
			std::string item;
			while (std::getline(iss, item, delim))
			{
				*result++ = item;
			}
		}

		static std::vector<std::string> split(const std::string& s, char delim)
		{
			std::vector<std::string> elems;
			split(s, delim, std::back_inserter(elems));
			return elems;
		}

	private:
		std::string m_FileLocation;
		std::shared_ptr<std::vector<std::shared_ptr<Token>>> m_Tokens;

		uint32_t m_LineNumber = 0, m_ColumnNumber = 0;
		uint32_t m_StoredLineNumber = 0, m_StoredColumnNumber = 0;

		//LEXER STATE FLAGS
		// double_quote single_quote raw block_comment line_comment
		char flags = 0;
		std::string persistent_contents;

		std::string m_FileString;

		const std::string alphabetCharacters = {
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
		};

		const std::map<std::string, TokenType> s_KeywordDict = {
			{u8"INT",                 TokenType::Type},
			{u8"Int",                 TokenType::Type},
			{u8"int",                 TokenType::Type},
			{u8"INTEGER",             TokenType::Type},
			{u8"Integer",             TokenType::Type},
			{u8"integer",             TokenType::Type},
			{u8"FLOAT",               TokenType::Type},
			{u8"Float",               TokenType::Type},
			{u8"float",               TokenType::Type},
			{u8"REAL",                TokenType::Type},
			{u8"Real",                TokenType::Type},
			{u8"real",                TokenType::Type},
			{u8"NUMBER",              TokenType::Type},
			{u8"Number",              TokenType::Type},
			{u8"number",              TokenType::Type},
			{u8"COMPLEX",             TokenType::Type},
			{u8"Complex",             TokenType::Type},
			{u8"complex",             TokenType::Type},
			{u8"STRING",              TokenType::Type},
			{u8"String",              TokenType::Type},
			{u8"string",              TokenType::Type},
			{u8"BOOL",                TokenType::Type},
			{u8"Bool",                TokenType::Type},
			{u8"bool",                TokenType::Type},
			{u8"STR",                 TokenType::Type},
			{u8"Str",                 TokenType::Type},
			{u8"str",                 TokenType::Type},
			{u8"DICT",                TokenType::Type},
			{u8"Dict",                TokenType::Type},
			{u8"dict",                TokenType::Type},
			{u8"DICTIONARY",          TokenType::Type},
			{u8"Dictionary",          TokenType::Type},
			{u8"dictionary",          TokenType::Type},
			{u8"MAP",                 TokenType::Type},
			{u8"Map",                 TokenType::Type},
			{u8"map",                 TokenType::Type},
			{u8"MAPPING",             TokenType::Type},
			{u8"Mapping",             TokenType::Type},
			{u8"mapping",             TokenType::Type},
			{u8"LIST",                TokenType::Type},
			{u8"List",                TokenType::Type},
			{u8"list",                TokenType::Type},
			{u8"ARRAY",               TokenType::Type},
			{u8"Array",               TokenType::Type},
			{u8"array",               TokenType::Type},
			{u8"TUPLE",               TokenType::Type},
			{u8"Tuple",               TokenType::Type},
			{u8"tuple",               TokenType::Type},
			{u8"NULL",                TokenType::None},
			{u8"Null",                TokenType::None},
			{u8"null",                TokenType::None},
			{u8"NONE",                TokenType::None},
			{u8"None",                TokenType::None},
			{u8"none",                TokenType::None},
			{u8"EMPTY",               TokenType::None},
			{u8"Empty",               TokenType::None},
			{u8"empty",               TokenType::None},
			{u8"INFINITY",            TokenType::Float},
			{u8"Infinity",            TokenType::Float},
			{u8"infinity",            TokenType::Float},
			{u8"NAN",                 TokenType::Float},
			{u8"NaN",                 TokenType::Float},
			{u8"nan",                 TokenType::Float},
			{u8"IS",                  TokenType::Is},
			{u8"≣",                   TokenType::Is},
			{u8"≡",                   TokenType::Is},
			{u8"===",                 TokenType::Is},
			{u8"NOT",                 TokenType::Not},
			{u8"!",                   TokenType::Not},
			{u8"AND",                 TokenType::And},
			{u8"&&",                  TokenType::And},
			{u8"∧",                   TokenType::And},
			{u8"OR",                  TokenType::Or},
			{u8"∨",                   TokenType::Or},
			{u8"||",                  TokenType::Or},
			{u8"=",                   TokenType::Equal},
			{u8"==",                  TokenType::Equal},
			{u8"EQUALS",              TokenType::Equal},
			{u8"!=",                  TokenType::NotEqual},
			{u8"≠",                   TokenType::NotEqual},
			{u8"=/=",                 TokenType::NotEqual},
			{u8"<",                   TokenType::LessThan},
			{u8"≨",                   TokenType::LessThan},
			{u8"≱",                   TokenType::LessThan},
			{u8">",                   TokenType::GreaterThan},
			{u8"≩",                   TokenType::GreaterThan},
			{u8"≰",                   TokenType::GreaterThan},
			{u8"<=",                  TokenType::LessThanEqual},
			{u8"≤",                   TokenType::LessThanEqual},
			{u8"⩽",                   TokenType::LessThanEqual},
			{u8"≦",                   TokenType::LessThanEqual},
			{u8"≯",                   TokenType::LessThanEqual},
			{u8"⩾",                   TokenType::GreaterThanEqual},
			{u8">=",                  TokenType::GreaterThanEqual},
			{u8"≥",                   TokenType::GreaterThanEqual},
			{u8"≧",                   TokenType::GreaterThanEqual},
			{u8"≮",                   TokenType::GreaterThanEqual},
			{u8"*",                   TokenType::Multiply},
			{u8"**",                  TokenType::Power},
			{u8"/",                   TokenType::Divide},
			{u8"DIVI",                TokenType::Intdiv},
			{u8"+",                   TokenType::Plus},
			{u8"-",                   TokenType::Minus},
			{u8"%",                   TokenType::Modulo},
			{u8"^",                   TokenType::Xor},
			{u8"&",                   TokenType::BitwiseAnd},
			{u8"|",                   TokenType::BitwiseOr},
			{u8">>",                  TokenType::ShiftRight},
			{u8"<<",                  TokenType::ShiftLeft},
			{u8"(",                   TokenType::LParen},
			{u8")",                   TokenType::RParen},
			{u8"RAW",                 TokenType::Raw},
			{u8"PLAINTEXT",           TokenType::Raw},
			{u8"INPUT",               TokenType::Input},
			{u8"POS",                 TokenType::PositiveTypeMod},
			{u8"POSITIVE",            TokenType::PositiveTypeMod},
			{u8"NEG",                 TokenType::NegativeTypeMod},
			{u8"NEGATIVE",            TokenType::NegativeTypeMod},
			{u8"NONZERO",             TokenType::NonZeroTypeMod},
			{u8"NONPOS",              TokenType::NegativeTypeMod},
			{u8"NONPOSITIVE",         TokenType::NegativeTypeMod},
			{u8"NONNEG",              TokenType::PositiveTypeMod},
			{u8"NONNEGATIVE",         TokenType::PositiveTypeMod},
			{u8"OUTPUT",              TokenType::Output},
			{u8"PRINT",               TokenType::Output},
			{u8"CREATE",              TokenType::Create},
			{u8"CAST",                TokenType::Cast},
			{u8"CALL",                TokenType::Call},
			{u8"WITH",                TokenType::With},
			{u8"QUIT",                TokenType::Quit},
			{u8"EXIT",                TokenType::Quit},
			{u8"STOP",                TokenType::Quit},
			{u8"REQUIRE",             TokenType::Require},
			{u8"INCREMENT",           TokenType::Increment},
			{u8"INC",                 TokenType::Increment},
			{u8"DECREMENT",           TokenType::Decrement},
			{u8"DEC",                 TokenType::Decrement},
			{u8"IF",                  TokenType::If},
			{u8"DO",                  TokenType::Do},
			{u8"ELSE",                TokenType::Else},
			{u8"FUNCTION",            TokenType::Function},
			{u8"FUNC",                TokenType::Function},
			{u8"TAKES",               TokenType::Takes},
			{u8"<-",                  TokenType::Takes},
			{u8"DEFAULT",             TokenType::Default},
			{u8"RETURNS",             TokenType::ReturnType},
			{u8"->",                  TokenType::ReturnType},
			{u8"AS",                  TokenType::As},
			{u8"RETURN",              TokenType::Return},
			{u8"PROCEDURE",           TokenType::Procedure},
			{u8"END",                 TokenType::End},
			{u8"ANONF",               TokenType::AnonFunc},
			{u8"ANONFUNC",            TokenType::AnonFunc},
			{u8"ANONFUNCTION",        TokenType::AnonFunc},
			{u8"ANONYMOUSF",          TokenType::AnonFunc},
			{u8"ANONYMOUSFUNC",       TokenType::AnonFunc},
			{u8"ANONYMOUSFUNCTION",   TokenType::AnonFunc},
			{u8"ANONP",               TokenType::AnonProc},
			{u8"ANONPROC",            TokenType::AnonProc},
			{u8"ANONPROCEDURE",       TokenType::AnonProc},
			{u8"ANONYMOUSP",          TokenType::AnonProc},
			{u8"ANONYMOUSPROC",       TokenType::AnonProc},
			{u8"ANONYMOUSPROCEDURE",  TokenType::AnonProc},
			{u8"SET",                 TokenType::Set},
			{u8"TO",                  TokenType::To},
			{u8"FOR",                 TokenType::For},
			{u8"OF",                  TokenType::Of},
			{u8"WHILE",               TokenType::While},
			{u8"STRUCTURE",           TokenType::Struct},
			{u8"STRUCT",              TokenType::Struct},
			{u8"BREAK",               TokenType::Break},
			{u8"IMPORT",              TokenType::Import},
			{u8"//",                  TokenType::SingleLineComment},
			{u8"/*",                  TokenType::StartMultiLineComment},
			{u8"*/",                  TokenType::EndMultiLineComment},
			{u8":",                   TokenType::DictEquator},
			{u8"\n",                  TokenType::Newline},
			{u8",",                   TokenType::Comma},
			{u8".",                   TokenType::VarAccessOp},
			{u8"NOIMPORT",            TokenType::NoImport},
			{u8"NOINSTALL",           TokenType::NoInstall},
			{u8"NOBARE",              TokenType::NoBare}
			/*{"\f",                  TokenType::Whitespace},
			//{"\u200b",              TokenType::Whitespace},
			{"\t",                    TokenType::Whitespace},
			{" ",                     TokenType::Whitespace}*/
		};
	};
}
