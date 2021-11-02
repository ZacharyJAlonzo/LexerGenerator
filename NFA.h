#ifndef NFA_H
#define NFA_H

#pragma once

class NFAState;


class NFA
{

public:
	NFA() = default;
	NFAState* first = nullptr;
	NFAState* last = nullptr;
};



#endif
