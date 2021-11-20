#include "Generator.h"
#include "UtilFunctions.h"
#include <regex>

namespace Spliwaca
{
	template<typename T>
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

	std::shared_ptr<Generator> Generator::Create(std::shared_ptr<EntryPoint> entryPoint)
	{
		return std::make_shared<Generator>(entryPoint);
	}

	std::string Generator::GenerateCode(int& errorCode)
	{
		PROFILE_FUNCTION();
		m_Code = "import libsplw\nscope_vars = libsplw.default_scope.copy()\n\n";

		if (m_EntryPoint->require && m_EntryPoint->require->requireType->GetContents() == "transpiler_py")
		{
			m_ScopeImportConfigs.push_back(new ImportConfig(true, true, true, true));
		}
		else if (m_EntryPoint->requirePresent == false || m_EntryPoint->require->requireType->GetContents() == "transpiler")
		{
			m_ScopeImportConfigs.push_back(new ImportConfig(true, false, false, true));
		}
		else if (m_EntryPoint->require)
		{
			SPLW_CRITICAL("This generator is not compatible with the specified require statement, exiting.");
			errorCode = 1;
			return m_Code;
		}

		GenerateStatements(m_EntryPoint->statements);

		if (m_AbortPrint)
		{
			errorCode = 1;
		}

		return m_Code;
	}

