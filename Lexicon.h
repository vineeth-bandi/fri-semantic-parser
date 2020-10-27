#include "SemanticNode.h"
#include <vector>
#include "Ontology.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <boost/variant.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <Eigen/Dense>
class Lexicon
{

public:
	Lexicon(Ontology *ontology, std::string lexicon_fname, std::string word_embeddings_fn, std::string vocab_fn);
	Ontology *ontology;
	std::vector<boost::variant<std::string, std::vector<int>>> categories;
	std::vector<std::vector<int>> entries;
	std::unordered_map<int, std::vector<int>> pred_to_surface;
	std::vector<std::string> surface_forms;
	std::vector<SemanticNode *> semantic_forms;
	std::vector<std::vector<int>> reverse_entries;
	std::vector<int> neighbor_surface_forms;
	std::vector<int> sem_form_expected_args;
	std::vector<int> sem_form_return_cat;
	std::vector<std::vector<std::tuple<int, int>>> category_consumes;
	bool generator_should_flush;
	bool sem_form_expected;
	Eigen::MatrixXd wv;
	std::unordered_map<std::string, int> vocab;
	void load_word_embeddings(std::string fn, std::string fn2);
	std::vector<std::tuple<int, double>> get_lexicon_word_embedding_neighbors(std::string w, int n); // nalin
	void update_support_structures();
	void compute_pred_to_surface(std::unordered_map<int, std::vector<int>>);
	std::vector<std::vector<int>> compute_reverse_entries();
	int calc_exp_args(int idx);
	int calc_return_cat(int idx);
	std::vector<std::tuple<int, int>> find_consumables_for_cat(int idx);
	int get_or_add_category(boost::variant<std::vector<int>, std::string> c);
	std::string compose_str_from_category(int idx);
	std::vector<int> get_semantic_forms_for_surface_form(std::string surface_form);
	std::vector<int> get_surface_forms_for_predicate(boost::variant<std::string, int> pred);
	void read_lex_from_file(std::string fname);
	void expand_lex_from_strs(std::vector<std::string> lines);
	std::vector<boost::variant<int, SemanticNode *>> read_syn_sem(std::string s);
	std::vector<int> get_all_preds_from_semantic_form(SemanticNode *node);
	int read_category_from_str(std::string s);
	SemanticNode *read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, std::vector<std::string> scoped_lambdas);
	SemanticNode *instantiate_wild_type(SemanticNode *root);
	/* void delete_semantic_form_for_surface_form(std::string surface_form, int ont_idx); */
};
