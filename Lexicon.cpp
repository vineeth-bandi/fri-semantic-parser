#include "Lexicon.h"
using namespace std;
Lexicon::Lexicon(Ontology* ontology, , bool expanding_ont){
	this.ontology = ontology;
	this.allow_expanding_ont = expanding_ont;
	generator_should_flush = false;

	// read lex from files, line 11,
	// update support structures, do in constructor

    // dictionaries are int keys to int vectors?



}

// // Type load_word_embeddings(fn) {
//     // Type wvb = NULL;
//     if (fn != NULL) {
//         // need c++ equivalent for split
//         bool wvb = fn.split('.')[-1] == 'bin' ? true : false;

//         // try:
//         //     wv = gensim.models.KeyedVectors.load_word2vec_format(fn, binary=wvb, limit=50000)
//         // except AttributeError:
//         //     wv = gensim.models.Word2Vec.load_word2vec_format(fn, binary=wvb, limit=50000)
//     }
//     return wv;
// }

void Lexicon::update_support_structures() {
    compute_pred_to_surface(pred_to_surface);
    reverse_entries = compute_reverse_entries();
    for(int i = 0; i < semantic_forms.size(); i++) {
        sem_form_expected_args.push_back(calc_exp_args(i));
    }

    for(int i = 0; i < semantic_forms.size(); i++) {
        sem_form_return_cat.push_back(calc_return_cat(i));
    } 
    for(int i = 0; i < categories.size(); i++) {
        category_consumes.push_back(find_consumables_for_cat(i));
    } 
    generator_should_flush = true;
}

// pts is a dictionary (pred to surface), each contain vector of ints (sur_idxs) std::map<int, vector<int>>
void Lexicon::compute_pred_to_surface(std::map<int, vector<int>> pts){
    for (int sur_idx = 0; i < entries.size(); i++) {
        for(int sem_idx : entries[sur_idx]) {
            vector<SemanticNode *> to_examine;
            to_examine.push_back(semantic_forms[sem_idx]);
            while(to_examine.size() > 0) {
                SemanticNode *curr = to_examine.pop_back();
                if(!curr.is_lambda){
                    // C++20 now has map.contains().
                    // find might be incorrect for a map (first part of if statement)
                    if(pts.contains(curr.idx_) && !(std::find(pts[curr.idx_].begin(), pts[curr.idx_].end(), sur_idx) != pts[curr.idx_].end())){
                        pts[curr.idx_].push_back(sur_idx);
                    } else if (!pts.contains(curr.idx_)) {
                        vector<int> sur_idx_vec;
                        sur_idx_vec.push_back(sur_idx);
                        pts[curr.idx_] = sur_idx_vec;
                    }
                }
                if (curr.children != NULL) {
                    for (int i = 0; i < curr.children.size(); i++) to_examine.push_back(curr.children[i]);
                }
            }
        }
    }
}

vector<vector<int>> Lexicon::compute_reverse_entries(){
    std::map<int, vector<int>> r; 
    for (int sur_idx = 0; i < surface_forms.size(); i++) {
        for (int sem_idx : entries[sur_idx]) {
            if (r.contains(sem_idx) && !(std::find(r[sem_idx].begin(), r[sem_idx].end(), sur_idx) != r[sem_idx].end())) {
                r[sem_idx].push_back(sur_idx);
            } else {
                vector<int> sur_idx_vec;
                sur_idx_vec.push_back(sur_idx);
                r[sem_idx] = sur_idx_vec;
            }
        }
    }
    for (int sem_idx = 0; sem_idx < semantic_forms.size(); i++) {
        if (!r.contains(sem_idx)) {
            vector<int> empty_vec;
            r[sem_idx] = empty_vec;
        }
    }
    // vec of int vectors?
    vector<vector<int>> r_list;
    for (int i = 0; i < r.size(); i++) {
        if (r.contains(i)) {
            r_list.push_back(r[i]);
        } else {
            vector<int> empty_vec;
            r_list.push_back(empty_vec);
        }
    }
    return r_list;
}

int Lexicon::calc_exp_args(int idx){
    int exp_args = 0;
    int curr_cat = semantic_forms[idx].category_;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        exp_args += 1;
        curr_cat = categories[curr_cat][0];
    }
    return exp_args;
}

int Lexicon::calc_return_cat(int idx){
    int curr_cat = semantic_forms[idx].category_;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        curr_cat = categories[curr_cat][0];
    }
    return curr_cat;
}

