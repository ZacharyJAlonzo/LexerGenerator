#include "LexGen.h"
#include "NFAState.h"
#include "NFA.h"
#include "Token.h"
#include <iostream>
#include <unordered_set>
#include <stack>




NFA* LexGen::BuildNFA(std::fstream* file)
{
	//read token types
	std::string line;
	int index = 0;
	
	getline(*file, line);
	while (line != "%%")
	{
		Keywords[line] = index++;
		TokenTypes[line] = index++;
		getline(*file, line);
	}
	getline(*file, line);
	while (line != "%")
	{
		TokenTypes[line] = index++;
		getline(*file, line);
	}
	
	NFA* CompleteNFA = new NFA();
	CompleteNFA->first = new NFAState();
	while (std::getline(*file, line))
	{
		NFA* RegexNFA = RegexToNFA(line);
		CompleteNFA->first->InsertState("`", RegexNFA->first);
	}
	
	return CompleteNFA;
}

NFA* LexGen::RegexToNFA(std::string line)
{
	//Determine the token type//
	std::string TokenType = line.substr(0, line.find(":"));
	line = line.substr(line.find(":")+1);


	//build the NFA//
	std::stack<NFA*> NFAStack = {};
	std::stack<char> SymbolStack = {};

	for (int i = 0; i < line.size(); i++)
	{
		switch (line[i])
		{
			case '*':
			case '|':
			case '(':
			{
				SymbolStack.push(line[i]);
				break;
			}
			case '"':
			{
				//string processing
				i++; //eat "
				char buf[1] = { line[i] };
				NFA* fin = SimpleNFA(std::string(buf, 1)); //take everything literally
				i++;
				while (line[i] != '"')//merge forwards
				{
					buf[0] = line[i];
					NFA* t = SimpleNFA(std::string(buf, 1));
					fin->last->InsertState("`", t->first);
					fin->last = t->last;
					i++;
				}
				NFAStack.push(fin);

				break;
			}
			case ')':
			{
				//unwind the stack
				while (SymbolStack.top() != '(')
				{
					switch (SymbolStack.top())
					{
						case '*':
						{
							NFA* one = NFAStack.top();
							NFAStack.pop();
							
							NFAStack.push(KleeneNFA(one));

							SymbolStack.pop();

							break;
						}
						case '|':
						{
							NFA* one = NFAStack.top();
							NFAStack.pop();
							NFA* two = NFAStack.top();
							NFAStack.pop();

							NFAStack.push(OrNFA(one, two));

							SymbolStack.pop();

							break;
						}
					}
				}

				SymbolStack.pop();
				
				break;
			}
			case '[':
			{
				//range regex specified
				i++;
				switch (line[i])
				{
					case 'a':
					{
						NFAStack.push(SimpleNFA("[a-z]"));
						break;
					}
					case 'A':
					{
						NFAStack.push(SimpleNFA("[A-Z]"));
						break;
					}
					case '0':
					{
						NFAStack.push(SimpleNFA("[0-9]"));
						break;
					}
				}

				i += 3;
				break;
			}
			case '\\':
			{
				i++; //eat backslash

				switch (line[i])
				{
					case 'n':
					{
						NFAStack.push(SimpleNFA("\n"));
						break;
					}
					case 't':
					{
						NFAStack.push(SimpleNFA("\t"));
						break;
					}
					case 'r':
					{
						NFAStack.push(SimpleNFA("\r"));
						break;
					}
					default:
					{
						char buf[1] = { line[i] };
						NFAStack.push(SimpleNFA(std::string(buf, 1)));
						break;
					}
				}

				break;
			}
			default:
			{
				char buf[1] = { line[i] };
				NFAStack.push(SimpleNFA(std::string(buf, 1)));

				break;
			}


		}

	}

	NFA* top = NFAStack.top();
	NFAStack.pop();

	while (!NFAStack.empty())
	{
		NFA* prev = NFAStack.top();
		NFAStack.pop();

		prev->last->InsertState("`", top->first);
		prev->last = top->last;

		top = prev;
	}

	top->last->token = new Token(TokenType);

	return top;
}



NFAState* LexGen::Move(NFAState* current, std::string regex)
{
	if (current->nextList.find(regex) != current->nextList.end())
	{
		return *(current->nextList.find(regex)->second.begin());
		//the only element in the list
	}
	else return nullptr;
}

Token* LexGen::containsAcceptState(std::unordered_set<NFAState*> set)
{

	for (auto it = set.begin(); it != set.end(); it++)
	{
		if ((*it)->token)
		{
			return new Token((*it)->token->type);
		}
	}

	return nullptr;
}

std::unordered_set<NFAState*> LexGen::FollowEpsilon(NFAState* current)
{
	if (current == nullptr)
		return {};

	std::unordered_set<NFAState*> stateList;

	if (current->nextList.find("`") != current->nextList.end())
	{
		std::unordered_set<NFAState*> EList = current->nextList.find("`")->second;
			
		for (auto it = EList.begin(); it != EList.end(); it++)
		{
			stateList.insert(*it);
			std::unordered_set<NFAState*> recur = FollowEpsilon(*it);
			stateList.insert(recur.begin(), recur.end());
		}
	
	}

	return stateList;
}


