#pragma once
#include <cstdint>

#include <memory>
#include "Frontend/Lexer/Token.h"

namespace Spliwaca
{
	enum class TypeErrorType : uint8_t
	{

	};

	class TypeError
	{
	public:
		TypeError(TypeErrorType errorCode, std::shared_ptr<Token> token)
			: m_ErrorCode(errorCode), m_LineNumber(token->GetLineNumber()), m_ColumnNumber(token->GetCharacterNumber()),
			  m_ColumnSpan(token->GetContents().length()), m_TokenType(token->GetType())
		{}

		TypeError(TypeErrorType errorCode, uint32_t lineNumber, uint32_t columnNumber, size_t columnSpan, Spliwaca::TokenType type)
			: m_ErrorCode(errorCode), m_LineNumber(lineNumber), m_ColumnNumber(columnNumber),
			m_ColumnSpan(columnSpan), m_TokenType(type)
		{}

		~TypeError() = default;

		inline const TypeErrorType  GetErrorCode() const { return m_ErrorCode; }
		inline const uint32_t GetLineNumber() const { return m_LineNumber; }
		inline const uint32_t GetColumnNumber() const { return m_ColumnNumber; }
		inline const size_t GetColumnSpan() const { return m_ColumnSpan; }
		inline const TokenType GetTokenType() const { return m_TokenType; }

		//2^8 = 256 error codes available
		//Code 0: Unexpected Token -- expected newline
		//Code 1: Unexpected Token -- expected identifier
		//Code 2: Unexpected Token -- expected statement
		//Code 3: Unexpected Token -- expected do
		//Code 4: Unexpected Token -- expected "END IF" token pair
		//Code 5: Unexpected Token "ELSE" -- cannot have more than one else in an if tree

	private:
		TypeErrorType m_ErrorCode;
		uint32_t m_LineNumber, m_ColumnNumber;
		size_t m_ColumnSpan;
		TokenType m_TokenType;
		//std::shared_ptr<Token> m_OffendingToken;
	};
}