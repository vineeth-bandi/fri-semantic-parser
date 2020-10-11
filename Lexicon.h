#include "SemanticNode.h"
#include <vector>
#include "Ontology.h"
#include <string.h>
#include <algorithm>
#include <boost/variant.hpp>

using namespace std;

class Lexicon{

public:
	Lexicon(Ontology ontology, , bool expanding_ont)
    Ontology* ontology;
    vector<boost::variant<std::string, std::vector<int>>> categories;
    // still figure out what the lists contain
    std::vector<> entries;
    std::vector<> pred_to_surface;
    std::vector<> surface_forms;
    std::vector<> semantic_forms;
    std::vector<> reverse_entries;
    std::vector<> sem_form_expected_args;
    std::vector<> sem_form_return_cat;
    std::vector<> category_consumes;
    bool generator_should_flush;
    bool sem_form_expected;
    bool allow_expanding_ont;
	void compute_pred_to_surface(vector<int> pts);
	vector<int> compute_reverse_entries();
	int calc_exp_args(int idx);
	int calc_return_cat(int idx);
	int get_or_add_category(int c);
	// two diff returns, can be a str or int
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
