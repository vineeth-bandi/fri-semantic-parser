#include "Lexicon.h"

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
                    if(pts.contains(curr.idx) && !(std::find(pts[curr.idx].begin(), pts[curr.idx].end(), sur_idx) != pts[curr.idx].end())){
                        pts[curr.idx].push_back(sur_idx);
                    } else if (!pts.contains(curr.idx)) {
                        vector<int> sur_idx_vec;
                        sur_idx_vec.push_back(sur_idx);
                        pts[curr.idx] = sur_idx_vec;
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

// check return type
vector<int> Lexicon::find_consumables_for_cat(int idx) {
    // list of list of int or just string
    std::vector<vector<int>> consumables;
    for (SemanticNode* sem_form : semantic_forms) {
        //boost
        boost::variant<std::string, std::vector<int>> curr = categories[sem_form.category];
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


vector<int> Lexicon::get_surface_forms_for_predicate(boost::variant<std::string, int> pred){
    if (pred.type() == typeid(std::string)) {
        std::vector<std::string>::iterator it;
        if (std::find(ontology -> preds_.begin(), ontology -> preds_.end(), pred) != ontology -> preds_.end()){
            return pred_to_surface[std::distance(ontology -> preds_.begin(), it)]
        }
    }
    else{
        if (pred_to_surface.find(pred) != pred_to_surface.end()){
            return pred_to_surface[pred];
        }
    }
    return std::vector<int>();
}

// custom methods: readfile and strip
bool readFile(std::string fileName, std::vector<std::string>&fileVec){
    std::ifstream in(fileName.c_str());
    if (!in){
        return false;
    }
    std::string line;
    while (std::getLine(in, line)){
        if (line.size() > 0){
            fileVec.push_back(line);
        }
    }
    in.close();
    return true;
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

std::vector<std::string> split(std::string in, std::string delimiter){
    std::string input = in;
    std::vector<std::string> lhs;
    size_t pos = 0;
    std::string token;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        token = input.substr(0, pos);
        lhs.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    return lhs;
}

// three diff returns
void Lexicon::read_lex_from_file(std::string fname){
    surface_forms = std::vector<std::string>();
    semantic_forms = std::vector<SemanticNode *>();
    entries = std::vector<vector<int>> entries();
    pred_to_surface = std::map<int, vector<int>>();
    std::vector<std::string> fileVec; 
    readFile(fname, fileVec)
    expand_lex_from_strs(filVec);
}


void Lexicon::expand_lex_from_strs(std::vector<std::string> lines){
    for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        std::string line = *it;
        line = strip(line);
        if (line.length() == 0 || line[0] == '#') continue;

        //split into two
        std::vector<std::string> lineParts(split(line, " :- ")); 
        std::string lhs = lineParts.begin();
        std::string rhs = lineParts.end();
        std::string surface_form = strip(lhs);

        std::vector<std::string>::iterator itr = find(surface_forms.begin(), surface_forms.end(), surface_form);
        size_t sur_idx;
        if(itr != surface_forms.end()){
            sur_idx = (size_t)std::distance(surface_forms.begin(), itr);
        }
        else{
            sur_idx = surface_forms.size();
            surface_forms.push_back(surface_form);
            entries.append(std::vector<int>());
        }

        std::vector<boost::variant<int, SemanticNode*>> ret(read_syn_sem(rhs));
        int cat_idx = ret.begin();
        SemanticNode* semantic_form = ret.end();

        std::vector<SemanticNode*>::iterator semItr = find(semantic_forms.begin(), semantic_forms.end(), semantic_form);
        size_t sem_idx;
        if(semItr != semantic_forms.end()){
            sem_idx = (size_t)std::distance(semantic_forms.begin(), semItr);
        }
        else{
            sem_idx = semantic_forms.size();
            semantice_forms.push_back(semantic_form);
        }
        entries[sur_idx].push_back(sem_idx);
        vector<int> preds_in_semantic_form(get_all_preds_from_semantic_form(semantic_forms[sem_idx]));
        for (std::vector<int>::iterator pred = preds_in_semantic_form.begin(); pred != preds_in_semantic_form.end(); ++pred){
            if (pred_to_surface.find(*pred) != pred_to_surface.end()){
                pred_to_surface[*pred].push_back(sur_idx);
            }
            else{
                pred_to_surface[*pred] = std::vector<int>{sur_idx};
            }
        }
    }
}

// two returns, returns as boost vec of int and semanticnode* (indexes 0 and 1)
std::vector<boost::variant<int, SemanticNode *>> Lexicon::read_syn_sem(std::string s){
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


vector<int> Lexicon::get_all_preds_from_semantic_form(SemanticNode* node){

}

bool Lexicon::form_contains_DESC_predicate(SemanticNode* node){

}

int Lexicon::read_category_from_str(std::string s){

}


// new param bool? 
SemanticNode* Lexicon::read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, vector<std::string> scoped_lambdas, bool allow_expanding_ont){

}

void Lexicon::delete_semantic_form_for_surface_form(SemanticNode *surface_form, int ont_idx){
    if (!(std::find(surface_forms.begin(), surface_forms.end(), surface_form) != surface_forms.end())) {
        return;
    }
    SemanticNode *matching_semantic_form = NULL;
    for (SemanticNode *semantic_form : semantic_forms) {
        if (semantic_form.idx == ont_idx) {
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