	void Generator::GenerateStatements(std::shared_ptr<Statements> statements)
	{
		for (std::shared_ptr<Statement> s : statements->statements)
		{
			PROFILE_SCOPE("Statement loop - Generator::GenerateStatements(std::shared_ptr<Statements>)");
			ImportConfig* importConfig = getCurrentImportConfig();
			switch (s->statementType)
			{
			case 0:  GenerateIf(s->ifNode); break;
			case 1:  GenerateSet(s->setNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 2:  GenerateInput(s->inputNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 3:  GenerateOutput(s->outputNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 4:  GenerateInc(s->incNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 5:  GenerateDec(s->decNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 6:  GenerateFor(s->forNode); break;
			case 7:  GenerateWhile(s->whileNode); break;
			case 8:  GenerateQuit(s->quitNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 9:  GenerateCall(s->callNode, true); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 10: GenerateFunc(s->funcNode); break;
			case 11: GenerateProc(s->procNode); break;
			case 12: GenerateStruct(s->structNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 13: GenerateReturn(s->returnNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 14: GenerateImport(s->importNode); m_Code += "  # Source line " + std::to_string(s->lineNumber + 1) + "\n"; break;
			case 15: importConfig->allowImport = false; importConfig->allowPyImport = false; break;
			case 16: importConfig->allowInstall = false; break;
			case 17: importConfig->allowBare = false; break;
			}
			m_Code += "\n";
			m_InterpreterCall = false;
		}
	}

	void Generator::GenerateIf(std::shared_ptr<IfNode> node)
	{
		PROFILE_FUNCTION();
		for (uint32_t i = 0; i < node->conditions.size(); i++)
		{
			if (i == 0)
			{
				m_Code += m_Tabs + "if ";
			}
			else
			{
				m_Code += m_Tabs + "elif ";
			}

			GenerateList(node->conditions.at(i));
			m_Code += ":   # Source line " + std::to_string(node->lineNumbers[i] + 1) + "\n";

			m_Tabs += "    ";

			GenerateStatements(node->bodies.at(i));

			m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
		}

		if (node->elsePresent)
		{
			m_Code += m_Tabs + "else: # Source line " + std::to_string(node->lineNumbers[node->lineNumbers.size() - 1] + 1) + "\n";

			m_Tabs += "    ";

			GenerateStatements(node->bodies.back());

			m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
		}
	}

	void Generator::GenerateSet(std::shared_ptr<SetNode> node)
	{
		PROFILE_FUNCTION();
		if (node->id->accessPresent)
		{
			bool interpreter_var = false;
			std::string getattrTree = node->id->GenerateGetattrTree(getCurrentImportConfig(), interpreter_var, true);
			if (!interpreter_var)
			{
				m_Code += m_Tabs + "setattr(" + getattrTree + ", \"" + node->id->GetFinalId() + "\", "; GenerateList(node->list); m_Code += ")";
			}
			else
			{
				m_Code += m_Tabs + getattrTree + " = "; GenerateList(node->list);
			}
		}
		else
		{
			m_Code += m_Tabs + "scope_vars[\"" + node->id->GetContents() + "\"] = ";  GenerateList(node->list);
		}
	}

	void Generator::GenerateInput(std::shared_ptr<InputNode> node)
	{
		PROFILE_FUNCTION();
		if (node->id->accessPresent)
		{
			bool interpreter_var = false;
			std::string getattrTree = node->id->GenerateGetattrTree(getCurrentImportConfig(), interpreter_var, true);
			if (!interpreter_var)
			{
				m_Code += m_Tabs + "setattr(" + getattrTree + ", \"" + node->id->GetFinalId() + "\", libsplw.handle_input('";
			}
			else
			{
				m_Code += m_Tabs + getattrTree + " = libsplw.handle_input('";
			}
			if (node->signSpec)
				m_Code += node->signSpec->GetContents() + " ";
			GenerateType(node->type);
			m_Code += "')";
		}
		else
		{
			m_Code += m_Tabs + "scope_vars[\"" + node->id->GetContents() + "\"] = libsplw.handle_input('";
			if (node->signSpec)
				m_Code += node->signSpec->GetContents() + " ";
			GenerateType(node->type);
			m_Code += "')";
		}
	}

	void Generator::GenerateOutput(std::shared_ptr<OutputNode> node)
	{
		PROFILE_FUNCTION();
		m_Code += m_Tabs + "print(" + ParseRaw(node->raw) + ")";
	}

	void Generator::GenerateInc(std::shared_ptr<IncNode> node)
	{
		PROFILE_FUNCTION();
		if (node->id->accessPresent)
		{
			bool interpreter_var = false;
			std::string getAttrTree = node->id->GenerateGetattrTree(getCurrentImportConfig(), interpreter_var, true);
			if (interpreter_var)
			{
				m_Code += m_Tabs + getAttrTree + " += 1";
			}
			else
			{
				m_Code += m_Tabs + "setattr(" + getAttrTree + ", \"" + node->id->GetFinalId() + "\", " + node->id->GenerateGetattrTree(getCurrentImportConfig()) + " + 1)";
			}
		}
		else
		{
			m_Code += m_Tabs + "scope_vars[\"" + node->id->GetContents() + "\"] += 1";
		}
	}

	void Generator::GenerateDec(std::shared_ptr<DecNode> node)
	{
		PROFILE_FUNCTION();
		if (node->id->accessPresent)
		{
			bool interpreter_var = false;
			std::string getAttrTree = node->id->GenerateGetattrTree(getCurrentImportConfig(), interpreter_var, true);
			if (interpreter_var)
			{
				m_Code += m_Tabs + getAttrTree + " -= 1";
			}
			else
			{
				m_Code += m_Tabs + "setattr(" + getAttrTree + ", \"" + node->id->GetFinalId() + "\", " + node->id->GenerateGetattrTree(getCurrentImportConfig()) + " - 1)";
			}
		}
		else
		{
			m_Code += m_Tabs + "scope_vars[\"" + node->id->GetContents() + "\"] -= 1";
		}
	}

	void Generator::GenerateFor(std::shared_ptr<ForNode> node)
	{
		PROFILE_FUNCTION();
		if (node->id->accessPresent)
		{
			bool interpreter_var = false;
			std::string getAttrTree = node->id->GenerateGetattrTree(getCurrentImportConfig(), interpreter_var, true);
			if (interpreter_var)
			{
				m_Code += m_Tabs + "for " + getAttrTree + " in "; GenerateList(node->iterableExpr); m_Code += ": # Source line " + std::to_string(node->lineNumber + 1) + "\n";
			}
			else
			{
				std::string for_var = "__for_var_line_" + std::to_string(node->id->GetLineNumber()) + "_char_" + std::to_string(node->id->GetColumnNumber());
				m_Code += m_Tabs + "for " + for_var + " in "; GenerateList(node->iterableExpr); m_Code += ": # Source line " + std::to_string(node->lineNumber + 1) + "\n";
				m_Code += m_Tabs + "    setattr(" + getAttrTree + ", " + node->id->GetFinalId() + ", " + for_var + ")";
			}
		}
		else
		{
			std::string for_var = "__for_var_line_" + std::to_string(node->id->GetLineNumber()) + "_char_" + std::to_string(node->id->GetColumnNumber());
			m_Code += m_Tabs + "for " + for_var + " in "; GenerateList(node->iterableExpr); m_Code += ": # Source line " + std::to_string(node->lineNumber + 1) + "\n";
			m_Code += m_Tabs + "    scope_vars[\"" + node->id->GetContents() + "\"] = " + for_var + "\n";
		}
		m_Tabs += "    ";

		GenerateStatements(node->body);

		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
	}

	void Generator::GenerateWhile(std::shared_ptr<WhileNode> node)
	{
		PROFILE_FUNCTION();
		m_Code += m_Tabs + "while "; GenerateBinOp(node->condition); m_Code += ": # Source line " + std::to_string(node->lineNumber + 1) + "\n";
		m_Tabs += "    ";

		GenerateStatements(node->body);

		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
	}

	void Generator::GenerateQuit(std::shared_ptr<QuitNode> node)
	{
		PROFILE_FUNCTION();
		m_Code += m_Tabs + "exit(";
		if (node->returnVal)
			GenerateAtom(node->returnVal);
		m_Code += ")";
	}

	void Generator::GenerateCall(std::shared_ptr<CallNode> node, bool statement)
	{
		PROFILE_FUNCTION();
		if (statement)
			m_Code += m_Tabs;
		GenerateExpr(node->function);
		if (!m_InterpreterCall)
		{
			m_Code += "(scope_vars";

			if (node->args.size() != 0)
			{
				for (uint32_t i = 0; i < node->args.size(); i++)
				{
					m_Code += ", "; GenerateExpr(node->args.at(i));
				}
			}
		}
		else if (node->args.size() != 0)
		{
			m_Code += "("; GenerateExpr(node->args.at(0));

			for (uint32_t i = 1; i < node->args.size(); i++)
			{
				m_Code += ", "; GenerateExpr(node->args.at(i));
			}

		}
		else
		{
			m_Code += "(";
		}
		m_Code += ")";
	}

	void Generator::GenerateFunc(std::shared_ptr<FuncNode> node)
	{
		PROFILE_FUNCTION();
		std::string func_name = "__func_name_line_" + std::to_string(node->id->GetLineNumber()) + "_char_" + std::to_string(node->id->GetColumnNumber());
		m_Code += m_Tabs + "@libsplw.type_check()\ndef " + func_name + "(prev_scope_vars: dict";

		assert(node->argNames.size() == node->argTypes.size());

		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			m_Code += ", arg" + std::to_string(i) + ": ";
			GenerateType(node->argTypes.at(i));
			//node->argNames.at(i)->GetContents()
		}

		m_Code += ") -> ";
		GenerateType(node->returnType);
		m_Code += ": # Source line " + std::to_string(node->lineNumber + 1) + "\n";

		m_Tabs += "    ";
		ImportConfig* oldConfig = getCurrentImportConfig();
		ImportConfig* newConfig = new ImportConfig(oldConfig->allowImport, oldConfig->allowPyImport, oldConfig->allowInstall, oldConfig->allowBare);
		m_ScopeImportConfigs.push_back(newConfig);
		//SPLW_INFO("Current allow bare state: {0}", getCurrentImportConfig()->allowBare);

		m_Code += m_Tabs + "scope_vars = prev_scope_vars.copy()\n";
		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			if (node->argNames[i]->accessPresent)
			{
				SPLW_ERROR("Attempting to pass an argument name with an access present. This is not allowed. Line: {0}, arg: {1}", node->argNames[i]->GetLineNumber(), node->argNames[i]->GetContents());
				m_AbortPrint = true;
			}
			else
			{
				m_Code += m_Tabs + "scope_vars['" + node->argNames[i]->GetContents() + "'] = arg" + std::to_string(i) + "\n";
			}
		}

		GenerateStatements(node->body);
		m_Code += m_Tabs + "raise libsplw.FunctionEndError\n";
		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
		if (node->id->accessPresent)
		{
			SPLW_ERROR("Attempting to define a function with an access in the function name. This is not allowed. Line: {0}, Name: {1}", node->id->GetLineNumber(), node->id->GetContents());
			m_AbortPrint = true;
		}
		else
		{
			m_Code += m_Tabs + "scope_vars['" + node->id->GetContents() + "'] = " + func_name + "\n";
		}
		m_ScopeImportConfigs.pop_back();
	}

	void Generator::GenerateProc(std::shared_ptr<ProcNode> node)
	{
		PROFILE_FUNCTION();
		std::string func_name = "__func_name_line_" + std::to_string(node->id->GetLineNumber()) + "_char_" + std::to_string(node->id->GetColumnNumber());
		m_Code += m_Tabs + "@libsplw.type_check()\ndef " + func_name + "(prev_scope_vars: dict";

		assert(node->argNames.size() == node->argTypes.size());

		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			m_Code += ", arg" + std::to_string(i) + ": ";
			GenerateType(node->argTypes.at(i));
			//node->argNames.at(i)->GetContents()
		}

		m_Code += "): # Source line " + std::to_string(node->lineNumber + 1) + "\n";

		m_Tabs += "    ";
		ImportConfig* oldConfig = getCurrentImportConfig();
		ImportConfig* newConfig = new ImportConfig(oldConfig->allowImport, oldConfig->allowPyImport, oldConfig->allowInstall, oldConfig->allowBare);		m_ScopeImportConfigs.push_back(newConfig);

		m_Code += m_Tabs + "scope_vars = prev_scope_vars.copy()\n";
		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			if (node->argNames[i]->accessPresent)
			{
				SPLW_ERROR("Attempting to pass an argument name with an access present. This is not allowed. Line: {0}, arg: {1}", node->argNames[i]->GetLineNumber(), node->argNames[i]->GetContents());
				m_AbortPrint = true;
			}
			else
			{
				m_Code += m_Tabs + "scope_vars['" + node->argNames[i]->GetContents() + "'] = arg" + std::to_string(i) + "\n";
			}
		}

		GenerateStatements(node->body);
		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);
		if (node->id->accessPresent)
		{
			SPLW_ERROR("Attempting to define a procedure with an access in the procedure name. This is not allowed. Line: {0}, Name: {1}", node->id->GetLineNumber(), node->id->GetContents());
			m_AbortPrint = true;
		}
		else
		{
			m_Code += m_Tabs + "scope_vars['" + node->id->GetContents() + "'] = " + func_name + "\n";
		}
		m_ScopeImportConfigs.pop_back();
	}

	void Generator::GenerateStruct(std::shared_ptr<StructNode> node)
	{
		PROFILE_FUNCTION();
		//scope_vars['A'] = libsplw.make_struct_class(('x', 'y', 'z'), {'x':int, 'y':int, 'z':int}, 'A')
		if (node->id->accessPresent)
		{
			SPLW_ERROR("Attempting to define a struct with an access in the struct name. This is not allowed. Line: {0}, Name: {1}", node->id->GetLineNumber(), node->id->GetContents());
			m_AbortPrint = true;
		}
		if (node->names.size() != node->types.size())
		{
			SPLW_CRITICAL("Mismatched types and names in struct declaration. This should not be possible. Struct beginning line: {0}", node->id->GetLineNumber());
			m_AbortPrint = true;
		}
		m_Code += m_Tabs + "scope_vars['" + node->id->GetContents() + "'] = libsplw.make_struct_class((";

		if (node->names.size() != 0)
		{
			m_Code += "'" + node->names[0]->GetContents() + "'";
		}
		else
		{
			SPLW_WARN("Structure has no members. This should not be necessary. Structure beginning Line: {0}, Name: {1}", node->id->GetLineNumber(), node->id->GetContents());
		}
		for (uint32_t i = 1; i < node->names.size(); i++)
		{
			m_Code += ", '" + node->names[i]->GetContents() + "'";
		}

		m_Code += "), {";

		if (node->names.size() != 0)
		{
			m_Code += "'" + node->names.at(0)->GetContents() + "': ";
			GenerateType(node->types.at(0));
		}
		for (uint32_t i = 1; i < node->names.size(); i++)
		{
			m_Code += ", '" + node->names.at(i)->GetContents() + "': ";
			GenerateType(node->types.at(i));
		}
		m_Code += "}, '" + node->id->GetContents() + "')";
	}

	void Generator::GenerateReturn(std::shared_ptr<ReturnNode> node)
	{
		PROFILE_FUNCTION();
		m_Code += m_Tabs + "return "; GenerateList(node->list);
	}

	void Generator::GenerateImport(std::shared_ptr<ImportNode> node)
	{
		PROFILE_FUNCTION();
		m_Code += m_Tabs + "import " + node->id->GetContents();
	}

	void Generator::GenerateList(std::shared_ptr<ListNode> node, bool fromAtom)
	{
		if (node->Items.at(0)->hasRight)
		{
			m_Code += "{";
		}
		else if (node->Items.size() > 1)
		{
			m_Code += "[";
		}
		else
		{
			if (fromAtom)
			{
				m_Code += "(";
			}
			GenerateDictEntry(node->Items.at(0));
			if (fromAtom)
			{
				m_Code += ")";
			}
			return;
		}

		GenerateDictEntry(node->Items.at(0));
		for (uint32_t i = 1; i < node->Items.size(); i++)
		{
			m_Code += ", ";
			GenerateDictEntry(node->Items.at(i));
		}

		if (node->Items.at(0)->hasRight)
		{
			m_Code += "}";
		}
		else if (node->Items.size() > 1)
		{
			m_Code += "]";
		}
	}

	void Generator::GenerateDictEntry(std::shared_ptr<DictEntryNode> node)
	{
		GenerateExpr(node->left);
		if (node->hasRight)
		{
			m_Code.append(": ");
			GenerateExpr(node->right);
		}
	}

	void Generator::GenerateExpr(std::shared_ptr<Expr> node)
	{
		switch (node->exprType)
		{
		case 1: GenerateBinOp(node->binOpNode); break;
		case 2: GenerateCreate(node->createNode); break;
		case 3: GenerateCast(node->castNode); break;
		case 4: GenerateCall(node->callNode, false); break;
		case 5: GenerateAnonf(node->anonfNode); break;
		case 6: GenerateAnonp(node->anonpNode); break;
		}
	}

	void Generator::GenerateBinOp(std::shared_ptr<BinOpNode> node)
	{
		GenerateFactor(node->left);
		if (node->opToken)
		{
			std::string opTokenStr = "";
			//std::transform(opTokenStr.begin(), opTokenStr.end(), opTokenStr.begin(),
			//	[](unsigned char c) { return std::tolower(c); });
			switch (node->opToken->GetType())
			{
			case TokenType::Is:
				opTokenStr = "is"; break;
			case TokenType::Not:
				opTokenStr = "not"; break;
			case TokenType::And:
				opTokenStr = "and"; break;
			case TokenType::Or:
				opTokenStr = "or"; break;
			case TokenType::Equal:
				opTokenStr = "=="; break;
			case TokenType::NotEqual:
				opTokenStr = "!="; break;
			case TokenType::LessThan:
				opTokenStr = "<"; break;
			case TokenType::GreaterThan:
				opTokenStr = ">"; break;
			case TokenType::LessThanEqual:
				opTokenStr = "<="; break;
			case TokenType::GreaterThanEqual:
				opTokenStr = ">="; break;
			case TokenType::Multiply:
				opTokenStr = "*"; break;
			case TokenType::Divide:
				opTokenStr = "/"; break;
			case TokenType::Intdiv:
				opTokenStr = "//"; break;
			case TokenType::Plus:
				opTokenStr = "+"; break;
			case TokenType::Minus:
				opTokenStr = "-"; break;
			case TokenType::Modulo:
				opTokenStr = "%"; break;
			case TokenType::Xor:
				opTokenStr = "^"; break;
			case TokenType::BitwiseAnd:
				opTokenStr = "&"; break;
			case TokenType::BitwiseOr:
				opTokenStr = "|"; break;
			case TokenType::ShiftRight:
				opTokenStr = ">>"; break;
			case TokenType::ShiftLeft:
				opTokenStr = "<<"; break;
			default:
				SPLW_CRITICAL("Bug: Operator {0} not handled", node->opToken->GetContents());
			}
			m_Code += " " + opTokenStr + " ";
			GenerateBinOp(node->right);
		}
	}

	void Generator::GenerateFactor(std::shared_ptr<FactorNode> node)
	{
		if (node->opTokenPresent)
		{
			std::string opTokenStr = "";
			//std::transform(opTokenStr.begin(), opTokenStr.end(), opTokenStr.begin(),
			//	[](unsigned char c) { return std::tolower(c); });
			switch (node->opToken->GetType())
			{
			case TokenType::Is:
				opTokenStr = "is"; break;
			case TokenType::Not:
				opTokenStr = "not"; break;
			case TokenType::And:
				opTokenStr = "and"; break;
			case TokenType::Or:
				opTokenStr = "or"; break;
			case TokenType::Equal:
				opTokenStr = "=="; break;
			case TokenType::NotEqual:
				opTokenStr = "!="; break;
			case TokenType::LessThan:
				opTokenStr = "<"; break;
			case TokenType::GreaterThan:
				opTokenStr = ">"; break;
			case TokenType::LessThanEqual:
				opTokenStr = "<="; break;
			case TokenType::GreaterThanEqual:
				opTokenStr = ">="; break;
			case TokenType::Multiply:
				opTokenStr = "*"; break;
			case TokenType::Divide:
				opTokenStr = "/"; break;
			case TokenType::Intdiv:
				opTokenStr = "//"; break;
			case TokenType::Plus:
				opTokenStr = "+"; break;
			case TokenType::Minus:
				opTokenStr = "-"; break;
			case TokenType::Modulo:
				opTokenStr = "%"; break;
			case TokenType::Xor:
				opTokenStr = "^"; break;
			case TokenType::BitwiseAnd:
				opTokenStr = "&"; break;
			case TokenType::BitwiseOr:
				opTokenStr = "|"; break;
			case TokenType::ShiftRight:
				opTokenStr = ">>"; break;
			case TokenType::ShiftLeft:
				opTokenStr = "<<"; break;
			default:
				SPLW_CRITICAL("Bug: Operator {0} not handled", node->opToken->GetContents());
			}
			m_Code += " " + opTokenStr + " ";
		}
		GenerateAtom(node->right);
	}

	void Generator::GenerateAtom(std::shared_ptr<AtomNode> node)
	{
		switch (node->type)
		{
		case 1:
		{
			if (node->token->GetType() == TokenType::Raw)
				m_Code += ParseRaw(node->token);
			else if (node->token->GetType() == TokenType::String)
				m_Code += "\"" + node->token->GetContents() + "\"";
			else if (node->token->GetType() == TokenType::Complex)
				m_Code += ParseComplex(node->token);
			else if (node->token->GetType() == TokenType::True)
				m_Code += "True";
			else if (node->token->GetType() == TokenType::False)
				m_Code += "False";
			else
				m_Code += StripLeadingZeros(node->token->GetContents());
			break;
		}
		case 2: GenerateList(node->list, true); break;
		case 3: m_Code += node->ident->GenerateGetattrTree(getCurrentImportConfig(), m_InterpreterCall, false); break;
		}

		if (node->listAccessPresent)
		{
			for (std::shared_ptr<ListNode> n : node->listAccess->indices)
			{
				m_Code += "[";
				GenerateList(n);
				m_Code += "]";
			}
		}
	}

	void Generator::GenerateCreate(std::shared_ptr<CreateNode> node)
	{
		GenerateType(node->createType); m_Code += "(";

		if (node->args.size() > 0)
		{
			GenerateExpr(node->args.at(0));
			for (uint32_t i = 1; i < node->args.size(); i++)
			{
				m_Code += ", "; GenerateExpr(node->args.at(i));
			}
		}

		m_Code += ")";
	}

	void Generator::GenerateCast(std::shared_ptr<CastNode> node)
	{
		GenerateType(node->castType); m_Code += "("; GenerateList(node->list); m_Code += ")";
	}

	void Generator::GenerateAnonf(std::shared_ptr<AnonfNode> node)
	{
		uint32_t charIndex = (uint32_t)m_Code.size() - 1;
		std::string code = "";
		while (m_Code.at(charIndex) != '\n' && charIndex != -1)
		{
			code = m_Code.back() + code;
			m_Code.pop_back();
			charIndex--;
		}

		std::string anonf_name = "__anonf_line_" + std::to_string(node->argNames.at(0)->GetLineNumber()) + "_" + std::to_string(node->argNames[0]->GetColumnNumber());
		m_Code += m_Tabs + "@libsplw.type_check()\ndef " + anonf_name + "(prev_scope_vars: dict";

		assert(node->argNames.size() == node->argTypes.size());

		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			m_Code += ", arg" + std::to_string(i) + ": ";
			GenerateType(node->argTypes.at(i));
			//node->argNames.at(i)->GetContents()
		}

		m_Code += ") -> ";
		GenerateType(node->returnType);
		m_Code += ":\n";

		m_Tabs += "    ";
		ImportConfig* oldConfig = getCurrentImportConfig();
		ImportConfig* newConfig = new ImportConfig(oldConfig->allowImport, oldConfig->allowPyImport, oldConfig->allowInstall, oldConfig->allowBare);
		m_ScopeImportConfigs.push_back(newConfig);

		m_Code += m_Tabs + "scope_vars = prev_scope_vars.copy()\n";
		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			if (node->argNames[i]->accessPresent)
			{
				SPLW_ERROR("Attempting to pass an argument name with an access present. This is not allowed. Line: {0}, arg: {1}", node->argNames[i]->GetLineNumber(), node->argNames[i]->GetContents());
				m_AbortPrint = true;
			}
			else
			{
				m_Code += m_Tabs + "scope_vars['" + node->argNames[i]->GetContents() + "'] = arg" + std::to_string(i) + "\n";
			}
		}

		GenerateStatements(node->body);
		m_Code += m_Tabs + "raise libsplw.FunctionEndError\n";
		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);

		m_Code += code + anonf_name;
		m_ScopeImportConfigs.pop_back();
	}

