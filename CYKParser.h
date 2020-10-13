#ifndef CYKPARSER_H
#define CYKPARSER_H
#include "Ontology.h"
#include "Lexicon.h"
#include <unordered_map>


std::string ont_fname = "ont.txt";
std::string lex_fname = "lex.txt";

typedef std::tuple<int, int> tuple2;
typedef std::tuple<int, int, int> tuple3;

class Parameters
{
public:
    Ontology ont;
    Lexicon lex;
    std::unordered_map<tuple2, int> _token_given_token_counts;
    std::unordered_map<tuple2, int> _CCG_given_token_counts;
    std::unordered_map<tuple3, int> _CCG_production_counts;
    std::unordered_map<int, int> _CCG_root_counts;
    std::unordered_map<tuple2, int> _lexicon_entry_given_token_counts;
    std::unordered_map<tuple3, int> _semantic_counts;
    std::unordered_map<int, double> _skipwords_given_surface_form;

    std::unordered_map<tuple2, double> token_given_token;
    std::unordered_map<tuple2, double> CCG_given_token;
    std::unordered_map<tuple3, double> CCG_production;
    std::unordered_map<int, double> CCG_root;
    std::unordered_map<tuple2, double> lexicon_entry_given_token;
    std::unordered_map<tuple3, double> semantic;
    std::unordered_map<int, double> skipwords_given_surface_form;
};

class CYKParser
{
public:


};


#endif