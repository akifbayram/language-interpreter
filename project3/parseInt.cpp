/* Implementation of Recursive-Descent Parser
 * parseInt.cpp
 * Programming Assignment 3
 * Fall 2022
*/

#include "parseInt.h"

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults;
queue <Value> * ValQue; 

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

bool IdentList(istream& in, int& line);

// Prog ::= PROGRAM IDENT StmtList END PROGRAM
bool Prog(istream& in, int& line) {
	bool f1;
	LexItem tok = Parser::GetNextToken(in, line);
		
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			
			f1 = StmtList(in, line); 
			
			if(f1) {
				tok = Parser::GetNextToken(in, line);
				if( tok == END){
					tok = Parser::GetNextToken(in, line);
					if(tok != PROGRAM){
						ParseError(line, "Missing PROGRAM Keyword.");
						return false;
					}
					return true;
				}
				else
				{
					ParseError(line, "Missing END of Program.");
					return false;
				}
				
			}
			else
			{
				ParseError(line, "Incorrect Program Body.");
				return false;
			}
		}
		else
		{
			ParseError(line, "Missing Program Name.");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if(tok.GetToken() == DONE && tok.GetLinenum() <= 1){
		ParseError(line, "Empty File");
		return true;
	}
	ParseError(line, "Missing PROGRAM.");
	return false;
}//End of Prog

// StmtList ::= Stmt; { Stmt; }
bool StmtList(istream& in, int& line){
	bool status;
		
	LexItem tok;
	
	status = Stmt(in, line);
		
	while(status)
	{
		tok = Parser::GetNextToken(in, line);
		if(tok != SEMICOL)
		{
			ParseError(line, "Missing semicolon at end of Statement.");
			return false;
		}
			
		status = Stmt(in, line);
	}
			
	tok = Parser::GetNextToken(in, line);
	if(tok == END )
	{
		Parser::PushBackToken(tok);
		return true;
	}
	else if(tok == ELSE)
	{
		Parser::PushBackToken(tok);
		return true;
	}
	else 
	{
		ParseError(line, "Syntactic error in Program Body.");
		return false;
	}
	
}//End of StmtList function

// Stmt ::= DeclStmt | ControlStmt 
bool Stmt(istream& in, int& line){
	bool status=false;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {
	case INT: case FLOAT: case BOOL:
		Parser::PushBackToken(t);
		status = DeclStmt(in, line);
		if(!status)
		{
			ParseError(line, "Incorrect Declaration Statement.");
			return status;
		}
		break;
	case IF: case PRINT: case IDENT: 
		Parser::PushBackToken(t);
		status = ControlStmt(in, line);
		
		if(!status)
		{
			ParseError(line, "Incorrect control Statement.");
			return status;
		}
		break;
	default:
		Parser::PushBackToken(t);
	}
	return status;
}//End of Stmt function

// DeclStmt ::= ( INT | FLOAT | BOOL ) VarList 
bool DeclStmt(istream& in, int& line)
{
	LexItem t, type;
	bool status;
	
	t = Parser::GetNextToken(in, line);

	if(t == INT || t == FLOAT || t == BOOL)
	{
		status = VarList(in, line, type);
		
		if (!status)
		{
			ParseError(line, "Incorrect variable in Declaration Statement.");
			return status;
		}
	}
	else
	{
		ParseError(line, "Incorrect Declaration Type.");
		return false;
	}
	
	return true;
		
}//End of DeclStmt
 
// VarList ::= Var { ,Var } 
bool VarList(istream& in, int& line, LexItem& type) {
	bool status = false;
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	status = Var(in, line, tok);
	if (!status) {
    	ParseError(line, "Incorrect variable in Declaration Statement.");
		return false;
	}

	if(tok == IDENT)
	{
		//set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
		}	
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
		
	}
	else
	{
		Parser::PushBackToken(tok);
		return true;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		status = VarList(in, line, type);
	}
	else {
		Parser::PushBackToken(tok);
		return true;
	}
	return status;	
}//End of VarList


// ControlStmt ::= AssigStmt | IfStmt | PrintStmt  
bool ControlStmt(istream& in, int& line) {
	bool status;
	
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
		Parser::PushBackToken(t);
		return false;
	}

	return status;
}//End of ControlStmt


// PrintStmt ::= PRINT (ExprList)
bool PrintStmt(istream& in, int& line) {
	LexItem t;
	Value val1;
	ValQue = new queue<Value>;
	bool ex = ExprList(in, line);
	
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
		
	
	if( !ex ) {
		ParseError(line, "Missing expression list after Print");
		while(!(*ValQue).empty())
		{
			ValQue->pop();		
		}
		delete ValQue;
		return false;
	}
	
	//Evaluate: print out the list of expressions' values
	while (!(*ValQue).empty())
	{
		Value nextVal = (*ValQue).front();
		cout << nextVal;
		ValQue->pop();
	}
	cout << endl;
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	return true;
}//End of PrintStmt


// IfStmt ::= IF (Expr) THEN StmtList [ ELSE StmtList ] END IF 

