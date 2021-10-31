#ifndef LEXER_H
#define LEXER_H

#pragma once
#include <fstream>
#include <string>
#include <unordered_map>

class Token;
class NFA;

class Lexer
{

public:
	
	Lexer(NFA* dfa, std::fstream* f, std::unordered_map<std::string, int> key);
	Token Scan();
	bool IsKeyword(std::string lexeme);


	NFA* DFA;
	std::fstream* file;
	std::unordered_map<std::string, int> Keywords;

	char current = ' ';



};




#endif