	void Generator::GenerateAnonp(std::shared_ptr<AnonpNode> node)
	{
		uint32_t charIndex = (uint32_t)m_Code.size() - 1;
		std::string code = "";
		while (m_Code.at(charIndex) != '\n' && charIndex != -1)
		{
			code = m_Code.back() + code;
			m_Code.pop_back();
			charIndex--;
		}

		std::string anonp_name = "__anonp_line_" + std::to_string(node->argNames.at(0)->GetLineNumber()) + "_" + std::to_string(node->argNames[0]->GetColumnNumber());
		m_Code += m_Tabs + "@libsplw.type_check()\ndef " + anonp_name + "(prev_scope_vars: dict";

		assert(node->argNames.size() == node->argTypes.size());

		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			m_Code += ", arg" + std::to_string(i) + ": ";
			GenerateType(node->argTypes.at(i));
			//node->argNames.at(i)->GetContents()
		}

		m_Code += "):\n";

		m_Tabs += "    ";
		ImportConfig* oldConfig = getCurrentImportConfig();
		ImportConfig* newConfig = new ImportConfig(oldConfig->allowImport, oldConfig->allowPyImport, oldConfig->allowInstall, oldConfig->allowBare);
		m_ScopeImportConfigs.push_back(newConfig);

		m_Code += m_Tabs + "scope_vars = prev_scope_vars.copy()\n";
		for (uint32_t i = 0; i < node->argNames.size(); i++)
		{
			if (node->argNames[i]->accessPresent)
			{
				SPLW_ERROR("Attempting to pass an argument name with an access present. This is not allowed. Line: {0}, arg: {1}", node->argNames[i]->GetLineNumber(), node->argNames[i]->GetContents());
				m_AbortPrint = true;
			}
			else
			{
				m_Code += m_Tabs + "scope_vars['" + node->argNames[i]->GetContents() + "'] = arg" + std::to_string(i) + "\n";
			}
		}

