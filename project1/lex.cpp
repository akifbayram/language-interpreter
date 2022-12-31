#include "lex.h"
#include <algorithm>
using namespace std;

map<int, string> enumList = {
		{ 0, "PROGRAM" },
		{ 1, "PRINT" },
		{ 2, "INT" },
		{ 3, "END" },
		{ 4, "IF" },
		{ 5, "FLOAT" },
		{ 6, "BOOL" },
		{ 7, "ELSE" },
		{ 8, "THEN" },
		{ 9, "TRUE" },
		{ 10, "FALSE" },
		{ 11, "IDENT" },
		{ 12, "ICONST" },
		{ 13, "RCONST" },
		{ 14, "SCONST" },
		{ 15, "BCONST" },
		{ 16, "PLUS" },
		{ 17, "MINUS" },
		{ 18, "MULT" },
		{ 19, "DIV" },
		{ 20, "ASSOP" },
		{ 21, "LPAREN" },
		{ 22, "RPAREN" },
		{ 23, "COMMA" },
		{ 24, "EQUAL" },
		{ 25, "GTHAN" },
		{ 26, "LTHAN" },
		{ 27, "SEMICOL" },
		{ 28, "AND" },
		{ 29, "OR" },
		{ 30, "NOT" },
		{ 31, "ERR" },
		{ 32, "DONE" },
};

ostream& operator<<(ostream& out, const LexItem& tok) {
    Token tkn = tok.GetToken();    
    string lex = tok.GetLexeme();

    if ( (tkn >= 11 && tkn <= 13)  || (tkn == 15 )) {
        transform(lex.begin(), lex.end(), lex.begin(), ::toupper);
        out << enumList[tkn] << "(" << lex << ")";
    }
    else if ( tkn == 14 )
        out << enumList[tkn] << "(" << lex << ")";
    else if ( (tkn >= 0 && tkn <= 10 ) || (tkn >= 16 && tkn <= 30) ) {
        transform(lex.begin(), lex.end(), lex.begin(), ::toupper);
        out << enumList[tkn]; 
    }
    return out;
};

LexItem id_or_kw(const string& lexeme, int linenum) {
    map<string, Token> keywordList = {
        { "PROGRAM", PROGRAM },
        { "END", END },
        { "end", END },
        { "print", PRINT },
        { "if", IF },
        { "then", THEN },
        { "int", INT }, 
        { "float", FLOAT },
        { "bool", BOOL },
        { "else", ELSE },
        { "true", TRUE },
        { "false", FALSE },
    };
    Token keyword = IDENT;
    
    auto it = keywordList.find(lexeme);
    
    if (it != keywordList.end() ) {
        keyword = it->second;
    }

    if ( keyword == TRUE || keyword == FALSE )
        keyword = BCONST;

    return LexItem(keyword, lexeme, linenum);
};

