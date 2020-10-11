#include "Lexicon.h"
using namespace std;
Lexicon::Lexicon(Ontology* ontology, , bool expanding_ont){
	this.ontology = ontology;
	this.allow_expanding_ont = expanding_ont;
	generator_should_flush = false;
	// read lex from files, line 11,
	// update support structures, do in constructor


}

void Lexicon::compute_pred_to_surface(vector<int> pts){

}

vector<int> Lexicon::compute_reverse_entries(){

}

int Lexicon::calc_exp_args(int idx){
    int exp_args = 0;
    int curr_cat = semantic_forms[idx].category;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        exp_args += 1;
        curr_cat = categories[curr_cat][0];
    }
    return exp_args;
}

int Lexicon::calc_return_cat(int idx){
    int curr_cat = semantic_forms[idx].category;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        curr_cat = categories[curr_cat][0];
    }
    return curr_cat;
}

// c in self categories?
int Lexicon::get_or_add_category(int c){
	// if () {
    //     return 
    // }
    categories.push_back();
    return categories.size - 1;
}

// two diff returns, can be a str or int
string Lexicon::compose_str_from_category(int idx){

}

vector<> Lexicon::get_semantic_forms_for_surface_form(vector<> surface_form){

}

vector<> Lexicon::get_surface_forms_for_predicate(vector<> pred){

}

// three diff returns
void Lexicon::read_lex_from_file(fname, bool allow_expanding_ont, ){

}

void Lexicon::expand_lex_from_strs(vector<> lines, vector<> surface_forms, vector<>semantic_forms, vector<> entries, vector<> pred_to_surface, bool allow_expanding_ont){

}

// two returns, returns as boost vec of int and semanticnode* (indexes 0 and 1)
int Lexicon::read_syn_sem_cat_idx(std::string s, bool allow_expanding_ont){
    std::string str = s;
    std::string delimiter = " : ";
    size_t pos = 0;
    std::string token;
    std::string lhs;
    std::string rhs;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        lhs = token;
        str.erase(0, pos + delimiter.length());
    }
    rhs = str;
    // uses strip function (no c equivalent in python)
    int cat_idx = read_category_from_str(std::string left(strip(const_cast<char*>(lhs.c_str()))));
    // check this
    SemanticNode *semantic_form = read_semantic_form_from_str(std::string right(strip(const_cast<char*>(rhs.c_str()))), cat_idx, NULL, vector<std::string> scoped; , true);
    std::vector<boost::variant<int, SemanticNode *>> returns;
    returns.push_back(cat_idx);
    returns.push_back(semantic_form);
    return returns;
}


// not sure, no c equivalent for python split
char *strip(char *s) {
    size_t size;
    char *end;
    size = strlen(s);
    if (!size)
        return s;
    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';
    while (*s && isspace(*s))
        s++;
    return s;
}

vector<int> Lexicon::get_all_preds_from_semantic_form(SemanticNode* node){

}

bool Lexicon::form_contains_DESC_predicate(SemanticNode* node){

}

int Lexicon::read_category_from_str(std::string s){

}


// new param bool? 
SemanticNode* Lexicon::read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, vector<std::string> scoped_lambdas, bool allow_expanding_ont){

}

void Lexicon::delete_semantic_form_for_surface_form(surface_form, int ont_idx){

}