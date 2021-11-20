#include "Typer.h"
#include "UtilFunctions.h"

namespace Spliwaca
{
	std::shared_ptr<Typer> Typer::Create()
	{
		return std::shared_ptr<Typer>(new Typer());
	}

	VarType Typer::TypeAST()
	{
		//std::shared_ptr<EntryPoint> ep = std::make_shared<EntryPoint>();
		//m_MainScope = std::make_shared<Scope>("Main", 0, ScopeType::Main);
		//m_CurrentScope = m_MainScope;
		//m_ScopeStack.push_back(m_MainScope);

		ep->require = TypeRequire();

		// Consume newline after require
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Newline && ep->require)
		{
			RegisterSyntaxError(SyntaxErrorType::expNewline, m_Tokens->at(m_TokenIndex));
		}
		else if (ep->require)
			IncIndex(); 

		//Begin Typeing statements
		ep->statements = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::eof)
		{
			RegisterSyntaxError(SyntaxErrorType::expNewline, m_Tokens->at(m_TokenIndex));
			return ep;
		}
		else
			return ep;
	}

	VarType Typer::TypeRequire()
	{
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Require)
		{
			std::shared_ptr<RequireNode> node = std::make_shared<RequireNode>();
			IncIndex();
			node->requireType = TypeIdentNode();
			return node;
		}
		return nullptr;
	}

	VarType Typer::TypeStatements()
	{
		VarType statements = std::make_shared<Statements>();
		while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::eof)
		{
			//Attempt to consume newline
			while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
			{
				IncIndex();
			}

			TokenType currTokType = m_Tokens->at(m_TokenIndex)->GetType();
			if (currTokType == TokenType::eof)
			{
				//If we have reached the end of the file, return
				break;
			}
			else if (currTokType == TokenType::End)
			{
				//Check whether this END matches the type of statement block we are in. If not, register a syntax error complaining before any
				//other error.
				TokenType nextTokType = m_Tokens->at(m_TokenIndex+1)->GetType();
				return statements;
				//ScopeType currentScopeType = m_CurrentScope->GetType();
				if (nextTokType == TokenType::Function)// && currentScopeType != ScopeType::Function)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndFunc, m_Tokens->at(m_TokenIndex));
				}
				else if (nextTokType == TokenType::Procedure)// && currentScopeType != ScopeType::Procedure)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndProc, m_Tokens->at(m_TokenIndex));
				}
				else if (nextTokType == TokenType::If)// && currentScopeType != ScopeType::If)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndIf, m_Tokens->at(m_TokenIndex));
				}
				else if (nextTokType == TokenType::For)// && currentScopeType != ScopeType::For)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndFor, m_Tokens->at(m_TokenIndex));
				}
				else if (nextTokType == TokenType::While)// && currentScopeType != ScopeType::While)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndWhile, m_Tokens->at(m_TokenIndex));
				}
				else if (nextTokType == TokenType::Struct)// && currentScopeType != ScopeType::Struct)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpEndStruct, m_Tokens->at(m_TokenIndex));
				}
				else
				{
					return statements;
				}
			}
			else if (currTokType == TokenType::Else)
			{
				TokenType nextTokType = m_Tokens->at(m_TokenIndex + 1)->GetType();
				return statements;
				//ScopeType currentScopeType = m_CurrentScope->GetType();
				if (nextTokType == TokenType::If)// && currentScopeType != ScopeType::If)
				{
					RegisterSyntaxError(SyntaxErrorType::unexpElseIf, m_Tokens->at(m_TokenIndex));
				}
				else
				{
					return statements;
				}
			}

			//IncIndex();
			//Attempt to Type statement
			VarType s = TypeStatement();
			if (s != nullptr)
			{
				statements->statements.push_back(s);
			}
			else
			{
				//If we didn't get a statement back, then there was an error and we are finished, 
				//as we do not know what is supposed to happen here.
				break;
			}
		}
		return statements;
	}

	VarType Typer::TypeStatement()
	{
		VarType s = std::make_shared<Statement>();
		s->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		switch (m_Tokens->at(m_TokenIndex)->GetType())
		{
		case TokenType::If: s->ifNode = TypeIf(); s->statementType = 0; break;
		case TokenType::Set: s->setNode = TypeSet(); s->statementType = 1; break;
		case TokenType::Input: s->inputNode = TypeInput(); s->statementType = 2; break;
		case TokenType::Output: s->outputNode = TypeOutput(); s->statementType = 3; break;
		case TokenType::Increment: s->incNode = TypeIncrement(); s->statementType = 4; break;
		case TokenType::Decrement: s->decNode = TypeDecrement(); s->statementType = 5; break;
		case TokenType::For: s->forNode = TypeFor(); s->statementType = 6; break;
		case TokenType::While: s->whileNode = TypeWhile(); s->statementType = 7; break;
		case TokenType::Quit: s->quitNode = TypeQuit(); s->statementType = 8; break;
		case TokenType::Call: s->callNode = TypeCall(); s->statementType = 9; break;
		case TokenType::Function: s->funcNode = TypeFunction(); s->statementType = 10; break;
		case TokenType::Procedure: s->procNode = TypeProcedure(); s->statementType = 11; break;
		case TokenType::Struct: s->structNode = TypeStruct(); s->statementType = 12; break;
		case TokenType::Return: s->returnNode = TypeReturn(); s->statementType = 13; break;
		case TokenType::Import: s->importNode = TypeImport(); s->statementType = 14; break;
		case TokenType::NoImport: s->statementType = 15; IncIndex(); break;
		case TokenType::NoInstall: s->statementType = 16; IncIndex(); break;
		case TokenType::NoBare: s->statementType = 17; IncIndex(); break;
		default:
			RegisterSyntaxError(SyntaxErrorType::expStatement, m_Tokens->at(m_TokenIndex));
			return nullptr;
		}
		return s;
	}

	VarType Typer::TypeIf()
	{
		VarType node = std::make_shared<IfNode>();
		node->lineNumbers.push_back(m_Tokens->at(m_TokenIndex)->GetLineNumber());
		IncIndex();

		node->conditions.push_back(TypeList());
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Do)
		{
			//There must be a "DO"
			RegisterSyntaxError(SyntaxErrorType::expDo, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("IF_line_"+std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::If));
		//m_CurrentScope = m_ScopeStack.back();

		node->bodies.push_back(TypeStatements());

		//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();

		while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Else)
		{
			node->lineNumbers.push_back(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			if (node->elsePresent)
			{
				//We cannot have more than one else
				RegisterSyntaxError(SyntaxErrorType::tooManyElse, m_Tokens->at(m_TokenIndex));
			}
			IncIndex();
			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::If)
			{
				IncIndex();
				node->conditions.push_back(TypeList());
				if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Do)
				{
					//There must be a "DO"
					RegisterSyntaxError(SyntaxErrorType::expDo, m_Tokens->at(m_TokenIndex));
				}
			}
			else if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Do)
			{
				//There must be a "DO"
				RegisterSyntaxError(SyntaxErrorType::expDo, m_Tokens->at(m_TokenIndex));
			}
			else
			{
				node->elsePresent = true;
			}
			IncIndex();
			//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("IF_line_" + std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::If));
			//m_CurrentScope = m_ScopeStack.back();

			node->bodies.push_back(TypeStatements());

			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			//m_ScopeStack.pop_back();
			//m_CurrentScope = m_ScopeStack.back();
		}
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex+(uint64_t)1)->GetType() == TokenType::If)
		{
			IncIndex(); IncIndex();
		}
		else
		{
			//Must have an end if - something is wrong here
			RegisterSyntaxError(SyntaxErrorType::expEndIf, m_Tokens->at(m_TokenIndex));
		}
		return node;
	}

	VarType Typer::TypeSet()
	{
		VarType node = std::make_shared<SetNode>();
		IncIndex();
		node->id = TypeIdentNode();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::To)
		{
			RegisterSyntaxError(SyntaxErrorType::expTo, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->list = TypeList();
		//VarType varType;
		/*switch (node->expr->exprType)
		{
		case 1:
		{
			if (node->expr->listNode->Items.size() > 1 || node->expr->listNode->Items.at(0)->hasRight)
			{
				varType = VarType::List;
			}
			else
			{
				if (node->expr->listNode->Items.at(0)->left->right != nullptr)
				{
					varType = VarType::Bool;
				}
				else
				{
					auto atom = node->expr->listNode->Items.at(0)->left->left->left->left->left->left->right;
					while (atom->expression)
					{
						atom = atom->expression->listNode->Items.at(0)->left->left->left->left->left->left->right;
					}
					if (atom->ident != nullptr)
					{
						varType = m_CurrentScope->FindIdent(atom->ident)->GetSymbolType();
					}
					else if (atom->token->GetType() == TokenType::Int)
					{

					}
				}
			}
		}
		}*/

		//m_CurrentScope->AddEntry(node->id->GetContents(), node->id->GetLineNumber(), node->expr->GetExprReturnType());
		return node;
	}

	VarType Typer::TypeInput()
	{
		VarType node = std::make_shared<InputNode>();

		IncIndex();
		auto type = m_Tokens->at(m_TokenIndex)->GetType();
		if (type == TokenType::PositiveTypeMod || type == TokenType::NegativeTypeMod || type == TokenType::NonZeroTypeMod)
		{
			node->signSpec = m_Tokens->at(m_TokenIndex);
			IncIndex();
			if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Type) {
				RegisterSyntaxError(SyntaxErrorType::expTypeMod, m_Tokens->at(m_TokenIndex));
			} else {
				node->type = TypeTypeNode();
			}
		}
		else if (type == TokenType::Type)
		{
			node->type = TypeTypeNode();
		}
		else
		{
			RegisterSyntaxError(SyntaxErrorType::expTypeMod, m_Tokens->at(m_TokenIndex));
		}
		//IncIndex();

		node->id = TypeIdentNode();
		//m_CurrentScope->AddEntry(node->id->GetContents(), node->id->GetLineNumber());

		return node;
	}

	VarType Typer::TypeOutput()
	{
		VarType node = std::make_shared<OutputNode>();
		IncIndex();
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Raw)
		{
			RegisterSyntaxError(SyntaxErrorType::expRaw, m_Tokens->at(m_TokenIndex));
		}
		else
		{
			node->raw = m_Tokens->at(m_TokenIndex);
			IncIndex();
		}
		return node;
	}

	VarType Typer::TypeIncrement()
	{
		VarType node = std::make_shared<IncNode>();
		IncIndex();
		node->id = TypeIdentNode();
		return node;
	}

	VarType Typer::TypeDecrement()
	{
		VarType node = std::make_shared<DecNode>();
		IncIndex();
		node->id = TypeIdentNode();
		return node;
	}

	VarType Typer::TypeFor()
	{
		VarType node = std::make_shared<ForNode>();
		node->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		IncIndex();

		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("FOR_line_" + std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::For));
		//m_CurrentScope = m_ScopeStack.back();
		node->id = TypeIdentNode();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Of)
		{
			RegisterSyntaxError(SyntaxErrorType::expOf, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->iterableExpr = TypeList();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Do)
		{
			RegisterSyntaxError(SyntaxErrorType::expDo, m_Tokens->at(m_TokenIndex));
		}
		IncIndex();
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Newline)
		{
			RegisterSyntaxError(SyntaxErrorType::expNewline, m_Tokens->at(m_TokenIndex));
		}

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::For)
		{
			//CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//CurrentScope->CloseScope(0);
			RegisterSyntaxError(SyntaxErrorType::expEndFor, m_Tokens->at(m_TokenIndex));
		}

		//ScopeStack.pop_back();
		//CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeWhile()
	{
		VarType node = std::make_shared<WhileNode>();
		node->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		IncIndex();

		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("WHILE_line_" + std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::While));
		//m_CurrentScope = m_ScopeStack.back();

		node->condition = TypeBinOpNode();
		
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Do)
		{
			RegisterSyntaxError(SyntaxErrorType::expDo, m_Tokens->at(m_TokenIndex));
		}
		IncIndex();
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Newline)
		{
			RegisterSyntaxError(SyntaxErrorType::expNewline, m_Tokens->at(m_TokenIndex));
		}

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::While)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//m_CurrentScope->CloseScope(0);
			RegisterSyntaxError(SyntaxErrorType::expEndWhile, m_Tokens->at(m_TokenIndex));
		}

		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeQuit()
	{
		VarType node = std::make_shared<QuitNode>();
		IncIndex();
		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Newline)
			node->returnVal = TypeAtom(true);
		return node;
	}

	VarType Typer::TypeCall()
	{
		VarType node = std::make_shared<CallNode>();
		IncIndex();
		node->function = TypeExpr();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::With)
		{
			IncIndex();

			node->args.push_back(TypeExpr());

			while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Comma)
			{
				IncIndex();

				node->args.push_back(TypeExpr());
			}
		}
		return node;
	}

	VarType Typer::TypeImport()
	{
		VarType node = std::make_shared<ImportNode>();
		IncIndex();

		node->id = TypeIdentNode();

		return node;
	}

	VarType Typer::TypeFunction()
	{
		VarType node = std::make_shared<FuncNode>();
		node->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		IncIndex();

		node->id = TypeIdentNode();

		//m_CurrentScope->AddEntry(node->id->GetContents(), node->id->GetLineNumber()); 
		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope(node->id->GetContents(), node->id->GetLineNumber(), ScopeType::Function));
		//m_CurrentScope = m_ScopeStack.back();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Takes)
		{
			IncIndex();
			node->argTypes.push_back(TypeTypeNode());
			node->argNames.push_back(TypeIdentNode());
			//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());

			while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::ReturnType)
			{
				if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Comma)
				{
					RegisterSyntaxError(SyntaxErrorType::expComma, m_Tokens->at(m_TokenIndex));
				}
				else
					IncIndex();
				node->argTypes.push_back(TypeTypeNode());
				node->argNames.push_back(TypeIdentNode());
				//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());
			}
		}

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::ReturnType)
		{
			RegisterSyntaxError(SyntaxErrorType::expReturns, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->returnType = TypeTypeNode();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::Function)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			RegisterSyntaxError(SyntaxErrorType::expEndFunc, m_Tokens->at(m_TokenIndex));
			//m_CurrentScope->CloseScope(0);
		}
		
		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();

		return node;
	}
	
	VarType Typer::TypeProcedure()
	{
		VarType node = std::make_shared<ProcNode>();
		node->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		IncIndex();

		node->id = TypeIdentNode();

		//m_CurrentScope->AddEntry(node->id->GetContents(), node->id->GetLineNumber());
		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope(node->id->GetContents(), node->id->GetLineNumber(), ScopeType::Procedure));
		//m_CurrentScope = m_ScopeStack.back();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Takes)
			{
				RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
			}
			else
				IncIndex();
			node->argTypes.push_back(TypeTypeNode());
			node->argNames.push_back(TypeIdentNode());
			//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());

			while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
			{
				if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Comma)
				{
					RegisterSyntaxError(SyntaxErrorType::expComma, m_Tokens->at(m_TokenIndex));
				}
				else
					IncIndex();
				node->argTypes.push_back(TypeTypeNode());
				node->argNames.push_back(TypeIdentNode());
				//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());
			}
		}

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::Procedure)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//m_CurrentScope->CloseScope(0);
			RegisterSyntaxError(SyntaxErrorType::expEndProc, m_Tokens->at(m_TokenIndex));
		}

		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeStruct()
	{
		VarType node = std::make_shared<StructNode>();
		node->lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();
		IncIndex();

		node->id = TypeIdentNode();

		//m_CurrentScope->AddEntry(node->id->GetContents(), node->id->GetLineNumber());
		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope(node->id->GetContents(), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::If));
		//m_CurrentScope = m_ScopeStack.back();

		

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->types.push_back(TypeTypeNode());

		node->names.push_back(TypeIdentNode());

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
		{
			IncIndex();
		}

		while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::End)
		{
			if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Comma)
			{
				RegisterSyntaxError(SyntaxErrorType::expComma, m_Tokens->at(m_TokenIndex));
			}
			else
				IncIndex();

			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
			{
				IncIndex();
			}

			node->types.push_back(TypeTypeNode());

			node->names.push_back(TypeIdentNode());

			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
			{
				IncIndex();
			}
		}
		
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::Struct)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			RegisterSyntaxError(SyntaxErrorType::expEndStruct, m_Tokens->at(m_TokenIndex));
		}
		
		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeExpr()
	{
		VarType node = std::make_shared<Expr>();
		switch (m_Tokens->at(m_TokenIndex)->GetType())
		{
		case TokenType::Create:
			node->createNode = TypeCreate();
			node->exprType = 2; break;
		case TokenType::Cast:
			node->castNode = TypeCast();
			node->exprType = 3; break;
		case TokenType::Call:
			node->callNode = TypeCall();
			node->exprType = 4; break;
		case TokenType::AnonFunc:
			node->anonfNode = TypeAnonFunc();
			node->exprType = 5; break;
		case TokenType::AnonProc:
			node->anonpNode = TypeAnonProc();
			node->exprType = 6; break;
		default:
			node->binOpNode = TypeBinOpNode();
			node->exprType = 1; break;
		}
		return node;
	}

	VarType Typer::TypeList()
	{
		VarType node = std::make_shared<ListNode>();
		uint32_t lineNumber = m_Tokens->at(m_TokenIndex)->GetLineNumber();

		node->Items.push_back(TypeDictEntry());
		while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Comma)
		{
			IncIndex();
			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
				IncIndex();
			node->Items.push_back(TypeDictEntry());
			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Newline)
				IncIndex();
		}

		int dictPairs = -1;
		for (auto dictNode : node->Items)
		{
			if (dictNode->hasRight && dictPairs == 0)
				RegisterSyntaxError(SyntaxErrorType::inconsistentDict, lineNumber, 0, 10, TokenType::None);
			else if (dictNode->hasRight)
				dictPairs = 1;
			else if (dictPairs == 1)
				RegisterSyntaxError(SyntaxErrorType::inconsistentDict, lineNumber, 0, 10, TokenType::None);
			else
				dictPairs = 0;
		}

		return node;
	}

	VarType Typer::TypeDictEntry()
	{
		VarType node = std::make_shared<DictEntryNode>();
		node->left = TypeExpr();
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::DictEquator)
		{
			IncIndex();
			node->right = TypeExpr();
			node->hasRight = true;
		}
		else
			node->hasRight = false;

		return node;
	}

	/*
	VarType Typer::TypeBooleanExpr()
	{
		VarType node = std::make_shared<BoolExprNode>();
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Not)
		{
			node->exprType = 1;
			IncIndex();
			node->right = TypeBooleanExpr();
		}
		else
		{
			
			node->left = TypeAddExpr();
			TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
			if (type == TokenType::NotEqual || type == TokenType::Equal || type == TokenType::LessThan || type == TokenType::GreaterThan || type == TokenType::LessThanEqual || type == TokenType::GreaterThanEqual)
			{
				node->exprType = 2;
				node->opToken = m_Tokens->at(m_TokenIndex);
				IncIndex();
				node->right = TypeBooleanExpr();
			}
			else
			{
				node->exprType = 3;
			}
		}
		return node;
	}

	VarType Typer::TypeAddExpr()
	{
		VarType node = std::make_shared<AddExprNode>();
		node->left = TypeMulExpr();
		TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
		if (type == TokenType::Plus || type == TokenType::Minus)
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypeAddExpr();
		}
		return node;
	}

	VarType Typer::TypeMulExpr()
	{
		VarType node = std::make_shared<MulExprNode>();
		node->left = TypeDivModExpr();
		TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
		if (type == TokenType::Multiply || type == TokenType::Divide)
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypeMulExpr();
		}
		return node;
	}

	VarType Typer::TypeDivModExpr()
	{
		VarType node = std::make_shared<DivModExprNode>();
		node->left = TypePower();
		TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
		if (type == TokenType::Intdiv || type == TokenType::Modulo)
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypeDivModExpr();
		}
		return node;
	}

	VarType Typer::TypePower()
	{
		VarType node = std::make_shared<PowerNode>();
		node->left = TypeFactor();
		TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
		if (type == TokenType::Power)
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypePower();
		}
		return node;
	}
	*/

	VarType Typer::TypeBinOpNode()
	{
		VarType node = std::make_shared<BinOpNode>();
		node->left = TypeFactor();
		std::vector<TokenType> acceptedTypes = { TokenType::And, TokenType::Or, TokenType::NotEqual, TokenType::Equal, TokenType::GreaterThan,
			TokenType::GreaterThanEqual, TokenType::LessThan , TokenType::LessThanEqual, TokenType::Plus, TokenType::Minus, TokenType::Modulo,
			TokenType::Multiply, TokenType::Divide, TokenType::Intdiv, TokenType::Power, TokenType::BitwiseAnd, TokenType::BitwiseOr};

		if (itemInVect(acceptedTypes, m_Tokens->at(m_TokenIndex)->GetType()))
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypeBinOpNode();
		}
		return node;
	}

	VarType Typer::TypeFactor()
	{
		VarType node = std::make_shared<FactorNode>();
		std::vector<TokenType> acceptedTypes = { TokenType::Plus, TokenType::Minus, TokenType::Not };
		if (itemInVect(acceptedTypes, m_Tokens->at(m_TokenIndex)->GetType()))
		{
			node->opToken = m_Tokens->at(m_TokenIndex);
			IncIndex();
			node->right = TypeAtom();
			node->opTokenPresent = true;
		}
		else
		{
			node->right = TypeAtom();
			node->opTokenPresent = false;
		}
		return node;
	}

	VarType Typer::TypeAtom(bool quit)
	{
		VarType node = std::make_shared<AtomNode>();
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::LParen)
		{
			IncIndex();
			node->list = TypeList();
			if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::RParen)
			{
				RegisterSyntaxError(SyntaxErrorType::expRParen, m_Tokens->at(m_TokenIndex));
			}
			else
				IncIndex();
			node->type = 2;
		}
		else
		{
			std::vector<TokenType> acceptedAtomTokenTypes = { TokenType::String, TokenType::Raw, TokenType::Int, TokenType::Float, TokenType::Complex, TokenType::Identifier, TokenType::None };
			TokenType type = m_Tokens->at(m_TokenIndex)->GetType();
			if (!itemInVect(acceptedAtomTokenTypes, type) && !(((m_Tokens->at(m_TokenIndex)->GetContents()[0] & ~0x20) >= 'A') && ((m_Tokens->at(m_TokenIndex)->GetContents()[0] & ~0x20) <= 'Z')))
			{
				//If it doesn't start with a valid ident, then it isn't an identifier, and if it isn't any of the others, then it must be an error
				if (quit)
					return nullptr;
				RegisterSyntaxError(SyntaxErrorType::expAtom, m_Tokens->at(m_TokenIndex));
				node->type = 0;
				IncIndex();
			}
			else if (type == TokenType::Identifier || ((m_Tokens->at(m_TokenIndex)->GetContents()[0] & ~0x20) >= 'A' && (m_Tokens->at(m_TokenIndex)->GetContents()[0] & ~0x20) <= 'Z' && type != TokenType::String && type != TokenType::Raw))
			{
				//VarType ident = TypeIdentNode();
				node->ident = TypeIdentNode();
				node->type = 3;
				//@IMPORTANT How are global variables managed? Are all variables declared in the main scope treated as globals, or do
				//           they need to be specially declared and placed in a global scope? main scope = global scope, scope state -> global variables
				// it is NOT an error to reference a variable from a previous scope before defining it in the current scope - different to python behaviour 
				// w/ functions
				/*if (m_CurrentScope->FindIdent(ident) || m_MainScope->FindIdent(ident))
				{
					node->ident = ident;
					node->type = 3;
				}
				else
				{
					//RegisterMissingVariable(ident->GetLineNumber(), ident->GetColumnNumber());
					node->type = 0;
				}*/
			}
			else if (itemInVect(acceptedAtomTokenTypes, type))
			{
				node->token = m_Tokens->at(m_TokenIndex);
				node->type = 1;
				IncIndex();
			}
			else {
				SPLW_CRITICAL("We shouldn't be here!");
			}
		}

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::LSquareParen)
		{
			node->listAccess = TypeListAccess();
			node->listAccessPresent = true;
		}
		return node;
	}

	VarType Typer::TypeListAccess()
	{
		VarType node = std::make_shared<ListAccessNode>();
		IncIndex();

		node->indices.push_back(TypeList());
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::RSquareParen)
			IncIndex();
		else
			RegisterSyntaxError(SyntaxErrorType::expRSquareParen, m_Tokens->at(m_TokenIndex));

		while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::LSquareParen)
		{
			node->indices.push_back(TypeList());
			if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::RSquareParen)
				IncIndex();
			else
				RegisterSyntaxError(SyntaxErrorType::expRSquareParen, m_Tokens->at(m_TokenIndex));
		}

		return VarType();
	}

	VarType Typer::TypeCreate()
	{
		VarType node = std::make_shared<CreateNode>();
		IncIndex();
		node->createType = TypeTypeNode();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::With)
		{
			node->args.push_back(TypeExpr());
			while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Comma)
			{
				node->args.push_back(TypeExpr());
			}
		}
		return node;
	}

	VarType Typer::TypeCast()
	{
		VarType node = std::make_shared<CastNode>();
		IncIndex();
		node->castType = TypeTypeNode();
		node->list = TypeList();
		return node;
	}

	VarType Typer::TypeReturn()
	{
		VarType node = std::make_shared<ReturnNode>();
		IncIndex();
		node->list = TypeList();
		return node;
	}

	VarType Typer::TypeAnonFunc()
	{
		VarType node = std::make_shared<AnonfNode>();
		IncIndex();

		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("ANONF_line_" + std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::Anonf));
		//m_CurrentScope = m_ScopeStack.back();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Takes)
		{
			IncIndex();
			node->argTypes.push_back(TypeTypeNode());
			node->argNames.push_back(TypeIdentNode());
			//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());

			while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::ReturnType)
			{
				if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Comma)
				{
					RegisterSyntaxError(SyntaxErrorType::expComma, m_Tokens->at(m_TokenIndex));
				}
				else
					IncIndex();
				node->argTypes.push_back(TypeTypeNode());
				node->argNames.push_back(TypeIdentNode());
				//m_CurrentScope->AddEntry(node->argNames.back()->GetContents(), node->argNames.back()->GetLineNumber());
			}
		}

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::ReturnType)
		{
			RegisterSyntaxError(SyntaxErrorType::expReturns, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->returnType = TypeTypeNode();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::Function)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//m_CurrentScope->CloseScope(0);
			RegisterSyntaxError(SyntaxErrorType::expEndFunc, m_Tokens->at(m_TokenIndex));
		}

		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeAnonProc()
	{
		VarType node = std::make_shared<AnonpNode>();
		IncIndex();

		//m_ScopeStack.push_back(m_CurrentScope->AddSubScope("ANONP_line_" + std::to_string(m_Tokens->at(m_TokenIndex)->GetLineNumber()), m_Tokens->at(m_TokenIndex)->GetLineNumber(), ScopeType::Anonp));
		//m_CurrentScope = m_ScopeStack.back();

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Takes)
			{
				RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
			}
			else
				IncIndex();

			node->argTypes.push_back(TypeTypeNode());

			node->argNames.push_back(TypeIdentNode());

			while (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
			{
				if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::Comma)
				{
					RegisterSyntaxError(SyntaxErrorType::expComma, m_Tokens->at(m_TokenIndex));
				}
				else
					IncIndex();

				node->argTypes.push_back(TypeTypeNode());

				node->argNames.push_back(TypeIdentNode());

			}
		}

		if (m_Tokens->at(m_TokenIndex)->GetType() != TokenType::As)
		{
			RegisterSyntaxError(SyntaxErrorType::expAs, m_Tokens->at(m_TokenIndex));
		}
		else
			IncIndex();

		node->body = TypeStatements();

		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::End && m_Tokens->at((uint64_t)m_TokenIndex + (uint64_t)1)->GetType() == TokenType::Procedure)
		{
			//m_CurrentScope->CloseScope(m_Tokens->at(m_TokenIndex)->GetLineNumber());
			IncIndex(); IncIndex();
		}
		else
		{
			//m_CurrentScope->CloseScope(0);
			RegisterSyntaxError(SyntaxErrorType::expEndFunc, m_Tokens->at(m_TokenIndex));
		}

		//m_ScopeStack.pop_back();
		//m_CurrentScope = m_ScopeStack.back();
		return node;
	}

	VarType Typer::TypeTypeNode()
	{
		VarType node = std::make_shared<TypeNode>();
		if (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Type)
		{
			node->typeToken = m_Tokens->at(m_TokenIndex);
			node->type = 2;
			IncIndex();
		}
		else
		{
			node->ident = TypeIdentNode();
			node->type = 1;
		}
		return node;
	}

	VarType Typer::TypeIdentNode()
	{
		VarType node = std::make_shared<IdentNode>();
		
		char first = m_Tokens->at(m_TokenIndex)->GetContents()[0];
		if (!(first >= 0x41 && first <= 0x5a) && !(first >= 0x61 && first <= 0x7a) && !(m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Identifier))
		{
			RegisterSyntaxError(SyntaxErrorType::expIdent, m_Tokens->at(m_TokenIndex));
		}
		else
		{
			node->ids.push_back(m_Tokens->at(m_TokenIndex));
			IncIndex();
		}

		while (m_Tokens->at(m_TokenIndex)->GetType() == TokenType::VarAccessOp)
		{
			IncIndex();
			first = m_Tokens->at(m_TokenIndex)->GetContents()[0];
			if (!(first >= 0x41 && first <= 0x5a) && !(first >= 0x61 && first <= 0x7a) && !(m_Tokens->at(m_TokenIndex)->GetType() == TokenType::Identifier)) {
				RegisterSyntaxError(SyntaxErrorType::expIdent, m_Tokens->at(m_TokenIndex));
			} else {
				node->ids.push_back(m_Tokens->at(m_TokenIndex));
				IncIndex();
			}
			node->accessPresent = true;
		}
		return node;
	}
}
