#include "Lexicon.h"

Lexicon::Lexicon(Ontology* ontology, std::string lexicon_fname, std::string word_embeddings_fn){
	this.ontology = ontology;
    surface_forms = 
    semantic_forms = ;
    entries = ;
    pred_to_surface = ;
    read_lex_from_file(lexicon_fname);
    reverse_entries = ;
    neighbor_surface_forms = ;
    //sem_form_expected_args = NULL;
    //sem_form_return_cat = NULL;
    //category_consumes = NULL;
    generator_should_flush = false;
    update_support_structures();
    wv = load_word_embeddings(word_embeddings_fn);
}

Type load_word_embeddings(std::string fn) {
    // Type wvb = NULL;
    if (fn != NULL) {
        // need c++ equivalent for split
        bool wvb = fn.split('.')[-1] == 'bin' ? true : false;

        // try:
        //     wv = gensim.models.KeyedVectors.load_word2vec_format(fn, binary=wvb, limit=50000)
        // except AttributeError:
        //     wv = gensim.models.Word2Vec.load_word2vec_format(fn, binary=wvb, limit=50000)
    }
    return wv;
}

vector<> Lexicon::get_lexicon_word_embedding_neighbors(w, int n) {
    if () {

    }
    candidate_neighbors;
    pred_cosine;
    if () {

    }
    vector<> top_k_sims;

    while (top_k_sims.size() < n && top_k_sims.size() < candidate_neighbors.size()) {
        int curr_max_val;
        top_k_sims.push_back();
    }
    return top_k_sims;
}

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
void Lexicon::compute_pred_to_surface(std::map<int, std::vector<int>> pts){
    for (int sur_idx = 0; i < entries.size(); i++) {
        for(int sem_idx : entries[sur_idx]) {
            vector<SemanticNode *> to_examine;
            to_examine.push_back(semantic_forms[sem_idx]);
            while(to_examine.size() > 0) {
                SemanticNode *curr = to_examine.pop_back();
                if(!curr->is_lambda_){
                    // C++20 now has map.contains().
                    // find might be incorrect for a map (first part of if statement)
                    if(pts.contains(curr->idx_) && !(std::find(pts[curr->idx_].begin(), pts[curr->idx_].end(), sur_idx) != pts[curr->idx_].end())){
                        pts[curr->idx_].push_back(sur_idx);
                    } else if (!pts.contains(curr->idx_)) {
                        std::vector<int> sur_idx_vec;
                        sur_idx_vec.push_back(sur_idx);
                        pts[curr->idx_] = sur_idx_vec;
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
    std::map<int, std::vector<int>> r; 
    for (int sur_idx = 0; i < surface_forms.size(); i++) {
        for (int sem_idx : entries[sur_idx]) {
            if (r.contains(sem_idx) && !(std::find(r[sem_idx].begin(), r[sem_idx].end(), sur_idx) != r[sem_idx].end())) {
                r[sem_idx].push_back(sur_idx);
            } else {
                std::vector<int> sur_idx_vec;
                sur_idx_vec.push_back(sur_idx);
                r[sem_idx] = sur_idx_vec;
            }
        }
    }
    for (int sem_idx = 0; sem_idx < semantic_forms.size(); i++) {
        if (!r.contains(sem_idx)) {
            std::vector<int> empty_vec;
            r[sem_idx] = empty_vec;
        }
    }
    // vec of int vectors?
    std::vector<std::vector<int>> r_list;
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
    int curr_cat = semantic_forms[idx]->category_;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        exp_args += 1;
        curr_cat = categories[curr_cat][0];
    }
    return exp_args;
}

int Lexicon::calc_return_cat(int idx){
    int curr_cat = semantic_forms[idx]->category_;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        curr_cat = categories[curr_cat][0];
    }
    return curr_cat;
}

// check return type
vector<int> Lexicon::find_consumables_for_cat(int idx) {
    // list of list of int or just string
    std::vector<std::vector<int>> consumables;
    for (SemanticNode* sem_form : semantic_forms) {
        //boost
        boost::variant<std::string, std::vector<int>> curr = categories[sem_form->category_];
        // while type(curr) is list and type(self.categories[curr[0]]):  what is second part of while loop
        while (curr.type() == typeid(std::vector<int>) && categories[curr[0]].type()) {
            if (curr[0] == idx) {
                break;
            }
            curr = categories[curr[0]];
        } 
        if (curr[0] == idx) {
            std::vector<int> cons;
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
int Lexicon::get_or_add_category(boost::variant<std::vector<int>, std::string> c){
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
        return std::vector<int> empty_vec;
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

// not sure, no c equivalent for python strip, change to std::string?
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

// rewrite split to be vector not just lhs
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
    entries = std::vector<std::vector<int>> entries();
    pred_to_surface = std::map<int, vector<int>>();
    std::vector<std::string> fileVec; 
    readFile(fname, fileVec)
    expand_lex_from_strs(filVec);
}

// check
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
        std::vector<int> preds_in_semantic_form = get_all_preds_from_semantic_form(semantic_forms[sem_idx]);
        // for (std::vector<int>::iterator pred = preds_in_semantic_form.begin(); pred != preds_in_semantic_form.end(); ++pred){
        //     if (pred_to_surface.find(*pred) != pred_to_surface.end()){
        //         pred_to_surface[*pred].push_back(sur_idx);
        //     }
        //     else{
        //         pred_to_surface[*pred] = std::vector<int>{sur_idx};
        //     }
        // }

        // might've fixed.
        for (int pred : preds_in_semantic_form) {
            if ((std::find(pred_to_surface.begin(), pred_to_surface.end(), pred) != pred_to_surface.end())) {
                pred_to_surface[pred].push_back(sur_idx);
            } else {
                pred_to_surface[pred] = std::vector<int>{sur_idx};
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
    std::vector<int> node_preds;
    if (!node->is_lambda_) {
        node_preds.push_back(node->idx_);
    }
    if (node->children_ == NULL) {
        return node_preds;
    }
    for (SemanticNode *c : node->children_) {
        std::vector<int> extend = get_all_preds_from_semantic_form(c);
        node_preds.insert(node_preds.end(), extend.begin(), extend.end());
    }
    return node_preds;
}

// check try catch statement valueerror??
int Lexicon::read_category_from_str(std::string s){
    int p;
    int i;
    if (s[0] == "(") {
        p = 1;
        for (i = 1; i < s.length() - 1; i++) {
            if (s[i] == "(") {
                p += 1;
            } else if (s[i] == ")") {
                p -= 1;
            }
            if (p == 0) {
                break;
            }
        }
        if (i == s.length() - 2 && p == 1 && s[s.length() - 1] == ")") {
            s = s.substr(1, s.length() - 2);
        }
    }
    p = 0;
    int fin_slash_idx = s.length() - 1;
    int direction;
    while (fin_slash_idx >= 0) {
        if (s[fin_slash_idx] == ")") {
            p += 1;
        } else if (s[fin_slash_idx] == "(") {
            p -= 1;
        } else if (p == 0) {
            if (s[fin_slash_idx] == "/") {
                direction = 1;
                break;
            } else if (s[fin_slash_idx] == "\\") {
                direction = 0;
                break;
            }
        }
        fin_slash_idx -= 1;
    }
    if (fin_slash_idx > 0) {
        int output_category_idx = read_category_from_str(s.substr(0, fin_slash_idx));
        int input_category_idx = read_category_from_str(s.substr(fin_slash_idx + 1, s.length() - (fin_slash_idx + 1)));
        boost::variant<std::string, std::vector<int>> category;
        category.push_back(output_category_idx);
        category.push_back(direction);
        category.push_back(input_category_idx);
    } else {
        if (s.find("(") != std::string::npos || s.find(")") != std::string::npos || s.find("\\") != std::string::npos) {
            std::cout << "Invalid atomic category '" << s << "'";
            exit (EXIT_FAILURE);
        }
        category = s;
    }
    try {
        int index;
        auto it = find(categories.begin(), categories.end(), category); 
        // If element was found 
        if (it != categories.end()) { 
            index = distance(categories.begin(), it); 
        }
        idx = index;
    } catch () {
        idx = categories.size();
        categories.push_back(category);
    }
    return idx;
}

SemanticNode* Lexicon::read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, vector<std::string> scoped_lambdas){
    s = s.strip();
    SemanticNode *node;
    std::string str_remaining;
    if(s.substr(0, 6) == "lambda") {
        std::vector<std::string> str_parts = split(strip(s.substr(6, s.length() - 6)), ".");
        string info = str_parts[0];
        std::vector<std::string> name_type = split(info, ":");
        string name = name_type[0];
        string type_str = name_type[1];
        scoped_lambdas.push_back(name);
        int name_idx = scoped_lambdas.size();
        int t = ontology->read_type_from_str(type_str);
        node = SemanticNode(parent, t, category, true, name_idx, true, NULL);
        for(int i = 1; i < str_parts.size(); i++) {
            str_remaining += str_parts[i];
            if (i < str_parts.size() - 1) {
                str_remaining += ".";
            }
        }
        str_remaining = str_remaining.substr(1, str_remaining.length() - 2);
    } else {
        int end_of_pred = 1;
        while (end_of_pred < s.length()) {
            if (s[end_of_pred] == "(") {
                break;
            }
            end_of_pred += 1;
        }
        string pred = s.substr(0, end_of_pred);

        SemanticNode *curr = parent;
        bool is_scoped_lambda = false;
        int pred_idx;
        while (curr != NULL && !is_scoped_lambda) {
            try {   
                pred_idx = scoped_lambdas + 1;
            } catch() {
                pred_idx = NULL;
            }
        }









    }

}

SemanticNode *Lexicon::instantiate_wild_type(SemanticNode *root){
    bool debug = false;

    int index;
    auto it = find(ontology->preds_.begin(), ontology->preds_.end(), "and"); 
    // If element was found 
    if (it != ontology->preds_.end()) { 
    // calculating the index 
    // of K 
        index = distance(ontology->preds_.begin(), it); 
    }

    if (root->idx_ == index) {
        std::string crta = ontology->compose_str_from_type(root->children_[0]->return_type_);
        std::string crtb = ontology->compose_str_from_type(root->children_[1]->return_type_);
        if (crta  != crtb) {
            std::cout << "ERROR: 'and' taking children of different return types " << crta << ", " << crtb;
            exit (EXIT_FAILURE);
        }
        root->type_ = ontology->read_type_from_str("<" + crta + ",<" + crta + "," + crta + ">>");
    }
    if (root->children_ != NULL) {
        for (int cidx = 0; cidx < root->children_.size(); cidx++) {
            root->children_[cidx] = instantiate_wild_type(root->children_[cidx]);
        }
    }
    return root;
}

void Lexicon::delete_semantic_form_for_surface_form(SemanticNode *surface_form, int ont_idx){
    if (!(std::find(surface_forms.begin(), surface_forms.end(), surface_form) != surface_forms.end())) {
        return;
    }
    SemanticNode *matching_semantic_form = NULL;
    for (SemanticNode *semantic_form : semantic_forms) {
        if (semantic_form->idx_ == ont_idx) {
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