bool IfStmt(istream& in, int& line) {
	bool ex = false, status ; 
	LexItem t;
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	Value val1;
	ex = Expr(in, line, val1);
	if( !ex || val1.IsErr() ) {
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != THEN)
	{
		ParseError(line, "If Statement Syntax Error");
		return false;
	}

	if (val1.GetBool() ) {
		status = StmtList(in, line);
		if(!status)
		{
			ParseError(line, "Missing Statement for If-Stmt Then-Part");
			return false;
		}
	}
	else {
        t = Parser::GetNextToken(in, line);
        while(t != SEMICOL && t != ELSE){
            t = Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(t);
    }	

	t = Parser::GetNextToken(in, line);
	if( t == ELSE ) {
		status = StmtList(in, line);
		if(!status)
		{
			ParseError(line, "Missing Statement for If-Stmt Else-Part");
			return false;
		}
		else
		{
			t = Parser::GetNextToken(in, line);
		}
		
	}
	
	if( t == END){
		t = Parser::GetNextToken(in, line);
		if(t != IF){
			ParseError(line, "Missing closing keywords of IF statement.");
			return false;
		}
		return true;
	}
	else
	{
		ParseError(line, "Missing END of IF.");
		return false;
	}
	
}//End of IfStmt function

//AssignStmt ::= Var = Expr 
bool AssignStmt(istream& in, int& line) {

	bool varstatus = false, status = false;
	LexItem t, lexReturn;
	Value retVal;
	
	varstatus = Var( in, line, lexReturn);
	
    if (varstatus){
        t = Parser::GetNextToken(in, line);

        if (t == ASSOP){
            Value ret;
            status = Expr(in, line, ret);
            //cout << ret << endl;
            if(!status) {
                ParseError(line, "Missing Expression in Assignment Statement");
                return status;
            }
            string retLex = lexReturn.GetLexeme();
            //INTEGER TYPE RET :: INTEGER = 2
            if(SymTable[retLex] == 2){
                if(ret.GetType() == VREAL ){
                    float z = ret.GetReal();
                    ret.SetType(VINT);
                    ret.SetInt((int)z);
                    //ret.SetInt((int)ret.GetReal());
                }else if(ret.GetType() == VINT){
                    //redundant
                    ret.SetInt(ret.GetInt());
                }
            }
            //REAL TYPE RET :: REAL = 6
            if(SymTable[retLex] == 6){
                if(ret.GetType() == VREAL){
                    ret.SetReal((float)ret.GetReal());
                }else if(ret.GetType() == VINT){
                    int z = ret.GetInt();
                    ret.SetType(VREAL);
                    ret.SetReal((float)z);
                }
            }
            //cout << ret << endl;
           // cout << lexReturn.GetLexeme() << endl;
            TempsResults[lexReturn.GetLexeme()] = ret;
            //cout << SymTable[lexReturn.GetLexeme()] << endl;

            if(ret.GetType() == VSTRING && SymTable[retLex] != SCONST){
                ParseError(line, "String Variable Must be Assigned a String");
                return false;
            }else if((ret.GetType() == VREAL || ret.GetType() == VINT) && (SymTable[retLex] != RCONST && SymTable[retLex] != ICONST)){
                ParseError(line, "Real or Integer Variable Must be Assigned a Real or Integer");
                return false;
            }

        }
        else if(t.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        else {
            ParseError(line, "Missing Assignment Operator");
            return false;
        }
    }
    else {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    return status;
}//End of AssignStmt


// Var ::= IDENT 
bool Var(istream& in, int& line, LexItem & idtok)
{
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		//cout<< "in assigstmt var: " << identstr << endl;
		if (!(defVar.find(identstr)->second))
		{
			ParseError(line, "Undeclared Variable");
			return false;
		}	
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	return false;
}//End of Var

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	Value val1;
	status = Expr(in, line, val1);
	ValQue->push(val1);
	if(!status || val1.IsErr() ){
		ParseError(line, "Missing Expression");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		
		status = ExprList(in, line);
		
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}//End of ExprList

//Expr ::= LogORExpr ::= LogANDExpr { || LogANDRxpr }
bool Expr(istream& in, int& line, Value & retVal) {
	
	LexItem tok;
	bool t1 = LogANDExpr(in, line, retVal);
		
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == OR ) 
	{
		t1 = LogANDExpr(in, line, retVal);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of Expr/LogORExpr


//LogANDExpr ::= EqualExpr { && EqualExpr }
bool LogANDExpr(istream& in, int& line, Value & retVal) {
	LexItem tok;
	Value val1, val2;
	bool t1 = EqualExpr(in, line, val1);
		
	if( !t1 ) {
		return false;
	}
	retVal = val1;
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == AND ) 
	{
		t1 = EqualExpr(in, line, val2);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}
		//evaluate expression for Logical AND
		retVal = retVal && val2;
		if(retVal.IsErr())
		{
			ParseError(line, "Illegal AND operation.");	
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
	}
	
	Parser::PushBackToken(tok);
	return true;
}//End of LogANDExpr

// EqualExpr ::= RelExpr  [== RelExpr ]
bool EqualExpr(istream& in, int& line, Value & retVal) {
	LexItem tok;
	Value val1, val2;
	bool t1 = RelExpr(in, line, val1);
		
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if ( tok == EQUAL ) 
	{
		t1 = RelExpr(in, line, val1);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok == EQUAL)
		{
			ParseError(line, "Illegal Equality Expression.");
			return false;
		}
		else if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of EqualExpr


// RelExpr ::= AddExpr  [ ( < | > ) AddExpr ] 
bool RelExpr(istream& in, int& line, Value & retVal) {
	LexItem tok;
	Value val1, val2;
	bool t1 = AddExpr(in, line, val1);
		
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if ( tok == LTHAN || tok == GTHAN) 
	{
		t1 = AddExpr(in, line, val1);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		
		if(tok == LTHAN || tok == GTHAN)
		{
			ParseError(line, "Illegal Relational Expression.");
			return false;
		}
		else if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of RelExpr

// AddExpr :: MultExpr { ( + | - ) MultExpr } //
bool AddExpr(istream& in, int& line, Value & retVal) {
	Value val1, val2;
	bool t1 = MultExpr(in, line, val1);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	retVal = val1;
    if(retVal.IsErr()){
        ParseError(line, "Undefined Variable ADD");
        return false;
    }
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == PLUS || tok == MINUS ) 
	{
		t1 = MultExpr(in, line, val2);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}

        if(tok == PLUS)
        {
            retVal = val1 + val2;
            if(retVal.IsErr())
            {
                ParseError(line, "Illegal addition operation.");
                return false;
            }
        }
        else if(tok == MINUS)
        {
            retVal = val1 - val2;
            if(retVal.IsErr())
            {
                ParseError(line, "Illegal subtraction operation.");
                return false;
            }
        }

		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of AddExpr


// MultExpr ::= UnaryExpr { ( * | / ) UnaryExpr } 
bool MultExpr(istream& in, int& line, Value & retVal) {
	
	Value val1, val2;
	bool t1 = UnaryExpr(in, line, val1);

	retVal = val1;

	LexItem tok;
	
	if( !t1 ) {
		return false;
	}

	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  )
	{
		t1 = UnaryExpr(in, line, val2);
		
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}

        if(tok == MULT){
            retVal = retVal * val2;
            if(retVal.IsErr()){
                ParseError(line, "Illegal multiplication operation.");
                //cout << "(" << tok.GetLexeme() << ")" << endl;
                return false;
            }
		}
		else if(tok == DIV){
			if(val2.IsInt()){
				if(val2.GetInt() == 0){
					cout<<"RUNTIME ERROR: Division by zero"<<endl;
					return false;
				}
			}
			else if(val2.IsReal()){
				if(val2.GetReal() == 0.0){
					cout<<"RUNTIME ERROR: Division by zero"<<endl;
					return false;
				}
			}
			else {
				cout<<"RUNTIME ERROR: Division by string"<<endl;
				return false;
			}
			retVal = retVal / val2;
		}
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		//Evaluate: evaluate the expression for multiplication or division
	}
	Parser::PushBackToken(tok);
	return true;
}//End of MultExpr


// UnaryExpr ::= ( - | + | ! ) PrimaryExpr | PrimaryExpr 
bool UnaryExpr(istream& in, int& line, Value & retVal)
{
	Value val1;
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if(t == MINUS )
	{
		sign = -1;
	}
	else if(t == PLUS)
	{
		sign = 1;
	}
	else
		Parser::PushBackToken(t);
		
	status = PrimaryExpr(in, line, sign, val1);
	return status;
}//End of UnaryExpr


// PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | ( Expr ))
bool PrimaryExpr(istream& in, int& line, int sign, Value & retVal) {
	LexItem tok = Parser::GetNextToken(in, line);

	if( tok == IDENT ) {
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second))
		{
			ParseError(line, "Using Undeclared Variable");
			return false;	
		}
		Value y = TempsResults[tok.GetLexeme()];

        retVal = TempsResults.find(lexeme)->second;
		return true;
	}
	else if( tok == ICONST ) {
		retVal.SetType(VINT);
		int val = stoi(tok.GetLexeme());
		if(sign== -1){
			retVal.SetInt(-1*val);
		}
		else{
			retVal.SetInt(val);
		}
		//cout<<"here int"<<endl;
		return true;
	}
	else if( tok == SCONST ) {
		retVal.SetType(VSTRING);
		retVal.SetString(tok.GetLexeme());
		//cout<<"here string"<<endl;
		return true;
	}
	else if( tok == RCONST ) {
		retVal.SetType(VREAL);
		float val = stof(tok.GetLexeme());
		if(sign== -1){
			retVal.SetReal(-1*val);
		}
		else{
			retVal.SetReal(val);
		}
		//cout<<"here real"<<endl;
		return true;
	}
	else if( tok == BCONST ) {
		
		return true;
	}
	else if( tok == LPAREN ) {
		Value val1;
		bool ex = Expr(in, line, val1);
		retVal = val1;
		if( !ex ) {
			ParseError(line, "Missing expression after Left Parenthesis");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;
		else 
		{
			Parser::PushBackToken(tok);
			ParseError(line, "Missing right Parenthesis after expression");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return true;
}


