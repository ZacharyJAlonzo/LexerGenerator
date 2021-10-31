#include "Lexer.h"
#include "Token.h"
#include "NFA.h"
#include "NFAState.h"
#include <iostream>

Lexer::Lexer(NFA* dfa, std::fstream* f, std::unordered_map<std::string, int> key) : DFA(dfa), file(f), Keywords(key)
{
	file->read(&current, 1);
}

Token Lexer::Scan()
{	
	NFAState* start = DFA->first;
	NFAState* cur = start;

	std::string lexeme = "";
	while (!file->eof())
	{
	
		char buf[1] = { current };
		//deal with ranges
		if (cur->nextList.find("[a-z]") != cur->nextList.end() && std::isalpha(current) && std::islower(current))
		{
			cur = *(cur->nextList.find("[a-z]")->second.begin());
			lexeme += current;
		}
		else if (cur->nextList.find("[A-Z]") != cur->nextList.end() && std::isalpha(current) && std::isupper(current))
		{
			cur = *(cur->nextList.find("[A-Z]")->second.begin());
			lexeme += current;
		}
		else if (cur->nextList.find("[0-9]") != cur->nextList.end() && std::isdigit(current))
		{
			cur = *(cur->nextList.find("[0-9]")->second.begin());
			lexeme += current;
		}
		else if (cur->nextList.find(std::string(buf, 1)) != cur->nextList.end())
		{
			cur = *(cur->nextList.find(std::string(buf, 1))->second.begin());
			lexeme += current;
		}
		else //not a possible next step
		{
			if (cur->token)
			{
				if (IsKeyword(lexeme))
				{
					std::string type = lexeme;
					for (int i = 0; i < type.size(); i++)
						type[i] = std::toupper(type[i]);

					Token token(type);
					token.lexeme = lexeme;
					
					return token;
				}
				else
				{
					Token token(cur->token->type);
					token.lexeme = lexeme;

					return token;
				}				
			}
			else
			{
				std::cout << "encountered error in lexing\n";
				exit(-1);
			}
		}

		file->read(&current, 1);		
	}

	//EOF reached, check the last lexeme
	if (cur->token)
	{
		if (IsKeyword(lexeme))
		{
			std::string type = lexeme;
			for (int i = 0; i < type.size(); i++)
				type[i] = std::toupper(type[i]);

			Token token(type);
			token.lexeme = lexeme;

			return token;
		}
		else
		{
			Token token(cur->token->type);
			token.lexeme = lexeme;

			return token;
		}
	}
	else
	{
		std::cout << "encoutered error in lexing.\n";
		exit(-1);
	}

}

bool Lexer::IsKeyword(std::string lexeme)
{
	for (int i = 0; i < lexeme.size(); i++)
	{
		lexeme[i] = std::toupper(lexeme[i]);
	}

	if (Keywords.find(lexeme) == Keywords.end())
	{
		return false;
	}

	return true;
}