		GenerateStatements(node->body);
		m_Tabs = m_Tabs.substr(0, m_Tabs.size() - 4);

		m_Code += code + anonp_name;
		m_ScopeImportConfigs.pop_back();
	}

	void Generator::GenerateType(std::shared_ptr<TypeNode> node)
	{
		if (node->type == 1)
		{
			m_Code += node->ident->GenerateGetattrTree(getCurrentImportConfig());
		}
		else
		{
			//m_Code += "__builtins__.";
			std::string typeTokenStr = node->typeToken->GetContents();
			std::transform(typeTokenStr.begin(), typeTokenStr.end(), typeTokenStr.begin(),
				[](unsigned char c) { return std::tolower(c); });
			if (typeTokenStr == "string")
				typeTokenStr = "str";
			else if (typeTokenStr == "real")
				typeTokenStr = "float";
			else if (typeTokenStr == "number")
				typeTokenStr = "float";
			else if (typeTokenStr == "integer")
				typeTokenStr = "int";
			else if (typeTokenStr == "dictionary")
				typeTokenStr = "dict";
			else if (typeTokenStr == "mapping")
				typeTokenStr = "dict";
			else if (typeTokenStr == "map")
				typeTokenStr = "dict";

			m_Code += typeTokenStr;
		}
	}

	/*void Generator::GenerateIdent(std::shared_ptr<IdentNode> node)
	{
		m_Code += node->GetContents();
	}*/

	bool validIdentifier(std::string id)
	{
		//If the last character is 'i' or a digit, then check regexes. Otherwise, just do the loop.
		PROFILE_FUNCTION();
		std::smatch m;
		if (id[id.size() - 1] == 'i' || (id[id.size() - 1] >= '0' && id[id.size() - 1] <= '9'))
		{
			if (std::regex_search(id, m, std::regex("(\\d+_*)+(\\.\\d+)?i?")) && m[0] == id) // Matches complex regex
			{
				return false;
			}
		}
		char invalidChars[] = { '~', '\\', ';', '#', '$', '@', '`', ',', '?', '.', '!', '%', '^', '<', '|', '\'', '&', ')', '*', '/', '+', '[', ']', '"', '=', '{', '}', ':', '>', '(', '-' };
		for (char c : id)
		{
			for (char d : invalidChars)
			{
				if (c == d)
				{
					return false;
				}
			}
		}
		return true;
	}

	std::string Generator::ParseRaw(std::shared_ptr<Token> token)
	{
		PROFILE_FUNCTION();
		std::string code = "fr\"";
		bool inIdent = false;
		IdentNode identNode = IdentNode();
		std::string ident = "";
		std::string contents = token->GetContents();
		for (char c : contents)
		{
			PROFILE_SCOPE("Token contents loop - Generator::ParseRaw(std::shared_ptr<Token>)");
			if (inIdent)
			{
				PROFILE_SCOPE("inIdent - Generator::ParseRaw(std::shared_ptr<Token>)");
				if (c == ' ')
				{
					if (validIdentifier(ident))
					{
						identNode.ids.push_back(std::make_shared<Token>(TokenType::Identifier, ident.c_str(), token->GetLineNumber(), token->GetCharacterNumber()));
						code += identNode.GenerateGetattrTree(getCurrentImportConfig());
					}
					else
					{
						SPLW_ERROR("Invalid identifier in RAW token, line {0}, char {1}", token->GetLineNumber(), token->GetCharacterNumber());
						m_AbortPrint = true;
					}
					code += "} ";
					inIdent = false;
					ident = "";
					identNode = IdentNode();
					continue;
				}
				else if (c == '.')
				{
					if (validIdentifier(ident))
					{
						identNode.ids.push_back(std::make_shared<Token>(TokenType::Identifier, ident.c_str(), token->GetLineNumber(), token->GetCharacterNumber()));
						identNode.accessPresent = true;
					}
					else
					{
						SPLW_ERROR("Invalid identifier in RAW token, line {0}, char {1}", token->GetLineNumber(), token->GetCharacterNumber());
						m_AbortPrint = true;
					}
					ident = "";
					continue;
				}
				else
				{
					ident += c;
					continue;
				}
			}
			else if (c != '$' && c != '"')//!charInStr("$\"", c))
				code += c;
			else if (c == '$')
			{
				inIdent = true;
				code += "{";
				continue;
			}
			else if (c == '"')
			{
				code += "\" + \"\\\"\" + fr\"";
				continue;
			}
			else
			{
				SPLW_ERROR("We shouldn't be here: line 935, Generator.cpp!");
			}
		}
		if (inIdent)
		{
			if (validIdentifier(ident))
			{
				identNode.ids.push_back(std::make_shared<Token>(TokenType::Identifier, ident.c_str(), token->GetLineNumber(), token->GetCharacterNumber()));
				code += identNode.GenerateGetattrTree(getCurrentImportConfig());
			}
			else
			{
				SPLW_ERROR("Invalid identifier in RAW token, line {0}, char {1}", token->GetLineNumber(), token->GetCharacterNumber());
				m_AbortPrint = true;
			}
			code += "}";
		}
		return code + "\"";
	}

	std::string Generator::ParseComplex(std::shared_ptr<Token> token)
	{
		std::string code = "";
		for (char c : token->GetContents())
		{
			if (c == 'i')
				code += 'j';
			else
				code += c;
		}
		return StripLeadingZeros(code);
	}

	std::string Generator::StripLeadingZeros(std::string token)
	{
		uint64_t EndOfLeadingZeros = 0;
		while (token[EndOfLeadingZeros] == '0' || token[EndOfLeadingZeros] == '_')
			EndOfLeadingZeros++;
		if (token.size() <= EndOfLeadingZeros || token[EndOfLeadingZeros] == '.')
			EndOfLeadingZeros--;
		return token.substr(EndOfLeadingZeros);
	}
}