LexItem getNextToken (istream& in, int& linenumber) {
    enum TokState { START, INID, INSTRING, ININT, INREAL, INCOMMENT} lexstate = START;
    string lexeme;
    char ch;

    while ( in.get(ch) ) {
        switch ( lexstate ) {
            case START:
                if ( ch == '\n') {
                    linenumber++;
                    continue;
                }
                if ( isspace(ch) ) 
                    continue;
                if (  ch == '.' && isdigit(in.peek()) ) { // if decimal followed by a number
                    lexeme += ch; 
                    return LexItem(ERR, lexeme, linenumber); // return error
                }
                if ( ch == '"' )
                    lexstate = INSTRING;
                
                else if ( isalpha(ch) || ch == '_' ) {
                    lexstate = INID;
                    lexeme = ch;
                }
                else if ( isdigit(ch) ) {
                    lexstate = ININT;
                    lexeme = ch;
                }
                else if ( ch == '/') {
                    if (in.peek() == '*' ) {
                        in.get(ch);
                        lexstate = INCOMMENT;
                    }
                    else   
                        return LexItem(DIV, lexeme, linenumber);
                }
                else {
                    lexeme = ch;
                    switch (ch) {
                        case '+':
                            return LexItem(PLUS, lexeme, linenumber);
                            break;
                        case '-':
                            return LexItem(MINUS, lexeme, linenumber);
                            break;
                        case '*':
                            return LexItem(MULT, lexeme, linenumber);
                            break;
                        // case '/':
                        //     return LexItem(DIV, lexeme, linenumber);
                        //     break;
                        case '!':
                            in.get(ch);
                            if ( in.peek() == '=' )
                                return LexItem(NOT, lexeme, linenumber);
                            // else
                            //     in.putback(ch);
                            break;
                        case '=':
                            in.get(ch);
                            if ( ch == '=' ) 
                                return LexItem(EQUAL, lexeme, linenumber);
                            else {
                                in.putback(ch);
                                return LexItem(ASSOP, lexeme, linenumber);
                            }
                            break;
                        case '(':
                            return LexItem(LPAREN, lexeme, linenumber);
                            break;
                        case ')':
                            return LexItem(RPAREN, lexeme, linenumber);
                            break;
                        case '>':
                            return LexItem(GTHAN, lexeme, linenumber);
                            break;
                        case '<':
                            return LexItem(LTHAN, lexeme, linenumber);
                            break;
                        case '&':
                            if ( in.peek() == '&' ) {
                                in.get(ch); // advance
                                return LexItem(AND, lexeme, linenumber);
                            }
                            break;
                        case '|':
                            if ( in.peek() == '|' ) {
                                in.get(ch); // advance
                                return LexItem(OR, lexeme, linenumber);
                            }
                            break;
                        case ';':
                            return LexItem(SEMICOL, lexeme, linenumber);
                            break;
                        case ',':
                            return LexItem(COMMA, lexeme, linenumber);
                            break;
                    } // endSwitch
                } // endElse
                break;

            case INID:
                if ( isdigit(ch) || isalpha(ch) || ch == '_' || ch == '@' )  
                    lexeme += ch;
                else if ( ch == '$') {
                    lexeme = ch;
                    return LexItem(ERR, lexeme, linenumber);
                }
                else {
                    in.putback(ch); 
                    return id_or_kw(lexeme, linenumber);
                }
                break;

            case INSTRING:
                if ( ch == '"' )  // if end of string
                    return LexItem(SCONST, lexeme, linenumber);
                else if ( ch == '\n' ) { 
                    lexeme.insert(0, 1,'\"');
                    return LexItem(ERR, lexeme, linenumber);
                }
                lexeme += ch;
                break;

            case ININT: // for recognizing integer constants
                if ( ch == '.' ) {
                    lexstate = INREAL; // identified as a real constant
                    in.putback(ch);
                }
                else if ( isdigit(ch) ) // if digit 
                    lexeme += ch; 
                else {
                    in.putback(ch); // rewind
                    return LexItem(ICONST, lexeme, linenumber);
                }
                break;

            case INREAL:
                if ( ch == '.' && isdigit(in.peek()) ) // if has a decimal followed by a digit
                    lexeme += ch; // keep accumulating
                else if ( ch == '.' && !isdigit(in.peek()) ) { // if has decimal but no following digit 
                    lexeme += ch; 
                    return LexItem(ERR, lexeme, linenumber); // return error
                }                
                else if ( count(lexeme.begin(), lexeme.end(), '.') >= 2)
                    return LexItem(ERR, lexeme, linenumber); // return error
                else if ( isdigit(ch) )
                    lexeme += ch; // keep accumulating
                else {
                    in.putback(ch); // rewind
                    return LexItem(RCONST, lexeme, linenumber);
                }
                break;

            case INCOMMENT:
                if ( ch == '*' && in.peek() == '/' ) {
                    in.get(ch); // advance
                    lexstate = START; // if comment finished, return to start
                }
                else if ( ch == '\n' )
                    linenumber++;
                else
                    continue;
                break;

        } //endSwitch
    } // endWhile
    
    if ( lexstate == INCOMMENT )
        return LexItem(ERR, lexeme, linenumber);
    else if ( in.eof() ) 
        return LexItem(DONE, lexeme, linenumber);
    else   
        return LexItem(ERR, lexeme, linenumber);

}; // endLexItem