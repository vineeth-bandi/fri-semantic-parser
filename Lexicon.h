#include "SemanticNode.h"
#include <vector>
#include "Ontology.h"
#include <string>
#include <algorithm>

using namespace std;

class Lexicon{

// TODO: all vectors change to boosts, confirm types in python and do type checking with boosts
private:
  // is this the right way to define an ontology?
  Ontology* ontology;
  // what is the type of category -> has lists of ints, and strings
  vector<int> categories;
  // look at line 11 for more details on how to implement
  vector<> entries;
  vector<> pred_to_surface;
  vector<> surface_forms;
  vector<> semantic_forms;
  vector<> reverse_entries;
  vector<> sem_form_expected_args;
  vector<> sem_form_return_cat;
  vector<> category_consumes;
  bool generator_should_flush;
  bool sem_form_expected;
  bool allow_expanding_ont;
public:
	Lexicon(Ontology ontology, , bool expanding_ont)
	void compute_pred_to_surface(vector<int> pts);
	vector<int> compute_reverse_entries();
	int calc_exp_args(int idx);
	int calc_return_cat(int idx);
	int get_or_add_category(int c);
	// two diff returns, can be a str or int
	string compose_str_from_category(int idx);
	vector<> get_semantic_forms_for_surface_form(vector<>surface_form);
	vector<> get_surface_forms_for_predicate(vector<> pred);
	// three diff returns
	// do we want to pass in pointers?
	void read_lex_from_file(fname, bool allow_expanding_ont, );
	void expand_lex_from_strs(vector<> lines, vector<> surface_forms, vector<>semantic_forms, vector<> entries, vector<> pred_to_surface, bool allow_expanding_ont);
	// two returns, do we want to pass inpointers?
	int read_syn_sem(vector<> s, bool allow_expanding_ont);
	vector<int> get_all_preds_from_semantic_form(SemanticNode* node);
	bool form_contains_DESC_predicate(SemanticNode* node);
	int read_category_from_str(vector<>s);
	SemanticNode* read_semantic_form_from_str(vector<> s, int category, parent, scoped_lambdas, bool allow_expanding_ont)
	void delete_semantic_form_for_surface_form(surface_form, int ont_idx);
};
