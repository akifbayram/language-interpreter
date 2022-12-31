	#include "parse.h"

	map<string, bool> defVar;

	namespace Parser {
		bool pushed_back = false;
		LexItem	pushed_token;

		static LexItem GetNextToken(istream& in, int& line) {
			if ( pushed_back ) {
				pushed_back = false;
				return pushed_token;
			}
			return getNextToken(in, line);
		}

		static void PushBackToken(LexItem & t) {
			if ( pushed_back ) {
				abort();
			}
			pushed_back = true;
			pushed_token = t;	
		}
	}

	static int error_count = 0;

	int ErrCount() {
		return error_count;
	}

	void ParseError(int line, string msg) {
		++error_count;
		cout << line << ": " << msg << endl;
	}

	// Prog ::= PROGRAM IDENT StmtList END PROGRAM
	bool Prog(istream& in, int& line) {
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
			if ( (t = Parser::GetNextToken(in, line)) == PROGRAM )
				return true;
			else
				return true;
		} else if ( t == DONE ) {
			ParseError(line - 1, "Syntactic error in Program Body.");
			ParseError(line - 1, "Incorrect Program Body.");
			return false;
		} else {
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
		return true;
	} // End Prog 

	// StmtList ::= Stmt; { Stmt; } 
	bool StmtList(istream& in, int& line) {
		bool status = Stmt(in, line);
		if ( !status ) {
			ParseError(line, "Syntactic error in Program Body.");
			return false;
		}
		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t != SEMICOL ) {
			ParseError(line, "Missing semicolon at End Statement");
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
	} // End StmtList

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
	} // End Stmt

	// DeclStmt ::= ( INT | FLOAT | BOOL ) VarList
	bool DeclStmt(istream& in, int& line) {
		LexItem t = Parser::GetNextToken(in, line);

		//	if ( t == INT || t == FLOAT || t == BOOL ) {
		bool status = VarList(in, line);
		if ( !status ) 
			ParseError(line, "Incorrect variable in Declaration Statement");
		return status;
	} // End DeclStmt

	// VarList ::= Var { ,Var } 
	bool VarList(istream& in, int& line) {
		bool status = Var(in, line);
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
	} // End VarList

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
	} // End ControlStmt

	// PrintStmt:= PRINT (ExprList) 
	bool PrintStmt(istream& in, int& line) {
		LexItem t = Parser::GetNextToken(in, line);
		
		if ( t != LPAREN ) {
			ParseError(line, "Missing Left Parenthesis");
			return false;
		}
		
		bool status = ExprList(in, line);
		if ( !status ) {
			ParseError(line, "Missing expression list after Print");
			return false;
		}

		if ( (t = Parser::GetNextToken(in, line)) != RPAREN ) {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}
		return status;
	} // End PrintStmt

	// IfStmt ::= IF (Expr) THEN StmtList { ELSE StmtList } END IF
	bool IfStmt(istream& in, int& line) {		
		LexItem t;
		
		if ( (t = Parser::GetNextToken(in, line)) != LPAREN ) {
			ParseError(line, "Missing Left Parenthesis");
			return false;
		}
		bool status = Expr(in, line);
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
		if ( ( t = Parser::GetNextToken(in, line) ) == ELSE ) {
			status = StmtList(in, line);
			if ( !status ) {
				ParseError(line, "Missing statement list");
				return false;
			}
		} else if ( t == END ) {
			if ( (t = Parser::GetNextToken(in, line)) == IF ) 
				return true;
			else {
				ParseError(line, "Missing closing keywords of IF statement.");
				return false;
			}
		}
		return true;
	} // End IfStmt

	// AssignStmt ::= Var = Expr 
	bool AssignStmt(istream& in, int& line) {
		bool status = false;
		LexItem t = Parser::GetNextToken(in, line);

		if ( defVar.find(t.GetLexeme()) == defVar.end() ) {
			ParseError(line, "Undeclared Variable");
			return false;
		}
		Parser::PushBackToken(t);
		bool statusVar = Var(in, line);
		if ( statusVar ) {
			if ( (t = Parser::GetNextToken(in, line)) == ASSOP ) {
				status = Expr(in, line);
				if ( !status )
					ParseError(line, "Missing Expression in Assignment Statement");
				return status;
			} else if ( t.GetToken() == SEMICOL )
				return status;
			else {
				ParseError(line, "Missing Assignment Operator =");
				return false;
			}
		} else {
			ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
			return false;
		}
		return status;
	} // End AssignStmt 

	// Var ::= IDENT 
	bool Var(istream& in, int& line) {
		LexItem t = Parser::GetNextToken(in, line);
		if ( t == IDENT ) {
			if ( !(defVar.find(t.GetLexeme())->second) )
				defVar[t.GetLexeme()] = true;
			return true;
		}
		return false;
	} // End Var

	// ExprList:= Expr {,Expr}
	bool ExprList(istream& in, int& line) {
		bool status = Expr(in, line);
		if ( !status ) {
			ParseError(line, "Missing Expression in Expression List");
			return false;
		}
		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t == COMMA )
			status = ExprList(in, line);
		else {
			Parser::PushBackToken(t);
			return true;
		}
		return status;
	} // End ExprList

	// Expr ::= LogORExpr ::= LogANDExpr { || LogANDRxpr }
	bool Expr(istream& in, int& line) {
		bool status = LogANDExpr(in, line);
		if ( !status )
			return status;
		LexItem t;
		if ( (t = Parser::GetNextToken(in, line)) == OR ) {
			status = LogANDExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal Expression");
			return status;
		}
		Parser::PushBackToken(t);
		return status;
	} // End Expr

	// LogANDExpr ::= EqualExpr { && EqualExpr }
	bool LogANDExpr(istream& in, int& line) {
		LexItem t;
		bool status = EqualExpr(in, line);
		if ( !status )
			return status;

		if ( (t = Parser::GetNextToken(in, line)) == AND ) {
			status = EqualExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal AND Expression");		
			return status;
		}
		Parser::PushBackToken(t);
		return status;
	}
	
	// EqualExpr ::= RelExpr [== RelExpr ]
	bool EqualExpr(istream& in, int& line) {
		bool status = RelExpr(in, line);
		if ( !status )
			return status;
		
		LexItem t;
		if ( (t = Parser::GetNextToken(in, line)) == EQUAL ) {
			status = RelExpr(in, line);
			if ( !status ) 
				ParseError(line, "Illegal EqualExpr");
			return status;
		}
		Parser::PushBackToken(t);
		return status;
	}

	// RelExpr ::= AddExpr [ ( < | > ) AddExpr ]
	bool RelExpr(istream& in, int& line) {
		bool status = AddExpr(in, line);
		if ( !status ) {
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
		return status;
	}

	// AddExpr :: MultExpr { ( + | - ) MultExpr } 
	bool AddExpr(istream& in, int& line) {
		bool status = MultExpr(in, line);
		if ( !status )
			return status;

		LexItem t = Parser::GetNextToken(in, line);
		while ( t == PLUS || t == MINUS  ) {
			status = MultExpr(in, line);
			if ( !status )
				ParseError(line, "Missing operand after operator");
			return status;
		}
		Parser::PushBackToken(t);
		return status;
	}

	// MultExpr ::= UnaryExpr { ( * | / ) UnaryExpr } 
	bool MultExpr(istream& in, int& line) {
		bool status = UnaryExpr(in, line);
		if ( !status )
			return status;
		
		LexItem t = Parser::GetNextToken(in, line);
		while ( t == MULT || t == DIV  ) {
			status = UnaryExpr(in, line);
			if ( !status ) {
				ParseError(line, "Missing operand after operator");
				break;
			}
			t = Parser::GetNextToken(in, line);
		}
		Parser::PushBackToken(t);
		return status;
	}

	// UnaryExpr ::= ( - | + | ! ) PrimaryExpr | PrimaryExpr 
	bool UnaryExpr(istream& in, int& line) {		
		LexItem t = Parser::GetNextToken(in, line);
		if ( t == PLUS || t == MINUS  || t == NOT)
			t = Parser::GetNextToken(in, line);
		Parser::PushBackToken(t);
		bool status = PrimaryExpr(in, line, 20);
		return status;

	}

	// PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | ( Expr )
	bool PrimaryExpr(istream& in, int& line, int sign) {
		LexItem t = Parser::GetNextToken(in, line);
		string lexeme = t.GetLexeme();

		switch ( t.GetToken() ) {
			case IDENT: {
				if ( !(defVar.find(lexeme)->second) ) {
					ParseError(line, "Undefined Variable");
					return false;
				}
				return true;
			}
			case ICONST:
			case SCONST:
			case RCONST:
			case BCONST:
				return true;
			case LPAREN: {
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
			default: 
				break;
		}
		return false;
	}