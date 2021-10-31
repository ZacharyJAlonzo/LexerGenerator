#ifndef LEXGEN_H
#define LEXGEN_H

#pragma once
#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

class NFA;
class NFAState;
class Token;

class LexGen
{
public:
	LexGen() = default;

private:	
	NFA* RegexToNFA(std::string line);	
	NFAState* Move(NFAState* current, std::string regex);
	std::unordered_set<NFAState*> FollowEpsilon(NFAState* current);
	Token* containsAcceptState(std::unordered_set<NFAState*> set);
	NFA* OrNFA(NFA* top, NFA* bottom);
	NFA* SimpleNFA(std::string regex);
	NFA* KleeneNFA(NFA* repeat);


	

public:
	void PrintNFA(NFAState* nfa, int id, std::string tabs, std::unordered_set<NFAState*> seen);
	NFA* BuildNFA(std::fstream* file);
	NFA* NFAtoDFA(NFA* input);

	std::unordered_map<std::string, int> TokenTypes;
	std::unordered_map<std::string, int> Keywords;
	

};








#endif