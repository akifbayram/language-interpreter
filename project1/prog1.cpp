#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>	
#include "lex.cpp"
#include "lex.h"

using namespace std;

int main( int argc, char *argv[]) {
	ifstream inFile;
	
	// File check
	if ( argc > 1 ) { // If arguments present
		inFile.open(argv[1]); // arg[1] argument must be file name
		if ( !inFile.is_open() ) { // If file can be opened
			cout << "CANNOT OPEN THE FILE " << argv[1] << endl;
			return 0;
		}
	}
	else if ( argc == 1 ) { // no arguments present
		cout << "NO SPECIFIED INPUT FILE NAME." << endl;
		return 0;
	}

	// Command line arguments
	string currentFlag;
	bool v = false, iconst = false, rconst = false, sconst = false, bconst = false, ident = false;
    
    for ( int i = 2; i < argc; i++ ) { // Loop through command line arguments, flag if found
    	currentFlag = argv[i]; 
        if ( argv[i][0] != '-' ) {
            cout << "ONLY ONE FILE NAME ALLOWED." << endl;
            return 0;
        }
        if ( currentFlag == "-v" ) 
        	v = true;
        else if ( currentFlag == "-iconst" ) 
        	iconst = true;
        else if ( currentFlag == "-rconst" ) 
        	rconst = true;
        else if ( currentFlag == "-sconst" ) 
        	sconst = true; 
        else if ( currentFlag == "-bconst" ) 
        	bconst = true;
        else if ( currentFlag == "-ident" ) 
        	ident = true;
        else {
            cout << "UNRECOGNIZED FLAG " << argv[i] << endl;
            return 0;
		}
	}

	LexItem tok;
 
	int countTokens = 0, countLines = 0;

    map<Token, map<string, string>> group;
	Token tkn;    
	string lex;

	while ( (tok = getNextToken(inFile, countLines)) != DONE) {
		
		if (v && tok != ERR) 
			cout << tok << endl;

		tkn = tok.GetToken();    
    	lex = tok.GetLexeme();

		if ( ( tok == IDENT || tok == ICONST || tok == RCONST)  || tok == BCONST ) {
			transform(lex.begin(), lex.end(), lex.begin(), ::toupper);
			group[tok.GetToken()][lex] = lex;
			countTokens++;
		}
		else if ( tok == SCONST ) {
			group[tkn][lex] = lex;
			countTokens++;
		}
		else if ( (tkn >= 0 && tkn <= 10 ) || (tkn >= 16 && tkn <= 30) ) {
			transform(lex.begin(), lex.end(), lex.begin(), ::toupper); // uppercase string
			group[tok.GetToken()][lex] = lex;
			countTokens++;			
		}
		else if ( tok == ERR && inFile.eof() ) {
			cout << "\nMissing a comment end delimiters '*/' at line " << countLines << endl;
			break;
		}
		else if ( tok == ERR && !inFile.eof() ) {
			cout << "Error in line " << ++countLines << " (" << tok.GetLexeme() << ")" << endl;
			return 0;
		}
	}

	cout << "Lines: " << countLines << endl;
	if ( countTokens )
		cout << "Tokens: " << countTokens << endl;

	
	map<Token, map<string, string>>::iterator it;

	if ( sconst && !group[SCONST].empty() ) {
		cout << "STRINGS:" << endl;
		for ( auto it : group[SCONST] ) {
			cout << "\"" << it.first << "\"" << endl;
		}
	}
	if ( iconst && !group[ICONST].empty() ) {
		string holder;
		int number;
		
		cout << "INTEGERS:" << endl;
		map<Token, map<int, int>> numbers;
		for ( auto it : group[ICONST] ) {
			holder = it.first;
			number = stoi(holder);
			numbers[ICONST][number] = number;
		}
		map<Token, map<int, int>>::iterator it2;
		for ( auto it2 : numbers[ICONST] ) {
			cout << it2.first << endl;
		}
	}
	if ( rconst && !group[RCONST].empty() ) {
		cout << "REALS:" << endl;
		for ( auto it : group[RCONST] ) {
			cout << it.first << endl;
		}
	}
	if ( bconst && !group[BCONST].empty() ) {
		string holder;
		cout << "Booleans:\n";
		for ( auto it : group[BCONST] ) {
			holder = it.first;
			transform(holder.begin(), holder.end(), holder.begin(), ::toupper);
			cout << holder << endl;
		}
	}
	if ( ident && !group[IDENT].empty() ) {
		string holder;
		int index = 0;
		cout << "IDENTIFIERS:" << endl;
		for ( auto it : group[IDENT] ) {
			holder = it.first;
			transform(holder.begin(), holder.end(), holder.begin(), ::toupper);
			
			// skip the last comma by comparing index to size of map
			if ( group[IDENT].size() - 1 == index ) 
				cout << holder << endl;
			else {
				cout << holder << ", " ;
				index++;
			}
		}
	}
}