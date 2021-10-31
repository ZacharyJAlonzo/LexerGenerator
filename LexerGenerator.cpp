// LexerGenerator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "LexGen.h"
#include "NFA.h"
#include "Lexer.h"
#include "Token.h"

int main()
{
    
    std::fstream file;
    file.open("Input.txt");

    std::fstream inputFile;
    inputFile.open("Test.txt");

    LexGen gen;
    NFA* nfa = gen.BuildNFA(&file);
   // gen.PrintNFA((gen.BuildNFA(&file))->first, 0, "", {});
    NFA* dfa = gen.NFAtoDFA(nfa);

    Lexer lex(dfa, &inputFile, gen.Keywords);


    while (inputFile.good())
    {
        Token t = lex.Scan();
        if(t.type != "WS")
        std::cout << t.type << ": " << t.lexeme << "\n";
    }

}
