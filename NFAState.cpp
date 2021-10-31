#include "NFAState.h"
#include <string>
#include <unordered_map>


NFAState::NFAState(Token* t) : token(t) { } 

void NFAState::InsertState(std::string regex, NFAState* state)
{
	if (nextList.find(regex) == nextList.end())
	{
		nextList.insert(std::pair<std::string, std::unordered_set<NFAState*>>(regex, { state }));
	}
	else
	{
		nextList.find(regex)->second.insert(state);
	}
}
