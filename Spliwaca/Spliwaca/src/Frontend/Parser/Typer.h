#pragma once
#include <memory>
#include <vector>
#include "Frontend/Lexer/Token.h"
#include "Nodes.h"
#include "Frontend/Scopes/Scope.h"

namespace Spliwaca
{
	class Typer
	{
	public:
		static std::shared_ptr<Typer> Create();
		~Typer() = default;

		std::shared_ptr<EntryPoint> TypeAST();
		
	private:
		//inline uint32_t IncIndex() { m_TokenIndex++; return m_TokenIndex; }

		//Typer() : { }

        

		//std::shared_ptr<Scope> m_MainScope;
		//std::vector<std::shared_ptr<Scope>> m_ScopeStack;
		//std::shared_ptr<Scope> m_CurrentScope;

	private:
		VarType TypeRequire();
		VarType TypeStatements();
		VarType TypeStatement();

		VarType TypeIf();
		VarType TypeSet();
		VarType TypeInput();
		VarType TypeOutput();
		VarType TypeIncrement();
		VarType TypeDecrement();
		VarType TypeFor();
		VarType TypeWhile();
		VarType TypeQuit();
		VarType TypeCall();
		VarType TypeImport();
		VarType TypeFunction();
		VarType TypeProcedure();
		VarType TypeStruct();

		VarType TypeExpr();

		VarType TypeList();
		VarType TypeDictEntry();
		/*VarType TypeBooleanExpr();
		VarType TypeAddExpr();
		VarType TypeMulExpr();
		VarType TypeDivModExpr();
		VarType TypePower();*/
		VarType TypeBinOpNode();
		VarType TypeFactor();
		VarType TypeAtom();
		VarType TypeListAccess();

		VarType TypeCreate();
		VarType TypeCast();
		VarType TypeReturn();
		VarType TypeAnonFunc();
		VarType TypeAnonProc();

		VarType TypeTypeNode();
		VarType TypeIdentNode();

	};
}
