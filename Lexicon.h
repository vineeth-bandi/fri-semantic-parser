#include "SemanticNode.h"
#include <vector>
#include "Ontology.h"
#include <string.h>
#include <algorithm>
#include <boost/variant.hpp>

using namespace std;

class Lexicon{

public:
	Lexicon(Ontology ontology, string lexicon_fname, word_embeddings_fn)
    Ontology* ontology;
    vector<boost::variant<std::string, std::vector<int>>> categories;
    std::vector<vector<int>> entries;
    std::map<int, vector<int>> pred_to_surface;
    std::vector<std::string> surface_forms;
    std::vector<SemanticNode *> semantic_forms;
    std::vector<vector<int>> reverse_entries;
    std::vector<int> sem_form_expected_args;
    std::vector<int> sem_form_return_cat;
    std::vector<vector<vector<int>>> category_consumes;
    bool generator_should_flush;
    bool sem_form_expected;
	void update_support_structures();
	void compute_pred_to_surface(std::map<int, vector<int>>);
	vector<vector<int>> compute_reverse_entries()
	int calc_exp_args(int idx);
	int calc_return_cat(int idx);
	vector<int> find_consumables_for_cat(int idx) 
	int get_or_add_category(boost::variant<vector<int>, std::string> c);
	string compose_str_from_category(int idx);
	vector<> get_semantic_forms_for_surface_form(vector<> surface_form);
	vector<> get_surface_forms_for_predicate(vector<> pred);
	// three diff returns
	void read_lex_from_file(fname, bool allow_expanding_ont, );
	void expand_lex_from_strs(vector<> lines, vector<> surface_forms, vector<> semantic_forms, vector<> entries, vector<> pred_to_surface, bool allow_expanding_ont);
	// two returns, do we want to pass inpointers?
	int read_syn_sem(vector<> s, bool allow_expanding_ont);
	vector<int> get_all_preds_from_semantic_form(SemanticNode* node);
	bool form_contains_DESC_predicate(SemanticNode* node);
	int read_category_from_str(vector<>s);
	SemanticNode* read_semantic_form_from_str(vector<> s, int category, SemanticNode *parent, vector<std::string> scoped_lambdas, bool allow_expanding_ont)
	// surface forms type
	void delete_semantic_form_for_surface_form(surface_form, int ont_idx);
};
