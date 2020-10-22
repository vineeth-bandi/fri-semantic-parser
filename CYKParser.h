#ifndef CYKPARSER_H
#define CYKPARSER_H
#include "Ontology.h"
#include "Lexicon.h"
#include "SemanticNode.h"
#include "ParseNode.h"
#include <unordered_map>
#include <string>
#include <vector>

std::string ont_fname = "ont.txt";
std::string lex_fname = "lex.txt";

typedef std::tuple<int, int> tuple2;
typedef std::tuple<int, int, int> tuple3;
typedef std::tuple<std::vector<int>, std::vector<int>> vvTuple2;
typedef std::tuple<int, std::vector<int>> ivTuple2;
typedef std::tuple<std::string, SemanticNode> dTuple;
typedef std::tuple<std::string, ParseNode, ParseNode, std::vector<std::vector<int>>, std::vector<std::vector<int>>, std::vector<std::string>, std::vector<std::string>> tupleTP;
typedef boost::variant<SemanticNode, ParseNode> boostNode;
typedef boost::variant<int, std::string, SemanticNode> lex_entries;
typedef boost::variant<std::string, ParseNode, std::vector<std::vector<lex_entries>>, std::vector<std::string>> boostT;

class Parameters
{
public:
    Ontology ont_;
    Lexicon lex_;
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

    Parameters(Ontology ont, Lexicon lex, bool allow_merge, bool use_language_model = false, double lexicon_weight = 1.0);
    void update_probabilities();
    std::unordered_map<int, double> init_skipwords_given_surface_form(double lexicon_weight);
    std::unordered_map<tuple2, double> init_ccg_given_token(double lexicon_weight);
    std::unordered_map<tuple3, double> init_ccg_production(double lexicon_weight, bool allow_merge);
    std::unordered_map<tuple2, double> init_lexicon_entry(double lexicon_weight);
    double get_semantic_score(ParseNode n);
    std::unordered_map<vvTuple2, int> count_token_bigrams(ParseNode y);
    std::unordered_map<tuple3, int> count_semantics(boostNode sn);
    void update_learned_parameters(std::vector<boostT> t);
    std::unordered_map<vvTuple2, int> count_lexical_entries(ParseNode y);
    std::unordered_map<tuple3, int> count_ccg_productions(ParseNode y);
    std::unordered_map<int, int> count_ccg_root(ParseNode y);
    std::unordered_map<ivTuple2, int> count_ccg_surface_form_pairs(ParseNode y);
};

class CYKParser
{
public:
    Ontology ont_;
    Lexicon lex_;
    Parameters theta;
    bool safety;
    bool use_language_model;
    bool allow_merge;
    bool perform_type_raising; 
    bool parsing_timeout_on_last_parse;
    std::unordered_map<int, int> type_raised; // map from semantic form idx to their type-raised form idx
    std::unordered_map<ParseNode, std::unordered_map<ParseNode, ParseNode>> cached_combinations; // indexed by left, then right node, value at result
    // additional linguistic information and parameters
    // TODO: read this from configuration files or have user specify it on instantiation
    const int max_multiword_expression = 1;
    const int max_new_senses_per_utterance = 3;
    const int max_cky_trees_per_token_sequence_beam = 32;
    const int max_hypothesis_categories_for_unknown_token_beam = 32;
    const int max_expansions_per_non_terminal = 32;
    const int max_new_skipwords_per_utterance = 2;
    const int max_missing_words_to_try = 2;
    const int missing_lexicon_entry_given_token_penalty = -100;
    const int missing_CCG_given_token_penalty = -100;
    const int training_reverse_fa_beam = 32;
    const int max_skip_sequences_to_consider = 32;
    const int training_max_topdown_trees_to_consider = 32;
    const int max_leaf_assignments_to_consider = 64;
    //method headers
    double get_language_model_score(ParseNode y);
    void type_raise_bare_nouns();
    std::string print_parse(SemanticNode p, bool show_category = false, bool show_lambda_types = false);
    std::vector<dTuple> read_in_paired_utterance_semantics(std::string fname);
    bool train_learner_on_semantic_forms(vector<dTuple> d, int epochs = 10, int epoch_offset = 0, int reranker_beam = 1, int verbose = 2, bool use_condor = false, std::string condor_target_dir, std::string condor_script_dir, std::vector<int> perf_log);
    std::tuple<tupleTP, int> get_training_pairs(vector<dTuple> d, int verbose = 2, int reranker_beam = 1, bool use_condor = false, std::string condor_target_dir, std::string condor_script_dir);
    most_likely_cky_parse(std::string s, int reranker_beam = 1, ParseNode known_root = NULL, reverse_fa_beam, bool debug = false, timeout);


    std::vector<std::string> tokenize(std::string s);
    
};


#endif