NFA* LexGen::OrNFA(NFA* top, NFA* bottom)
{
	NFA* OrNFA = new NFA();
	OrNFA->first = new NFAState();
	OrNFA->last = new NFAState();

	OrNFA->first->InsertState("`", top->first);
	OrNFA->first->InsertState("`", bottom->first);

	top->last->InsertState("`", OrNFA->last);
	bottom->last->InsertState("`", OrNFA->last);

	return OrNFA;
}

NFA* LexGen::SimpleNFA(std::string regex)
{
	NFAState* first = new NFAState();
	NFAState* last = new NFAState();

	first->InsertState(regex, last);

	NFA* simple = new NFA();
	simple->first = first;
	simple->last = last;

	return simple;
}

NFA* LexGen::KleeneNFA(NFA* repeat)
{
	NFA* KleeneNFA = new NFA();

	KleeneNFA->first = new NFAState();
	KleeneNFA->last = new NFAState();

	KleeneNFA->first->InsertState("`", repeat->first);
	KleeneNFA->first->InsertState("`", KleeneNFA->last);

	repeat->last->InsertState("`", KleeneNFA->last);
	repeat->last->InsertState("`", repeat->first);

	return KleeneNFA;
}

NFA* LexGen::NFAtoDFA(NFA* input)
{

	NFA* DFA = new NFA();

	std::string chars[89] = { "a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z",
							"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
							"0","1","2","3","4","5","6","7","8","9",
							"\\","\"","\n","\t","\r"," ",
							"*","+","-","/","=",">","<","!","%",
							";","{","}","(",")",".",
							"[a-z]","[A-Z]","[0-9]"};

	const int USABLE_SIZE = 89;
	char usable[USABLE_SIZE];
	//strcpy_s(usable, chars.c_str());


	std::vector<NFAState*> DFAStates;
	std::vector<std::unordered_set<NFAState*>> D;
	std::vector<std::unordered_set<NFAState*>> W;

	//from NFA First state, perform FollowEpsilon
	std::unordered_set<NFAState*> d0 = FollowEpsilon(input->first);

	int index = 0;

	NFAState* Zero = new NFAState();
	DFAStates.push_back(Zero);

	D = { d0 };
	W = { d0 };

	bool addedNode = false;


	NFAState* currentState = Zero;
	int curIndex = 0;

	while (!W.empty())
	{
		std::unordered_set<NFAState*> S = *W.begin();
		W.erase(W.begin());

		for (int i = 0; i < USABLE_SIZE; i++)
		{
			std::string regex = chars[i];

			std::unordered_set<NFAState*> T;
			for (auto it = S.begin(); it != S.end(); it++) //for each state in S, check the current input
			{
				std::unordered_set<NFAState*> t = FollowEpsilon(Move(*it, regex));
				T.insert(t.begin(), t.end()); //union sets

				NFAState* res = Move(*it, regex);
				if (res)
					T.insert(res); //add in move since it isnt added by follow epsilon
			}

			if (T.empty())
				continue;

			if (std::find(D.begin(), D.end(), T) == D.end()) //set not contained
			{
				NFAState* newState = new NFAState();
				newState->token = containsAcceptState(T);

				DFAStates.push_back(newState);
				addedNode = true;

				//T[S, alpha] <-- T
				currentState->InsertState(regex,  newState);

				D.push_back(T);
				W.push_back(T);

			}
			else //contained
			{
				addedNode = false;
				//find index
				for (int i = 0; i < D.size(); i++)
				{
					if (D[i] == T)
					{
						//DFAStates[i] is the state that we jump to on this input
						//T[S, alpha] <-- T
						currentState->InsertState(regex, DFAStates[i]);
					}
				}

			}

		}
		curIndex++;
		currentState = DFAStates[curIndex % DFAStates.size()]; //mod to avoid out of index errors... 

	}

	DFA->first = DFAStates[0];

	std::unordered_set<NFAState*> allStates = D[0];
	//D should contain all NFA states
	for (int i = 1; i < D.size(); i++)
	{
		allStates.insert(D[i].begin(), D[i].end());
	}

	for (auto it = allStates.begin(); it != allStates.end(); it++)
	{
		delete (*it); //free all previously created NFA states
	}


	std::cout << "DFA with " << DFAStates.size() << " states created. \n";
	return DFA;

}


//for debugging
void LexGen::PrintNFA(NFAState* nfa, int id, std::string tabs, std::unordered_set<NFAState*> seen)
{
	if (nfa)
	{
		if (nfa->token != nullptr)
			std::cout << nfa->token->type << "\n";
		else std::cout << "\n";

		for (auto it = nfa->nextList.begin(); it != nfa->nextList.end(); it++)
		{

			std::cout << tabs << nfa << ": " << it->first << " -> ";

			for (auto iter = it->second.begin(); iter != it->second.end(); iter++)
			{
				if (seen.find(*iter) != seen.end())
				{

					std::cout << tabs << nfa << ": " << *iter << " -> \n";
					continue;
				}
				else
				{
					seen.insert(*iter);
					
					PrintNFA(*(iter), id + 1, tabs + "\t", seen);

				}

			}
		}

	}
}
