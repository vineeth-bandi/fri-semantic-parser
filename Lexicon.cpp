#include "Lexicon.h"
using namespace std;
Lexicon::Lexicon(Ontology* ontology, , bool expanding_ont){
	this.ontology = ontology;
	this.allow_expanding_ont = expanding_ont;
	generator_should_flush = false;
	// read lex from files, line 11, how do we do this?
	// update support structures, do in constructor

}

void Lexicon::compute_pred_to_surface(vector<int> pts){

}

vector<int> Lexicon::compute_reverse_entries(){

}

int Lexicon::calc_exp_args(int idx){

}

int Lexicon::calc_return_cat(int idx){

}

int Lexicon::get_or_add_category(int c){
	
}

// two diff returns, can be a str or int
string Lexicon::compose_str_from_category(int idx){

}

vector<> Lexicon::get_semantic_forms_for_surface_form(vector<>surface_form){

}

vector<> Lexicon::get_surface_forms_for_predicate(vector<> pred){

}

// three diff returns
void Lexicon::read_lex_from_file(fname, bool allow_expanding_ont, ){

}

void Lexicon::expand_lex_from_strs(vector<> lines, vector<> surface_forms, vector<>semantic_forms, vector<> entries, vector<> pred_to_surface, bool allow_expanding_ont){

}

// two returns, do we want to pass inpointers?
int Lexicon::read_syn_sem(vector<> s, bool allow_expanding_ont){

}

vector<int> Lexicon::get_all_preds_from_semantic_form(SemanticNode* node){

}

bool Lexicon::form_contains_DESC_predicate(SemanticNode* node){

}

int Lexicon::read_category_from_str(vector<>s){

}

SemanticNode* Lexicon::read_semantic_form_from_str(vector<> s, int category, parent, scoped_lambdas, bool allow_expanding_ont){

}

void Lexicon::delete_semantic_form_for_surface_form(surface_form, int ont_idx){

}