// check return type
vector<int> Lexicon::find_consumables_for_cat(int idx) {
    // list of list of int or just string
    std::vector<vector<int>> consumables;
    for (SemanticNode* sem_form : semantic_forms) {
        //boost
        boost::variant<std::string, std::vector<int>> curr = categories[sem_form.category_];
        // while type(curr) is list and type(self.categories[curr[0]]):  what is second part of while loop
        while (curr.type() == typeid(std::vector<int>) && categories[curr[0]].type()) {
            if (curr[0] == idx) {
                break;
            }
            curr = categories[curr[0]];
        } 
        if (curr[0] == idx) {
            vector<int> cons;
            cons.push_back(curr[1]);
            cons.push_back(curr[2]);
            // check this "if cons not in consumables:"
            if (!(std::find(consumables.begin(), consumables.end(), cons) != consumables.end())) {
                consumables.push_back(cons);
            }
        }
    }
    return consumables;
}

// c in self categories?
int Lexicon::get_or_add_category(boost::variant<vector<int>, std::string> c){
    if (!(std::find(categories.begin(), categories.end(), c) != categories.end())) {
        auto it = find(categories.begin(), categories.end(), c); 
        // If element was found 
        if (it != categories.end()) { 
        // calculating the index 
        // of c
            int index = distance(categories.begin(), it); 
            return index;
        }
    }
    categories.push_back(c);
    return categories.size() - 1;
}

string Lexicon::compose_str_from_category(int idx){
    if(idx == NULL) {
        return "NONE IDX";
    }
    if(categories[idx].type() == typeid(std::string)) {
        return categories[idx];
    }
    std::string s = compose_str_from_category(categories[idx][0]);
    if (categories[idx][0].type() != typeid(std::string)) {
        s = "(" + s + ")";
    }
    if(categories[idx][1] == 0) {
        s += "\\";
    } else {
        s += "/";
    }
    std::string s2 = compose_str_from_category(categories[idx][2]);
    if (categories[idx][2].type() != typeid(std::string)) {
        s2 = "(" + s2 + ")";
    }
    return s + s2;
}

// find return type
vector<int> Lexicon::get_semantic_forms_for_surface_form(vector<SemanticNode *> surface_form){
    if (!(std::find(surface_forms.begin(), surface_forms.end(), surface_form) != surface_forms.end())) {
        return vector<int> empty_vec;
    } else {
        int index = 0;
        auto it = find(surface_forms.begin(), surface_forms.end(), surface_form); 
        // If element was found 
        if (it != surface_forms.end()) { 
        // calculating the index 
        // of K 
            index = distance(surface_forms.begin(), it); 
            return entries[index];
        }
        return entries[index];
    }
}

vector<> Lexicon::get_surface_forms_for_predicate(vector<> pred){
    if (pred.type() == typeid(std::string)) {
        
    }
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
SemanticNode* Lexicon::read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, vector<std::string> scoped_lambdas){

}

void Lexicon::delete_semantic_form_for_surface_form(SemanticNode *surface_form, int ont_idx){
    if (!(std::find(surface_forms.begin(), surface_forms.end(), surface_form) != surface_forms.end())) {
        return;
    }
    SemanticNode *matching_semantic_form = NULL;
    for (SemanticNode *semantic_form : semantic_forms) {
        if (semantic_form.idx_ == ont_idx) {
            matching_semantic_form = semantic_form;
            break;
        }
    }
    if (matching_semantic_form == NULL) {
        return;
    }

    int sur_idx = 0;
    auto it = find(surface_forms.begin(), surface_forms.end(), surface_form); 
    // If element was found 
    if (it != surface_forms.end()) { 
    // calculating the index 
    // of K 
        sur_idx = distance(surface_forms.begin(), it); 
    }
    int sem_idx = 0;
    it = find(surface_forms.begin(), surface_forms.end(), matching_semantic_form); 
    // If element was found 
    if (it != surface_forms.end()) { 
    // calculating the index 
    // of K 
        sem_idx = distance(surface_forms.begin(), it); 
    }

    // check if iterator erase equivalent to Python .remove
    if ((std::find(entries.begin(), entries.end(), sur_idx) != entries.end())) {
        if ((std::find(entries[sur_idx].begin(), entries[sur_idx].end(), sem_idx) != entries[sur_idx].end())) {
            entries[sur_idx].remove(entries[sur_idx].begin(), entries[sur_idx].end(), sem_idx);
        }
    }

    if ((std::find(pred_to_surface.begin(), pred_to_surface.end(), ont_idx) != pred_to_surface.end())) {
        if ((std::find(pred_to_surface[ont_idx].begin(), pred_to_surface[ont_idx].end(), sur_idx) != pred_to_surface[ont_idx].end())) {
            // del self.pred_to_surface[sur_idx] ??? dictionary
            it = pred_to_surface.find(sur_idx);             
            pred_to_surface.erase (it);
        }
    }

    if ((std::find(reverse_entries.begin(), reverse_entries.end(), sem_idx) != reverse_entries.end())) {
        if ((std::find(reverse_entries[sem_idx].begin(), reverse_entries[sem_idx].end(), sur_idx) != reverse_entries[sem_idx].end())) {
            reverse_entries.remove(reverse_entries.begin(), reverse_entries.end(), sur_idx);
        }
    }
}