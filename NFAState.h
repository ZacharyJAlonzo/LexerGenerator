#ifndef NFASTATE_H
#define NFASTATE_H

#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>


class Token;



class NFAState
{

public:

	NFAState() = default;
	NFAState(Token* t);
	void InsertState(std::string regex, NFAState* state);
	
	std::unordered_map<std::string, std::unordered_set<NFAState*>> nextList = {};
	Token* token = nullptr;

};


#endif
