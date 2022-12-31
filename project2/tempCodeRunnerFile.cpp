	/* Implementation of Recursive-Descent Parser
	* parse.cpp
	* Programming Assignment 2
	* Fall 2022
	*/

	#include "parse.h"

	map<string, bool> defVar;

	namespace Parser {
		bool pushed_back = false;
		LexItem	pushed_token;

		static LexItem GetNextToken(istream& in, int& line) {
			if( pushed_back ) {
				pushed_back = false;
				return pushed_token;
			}
			return getNextToken(in, line);
		}

		static void PushBackToken(LexItem & t) {
			if( pushed_back ) {
				abort();
			}
			pushed_back = true;
			pushed_token = t;	
		}

	}

	static int error_count = 0;

	int ErrCount()
	{
		return error_count;
	}

	void ParseError(int line, string msg)
	{
		++error_count;
		cout << line << ": " << msg << endl;
	}

	// Prog ::= PROGRAM IDENT StmtList END PROGRAM
	bool Prog(istream& in, int& line) {
		// cout << "IN PROG" << endl;
		LexItem t;
		
		if ( (t = Parser::GetNextToken(in, line)) != PROGRAM ) {
			ParseError(line, "Missing PROGRAM");
			return false;	
		}

		if ( (t = Parser::GetNextToken(in, line)) != IDENT ) {
			ParseError(line, "Missing Program Name.");
			return false;	
		}

		bool status = StmtList(in, line);
		if ( !status ) {
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
		if ( (t = Parser::GetNextToken(in, line)) == END ) {
			
			if ( (t = Parser::GetNextToken(in, line)) == PROGRAM ) {
				return true;
			}
			else {
				Parser::PushBackToken(t);
				ParseError(line, "Unrecognized Input Pattern");
				cout << "(" << t.GetLexeme() << ")" << endl;
				return false;
			}
		}
		else if ( t == DONE )
		{
			ParseError(line - 1, "Syntactic error in Program Body.");
			ParseError(line - 1, "Incorrect Program Body.");
			return false;
		}
		else 
		{
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
		

		// cout << "EXIT PROG " << t << endl;
		return true;
	}// End of Prog 




	// StmtList ::= Stmt; { Stmt; } 
	bool StmtList(istream& in, int& line) {
		bool status = false;

		status = Stmt(in, line);
		if ( !status ) {
			ParseError(line, "Syntactic error in Program Body.");
			return false;
		}
		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t != SEMICOL ) {
			ParseError(line, "Missing semicolon at end of Statement");
			return false;
		}
		while ( t == SEMICOL  ) {
			status = Stmt(in, line);
			if ( !status ) {
				ParseError(line, "Syntactic error in Program Body.");
				return false;
			}
			t = Parser::GetNextToken(in, line);
		}
		Parser::PushBackToken(t);
		return true;
	}//End of StmtList

	// Stmt ::= DeclStmt | ControlStmt
	bool Stmt(istream& in, int& line) {
		bool status = true;
		LexItem t = Parser::GetNextToken(in, line);

		switch ( t.GetToken() ) {
			case INT:
			case FLOAT:
			case BOOL:
				Parser::PushBackToken(t);
				status = DeclStmt(in, line);
				if ( !status )
					ParseError(line, "Incorrect Declaration Statement.");
				return status;
			case IF:
			case IDENT:
			case PRINT:
				Parser::PushBackToken(t);
				status = ControlStmt(in, line);
				if ( !status )
					ParseError(line, "Incorrect control Statement.");
				return status;
			default:
				Parser::PushBackToken(t);
				return true;
		}

		return true;
	}//End of Stmt


	// DeclStmt ::= ( INT | FLOAT | BOOL ) VarList
	bool DeclStmt(istream& in, int& line) {
		LexItem t = Parser::GetNextToken(in, line);
		bool status = false;

		//	if ( t == INT || t == FLOAT || t == BOOL ) {
		status = VarList(in, line);
		if ( !status ) 
			ParseError(line, "Incorrect variable in Declaration Statement");
		return status;
	}//End of DeclStmt

	// VarList ::= Var { ,Var } 
	bool VarList(istream& in, int& line) {
		bool status = false;

		status = Var(in, line);
		if ( !status ) {
			ParseError(line, "Missing Variable");
			return false;
		}
		
		LexItem t;
		while ( (t = Parser::GetNextToken(in, line)) == COMMA ) {
			t = Parser::GetNextToken(in, line);

			if ( defVar.find(t.GetLexeme()) != defVar.end() ) {
				ParseError(line, "Variable Redefinition");
				return false;
			}
			Parser::PushBackToken(t);
			status = Var(in, line);
			if ( !status ) {
				ParseError(line, "Missing Variable");
				return false;
			}
		}
		Parser::PushBackToken(t);
		return status;
	} //End of VarList


	// ControlStmt ::= AssigStmt | IfStmt | PrintStmt 
	bool ControlStmt(istream& in, int& line) {
		bool status = false;
		
		LexItem t = Parser::GetNextToken(in, line);
		switch( t.GetToken() ) {
			case PRINT:
				status = PrintStmt(in, line);
				break;
			case IF:
				status = IfStmt(in, line);
				break;
			case IDENT:
				Parser::PushBackToken(t);
				status = AssignStmt(in, line);
				break;
			default:
				break;
		}
		return status;
	} //End of ControlStmt



	//PrintStmt:= PRINT (ExprList) 
	bool PrintStmt(istream& in, int& line) {
		LexItem t = Parser::GetNextToken(in, line);
		
		if( t != LPAREN ) {
			ParseError(line, "Missing Left Parenthesis");
			return false;
		}
		
		bool status = ExprList(in, line);
		if( !status ) {
			ParseError(line, "Missing expression list after Print");
			return false;
		}

		if( (t = Parser::GetNextToken(in, line)) != RPAREN ) {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}
		return status;
	}//End of PrintStmt


	// IfStmt ::= IF (Expr) THEN StmtList { ELSE StmtList } END IF
	bool IfStmt(istream& in, int& line) {		
		bool status = false;
		LexItem t;
		
		if ( (t = Parser::GetNextToken(in, line)) != LPAREN ) {
			ParseError(line, "Missing Left Parenthesis");
			return false;
		}
		
		status = Expr(in, line);
		if ( !status ) {
			ParseError(line, "Missing if statement expression");
			return false;
		}

		if ( (t = Parser::GetNextToken(in, line)) != RPAREN ) {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}

		if ( (t = Parser::GetNextToken(in, line)) != THEN ) {
			ParseError(line, "Missing THEN");
			return false;
		}

		status = StmtList(in, line);
		if ( !status ) {
			ParseError(line, "Missing Statement for If-Stmt Then-Part");
			return false;
		}

		if ( (t = Parser::GetNextToken(in, line)) == ELSE ) {
			status = StmtList(in, line);
			if ( !status ) {
				ParseError(line, "Missing statement list");
				return false;
			}
		}
		else if ( t == END ) {
			if ( t == IF ) 
				return true;
			else {
				ParseError(line, "Missing closing keywords of IF statement.");
				return false;
			}
		}
		else {
			Parser::PushBackToken(t);
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
		}
		
		return true;
	}//End of IfStmt




	// AssignStmt ::= Var = Expr 
	bool AssignStmt(istream& in, int& line) {
		// cout << "  IN ASSIGNSTMT" << endl;
		bool status = false, statusVar = false;
		LexItem t = Parser::GetNextToken(in, line);


		if ( defVar.find(t.GetLexeme()) == defVar.end() ) {
			ParseError(line, "Undeclared Variable");
			return false;
		}
		Parser::PushBackToken(t);
		statusVar = Var(in, line);
		

		if ( statusVar ) {
			if ( (t = Parser::GetNextToken(in, line)) == ASSOP ) {
				// cout << "  ASSOP" << endl;
				status = Expr(in, line);
				if ( !status ) {
					ParseError(line, "Missing Expression in Assignment Statement");
				}
				// cout << "  EXIT ASSIGNSTMT 1" << endl;
				return status;
			}
			else if ( t.GetToken() == SEMICOL ) {
				// cout << "  EXIT ASSIGNSTMT 2" << endl;
				return status;
			}
			else {
				ParseError(line, "Missing Assignment Operator =");
				return false;
			}
		}
		else {
			ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
			return false;
		}
		
		// cout << "  EXIT ASSIGNSTMT" <<  t << endl;
		return status;
	}//End of AssignStmt 



	// Var ::= IDENT 
	bool Var(istream& in, int& line) {
		// cout << "   IN VAR" << endl;

		LexItem t = Parser::GetNextToken(in, line);
		// cout << "     " << t << endl;
		// cout << t.GetLexeme() << endl;
		if ( t == IDENT ) {
			if ( !(defVar.find(t.GetLexeme())->second) ) {
				defVar[t.GetLexeme()] = true;
			}
			return true;
		}
		// cout << "   EXIT VAR 2 " << endl;
		return false;
	} //End of Var


	//ExprList:= Expr {,Expr}
	bool ExprList(istream& in, int& line) {
		// cout << "IN EXPRLIST" << endl;
		bool status = false;

		status = Expr(in, line);
		if ( !status ) {
			ParseError(line, "Missing Expression in Expression List");
			return false;
		}
		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t == COMMA ) {
			status = ExprList(in, line);
		}
		else if ( t.GetToken() == ERR ) {
			ParseError(line, "Unrecognized Input Pattern");
			// cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			Parser::PushBackToken(t);
			return true;
		}
		return status;
	}//End of ExprList





	// Expr ::= LogORExpr ::= LogANDExpr { || LogANDRxpr }
	bool Expr(istream& in, int& line) {
		// cout << "    IN Expr" << endl;
		bool status = false;
		LexItem t;

		status = LogANDExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");
			return status;
		}
		if ( (t = Parser::GetNextToken(in, line)) == OR ) {
			// cout << "     " << t << endl;
			status = LogANDExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal Expression");
			return status;
		}
		// cout << "    EXIT Expr" << endl;
		Parser::PushBackToken(t);
		return status;
	}//End of Expr


	
	bool LogANDExpr(istream& in, int& line) {
		// cout << "    IN LogANDExpr" << endl;
		bool status = false;
		LexItem t;

		status = EqualExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");
			return status;
		}

		if ( (t = Parser::GetNextToken(in, line)) == AND ) {
			status = EqualExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal AND Expression");		
			return status;
		}
		Parser::PushBackToken(t);
		// cout << "     EXIT LogANDExpr " << t << endl;
		return status;
	}


	bool EqualExpr(istream& in, int& line) {
		// cout << "    IN EqualExpr" << endl;
		bool status = false;

		status = RelExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");		
			// cout << "     EXIT EqualExpr 1 " << endl;
			return status;
		}
		
		LexItem t;
		if ( (t = Parser::GetNextToken(in, line)) == EQUAL ) {
			status = RelExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal Equality Expression");
			return status;
		}
		Parser::PushBackToken(t);
		// cout << "     EXIT EqualExpr " << t << endl;
		return status;
	}


	bool RelExpr(istream& in, int& line) {
		// cout << "    IN RelExpr" << endl;
		bool status = false;

		status = AddExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");
			// cout << "     EXIT RelExpr 1 " << endl;
			return status;
		}

		LexItem t = Parser::GetNextToken(in, line);
		if ( t == LTHAN || t == GTHAN  ) {
			status = AddExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal RelExpr");
			return status;
		}
		Parser::PushBackToken(t);
		// cout << "     EXIT RelExpr " << t << endl;
		return status;
	}

	// AddExpr :: MultExpr { ( + | - ) MultExpr } 
	bool AddExpr(istream& in, int& line) {
		// cout << "    IN AddExpr" << endl;
		bool status = false;

		status = MultExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");
			// cout << "     EXIT AddExpr 1 " << endl;
			return status;
		}

		LexItem t = Parser::GetNextToken(in, line);
		while ( t == PLUS || t == MINUS  ) {
			status = MultExpr(in, line);
			if ( !status ) {
				ParseError(line, "Missing operand after operator");
			}
			return status;
		}
		Parser::PushBackToken(t);
		// cout << "     EXIT AddExpr " << t << endl;
		return status;
	}

	// MultExpr ::= UnaryExpr { ( * | / ) UnaryExpr } 
	bool MultExpr(istream& in, int& line) {
		// cout << "    IN MultExpr" << endl;
		bool status = false;

		status = UnaryExpr(in, line);
		if ( !status ) {
			//ParseError(line, "Error LogANDExpr");
			// cout << "     EXIT MultExpr 1 " << endl;
			return status;
		}
		
		LexItem t = Parser::GetNextToken(in, line);
		while ( t == MULT || t == DIV  ) {
			status = UnaryExpr(in, line);
			if ( !status ) {
				ParseError(line, "Missing operand after operator");
				// Parser::PushBackToken(t);
				break;
				return false;
			}
			t = Parser::GetNextToken(in, line);
		}
		Parser::PushBackToken(t);
		// cout << "     EXIT MultExpr " << t << endl;
		return status;
	}


	// The unary sign operators (+ or -) are applied upon unary numeric operands (i.e., INT, FLOAT). 
	// While the unary NOT operator is applied upon a Boolean operand (i.e., BOOL). 
	// UnaryExpr ::= ( - | + | ! ) PrimaryExpr | PrimaryExpr 
	bool UnaryExpr(istream& in, int& line) {
		// cout << "    IN UnaryExpr" << endl;
		bool status = false;
		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t == PLUS || t == MINUS  || t == NOT) {
			t = Parser::GetNextToken(in, line);
		}
		Parser::PushBackToken(t);
		status = PrimaryExpr(in, line, 20);
		if ( !status ) {
			//Parser::PushBackToken(t);
			//ParseError(line, "Error Unary");
		}
		// cout << "     EXIT UnaryExpr " << t << endl;
		return status;

	}


	// PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | ( Expr )
	bool PrimaryExpr(istream& in, int& line, int sign) {
		LexItem t = Parser::GetNextToken(in, line);
		string lexeme = t.GetLexeme();	
		// cout << "    IN PRIMARYEXPR " << t << endl;

		switch ( t.GetToken() ) {
			case IDENT: 
			{
				// cout << "  IDENT " << t << endl;
				// int val;
				if ( !(defVar.find(lexeme)->second) ) {
					ParseError(line, "Undefined Variable");
					return false;
				}
				return true;
			}

			case ICONST:
			{
				// cout << "  ICONST " << t << endl;
				// convert string of digits to integer number
				// create a val object for iconst and enter into sysmol table
				return true;
			}

			case SCONST:
			{
				// cout << "  SCONST " << t << endl;
				// create a Val object for SCONST and enter into sysmol table
				return true;
			}

			case RCONST:
			{
				// cout << "  RCONST " << t << endl;
			// convert string of digits to real number
			// create a val object for iconst and enter into sysmol table			return true;
				return true;
			}
				
			case BCONST:
			{
				// cout << "  BCONST " << t << endl;
				// 
				return true;
			}

			case LPAREN:
			{
				bool status = Expr(in, line);
				if ( !status ) {
					ParseError(line, "Missing expression after (");
					return false;
				}
				if ( (t = Parser::GetNextToken(in, line)) == RPAREN )
					return true;
				ParseError(line, "Missing ) after expression");
				return false;
			}

			case ERR:
			{
				ParseError(line, "Unrecognized Input Pattern");
				// cout << "(" << t.GetLexeme() << ")" << endl;
				return false;
			}

			default: 
			{
				break;
			}
		}
		return false;
	}