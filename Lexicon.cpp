#include "Lexicon.h"
using namespace std;
Lexicon::Lexicon(Ontology* onto, std::string lexicon_fname, std::string word_embeddings_fn, std::string vocab_fn): ontology(onto){
    // surface_forms = 
    // semantic_forms = ;
    // entries = ;
    // pred_to_surface = ;
    read_lex_from_file(lexicon_fname);
    // reverse_entries = ;
    // neighbor_surface_forms = ;
    //sem_form_expected_args = NULL;
    //sem_form_return_cat = NULL;
    //category_consumes = NULL;
    generator_should_flush = false;
    update_support_structures();
    load_word_embeddings(word_embeddings_fn, vocab_fn);
}
// custom methods: readfile and strip
bool readFile(std::string fileName, std::vector<std::string>&fileVec){
    std::ifstream in(fileName.c_str());
    if (!in){
        return false;
    }
    std::string line;
    while (std::getline(in, line)){
        if (line.size() > 0){
            fileVec.push_back(line);
        }
    }
    in.close();
    return true;
}

std::string strip(std::string s) {
    boost::trim(s);
    return s;
}

std::vector<std::string> split(std::string str, std::string delimiter){
    std::string s = std::string(str);
    size_t pos = 0;
    std::string token;
    std::vector<std::string> split_string = std::vector<std::string>();
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        split_string.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    split_string.push_back(s);
    return split_string;
}
void Lexicon::load_word_embeddings(std::string fn, std::string fn2)
{
   if (fn != "" && fn2 != "")
   {
      std::ifstream in(fn);
      std::string line;
      int rows = 50000;
      int cols = 300;
      int row = 0;
      int col = 0;
      Eigen::MatrixXd res = Eigen::MatrixXd(rows, cols);
      if (in.is_open())
      {
         while (std::getline(in, line))
         {

            char *ptr = (char *)line.c_str();
            int len = line.length();

            col = 0;

            char *start = ptr;
            for (int i = 0; i < len; i++)
            {

               if (ptr[i] == ',')
               {
                  res(row, col++) = atof(start);
                  start = ptr + i + 1;
               }
            }
            res(row, col) = atof(start);
            row++;
         }
         in.close();
      }
      wv = res;

   in = std::ifstream(fn2);
   std::unordered_map<std::string, int> dict = std::unordered_map<std::string, int>();
   if (in.is_open())
   {
      while ((std::getline(in, line)))
      {
         char *saveptr = NULL;
         char *ptr = (char *)line.c_str();
         int len = line.length();
         char *key = strtok_r(ptr, ",", &saveptr);
         int val = atoi(strtok_r(NULL, ",", &saveptr));
         dict[std::string(key)] = val;
      }
   }
   vocab = dict;
   }
}

std::vector<std::tuple<int, double>> Lexicon::get_lexicon_word_embedding_neighbors(std::string w, int n) {
    if (vocab.find(w) == vocab.end()) {
       return std::vector<std::tuple<int, double>>();
    }
    std::vector<int> candidate_neighbors = std::vector<int>();
    for(int sfidx =0; sfidx< surface_forms.size(); sfidx++){
       if(std::find(neighbor_surface_forms.begin(), neighbor_surface_forms.end(), sfidx) == neighbor_surface_forms.end()){
          candidate_neighbors.push_back(sfidx);
       }
    }
    bool found = false;
     std::vector<double> pred_cosine = std::vector<double>();
     int w_idx = vocab[w];
     double w_dist = sqrt((wv.row(w_idx) * wv.row(w_idx).transpose())(0));
     for(int vidx : candidate_neighbors){
        double sim = 0;
        std::string candidate = surface_forms[vidx];
        if(vocab.find(candidate) != vocab.end()){
           int candidate_idx = vocab[candidate];
           double candidate_dist =  sqrt((wv.row(candidate_idx) * wv.row(candidate_idx).transpose())(0));
           sim = ((wv.row(w_idx) * wv.row(candidate_idx).transpose())(0))/(w_dist * candidate_dist);
           sim = abs(sim);
           sim = (sim + 1.0)/2.0;
        }
        pred_cosine.push_back(sim);
     }
    double max_prob = 0;
    for(double prob : pred_cosine)
        max_prob = max_prob > prob ? max_prob : prob;
    if(max_prob == 0)
        return std::vector<std::tuple<int, double>>();
    std::vector<std::tuple<int, double>> max_sims = std::vector<std::tuple<int, double>> ();
    for(int i =0; i < pred_cosine.size(); i++){
        double x = pred_cosine[i];
        if(std::abs(x - max_prob) <= (1e-08 + 1e-05 * std::abs(max_prob)))
            max_sims.push_back(std::tuple<int, double>(i, x));
    }
    std::vector<std::tuple<int, double>> top_k_sims(max_sims);

    while (top_k_sims.size() < n && top_k_sims.size() < candidate_neighbors.size()) {
        double curr_max_val = 0;
        for(int sidx =0; sidx< candidate_neighbors.size(); sidx++){
            bool found = false;
            for(std::tuple<int, double> x : top_k_sims)
                found = std::get<0>(x) == sidx;
            if(!found)
                curr_max_val = curr_max_val >= pred_cosine[sidx] ? curr_max_val : pred_cosine[sidx];
        }
        for(int i =0; i < pred_cosine.size(); i++){
            double x = pred_cosine[i];
            if(std::abs(x - curr_max_val) <= (1e-08 + 1e-05 * std::abs(curr_max_val)))
                top_k_sims.push_back(std::tuple<int, double>(i, x));
        }
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

// pts is a dictionary (pred to surface), each contain vector of ints (sur_idxs) std::unordered_map<int, vector<int>>
void Lexicon::compute_pred_to_surface(std::unordered_map<int, std::vector<int>> pts){
    for (int sur_idx = 0; sur_idx < entries.size(); sur_idx++) {
        for(int sem_idx : entries[sur_idx]) {
            std::vector<SemanticNode *> to_examine;
            to_examine.push_back(semantic_forms[sem_idx]);
            while(to_examine.size() > 0) {
                SemanticNode *curr = to_examine.back();
                to_examine.pop_back();
                if(!curr->is_lambda_){
                    // C++20 now has unordered_map.contains(). prob best to use unordered_map find
                    // find might be incorrect for a unordered_map (first part of if statement)
                    if(pts.find(curr->idx_) != pts.end() && !(std::find(pts[curr->idx_].begin(), pts[curr->idx_].end(), sur_idx) != pts[curr->idx_].end())){
                        pts[curr->idx_].push_back(sur_idx);
                    } else if (pts.find(curr->idx_) == pts.end()) {
                        std::vector<int> sur_idx_vec;
                        sur_idx_vec.push_back(sur_idx);
                        pts[curr->idx_] = sur_idx_vec;
                    }
                }
                    for (int i = 0; i < curr->children_.size(); i++) 
                        to_examine.push_back(curr->children_[i]);
            }
        }
    }
}

std::vector<std::vector<int>> Lexicon::compute_reverse_entries(){
    std::unordered_map<int, std::vector<int>> r; 
    for (int sur_idx = 0; sur_idx < surface_forms.size(); sur_idx++) {
        for (int sem_idx : entries[sur_idx]) {
            if (r.find(sem_idx) != r.end() && !(std::find(r[sem_idx].begin(), r[sem_idx].end(), sur_idx) != r[sem_idx].end())) {
                r[sem_idx].push_back(sur_idx);
            } else {
                std::vector<int> sur_idx_vec;
                sur_idx_vec.push_back(sur_idx);
                r[sem_idx] = sur_idx_vec;
            }
        }
    }
    for (int sem_idx = 0; sem_idx < semantic_forms.size(); sem_idx++) {
        if (r.find(sem_idx) == r.end()) {
            std::vector<int> empty_vec;
            r[sem_idx] = empty_vec;
        }
    }
    // vec of int vectors?
    std::vector<std::vector<int>> r_list;
    for (int i = 0; i < r.size(); i++) {
        if (r.find(i) != r.end()) {
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
        std::vector<int> temp = boost::get<std::vector<int>>(categories[curr_cat]);
        exp_args += 1;
        curr_cat = temp[0];
    }
    return exp_args;
}

int Lexicon::calc_return_cat(int idx){
    int curr_cat = semantic_forms[idx]->category_;
    while (categories[curr_cat].type() == typeid(std::vector<int>)) {
        std::vector<int> temp = boost::get<std::vector<int>>(categories[curr_cat]);
        curr_cat = temp[0];
    }
    return curr_cat;
}

// check return type
std::vector<std::tuple<int, int>> Lexicon::find_consumables_for_cat(int idx) {
    // list of list of int or just string
    std::vector<std::tuple<int, int>> consumables;
    for (SemanticNode* sem_form : semantic_forms) {
        //boost
        boost::variant<std::string, std::vector<int>> curr = categories[sem_form->category_];
        // while type(curr) is list and type(self.categories[curr[0]]):  what is second part of while loop
        while (curr.type() == typeid(std::vector<int>)) {
            std::vector<int> temp = boost::get<std::vector<int>>(curr);
            if (temp[0] == idx) {
                break;
            }
            curr = categories[temp[0]];
        } 
        if (curr.type() == typeid(std::vector<int>)) {
            std::vector<int> temp = boost::get<std::vector<int>>(curr);
            if(temp[0] == idx){
                std::tuple<int, int> cons = std::tuple<int, int>(temp[1], temp[2]);
                // check this "if cons not in consumables:"
                if (!(std::find(consumables.begin(), consumables.end(), cons) != consumables.end())) {
                    consumables.push_back(cons);
                }
            }
        }
    }
    return consumables;
}

// c in self categories?
int Lexicon::get_or_add_category(boost::variant<std::vector<int>, std::string> c){
    if (!(std::find(categories.begin(), categories.end(), boost::variant<std::string, std::vector<int>>(c)) != categories.end())) {
        auto it = find(categories.begin(), categories.end(), boost::variant<std::string, std::vector<int>>(c)); 
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

std::string Lexicon::compose_str_from_category(int idx){
    if(categories[idx].type() == typeid(std::string)) {
        return boost::get<std::string>(categories[idx]);
    }
    std::vector<int> temp = boost::get<std::vector<int>>(categories[idx]);
    std::string s = compose_str_from_category(temp[0]);
    if (categories[temp[0]].type() != typeid(std::string)) {
        s = "(" + s + ")";
    }
    if(temp[1] == 0) {
        s += "\\";
    } else {
        s += "/";
    }
    std::string s2 = compose_str_from_category(temp[2]);
    if (categories[temp[2]].type() != typeid(std::string)) {
        s2 = "(" + s2 + ")";
    }
    return s + s2;
}

// find return type
std::vector<int> Lexicon::get_semantic_forms_for_surface_form(std::string surface_form){
    if (!(std::find(surface_forms.begin(), surface_forms.end(), surface_form) != surface_forms.end())) {
        return std::vector<int>();
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


std::vector<int> Lexicon::get_surface_forms_for_predicate(boost::variant<std::string, int> pred){
    if (pred.type() == typeid(std::string)) {
        typesBoost temp = boost::get<std::string>(pred);
        int indx = ontology->find_index(temp);
        if ( indx != -1){
            return pred_to_surface[indx];
        }
    }
    else{
        if (pred_to_surface.find(boost::get<int>(pred)) != pred_to_surface.end()){
            
            return pred_to_surface[boost::get<int>(pred)];
        }
    }
    return std::vector<int>();
}


// three diff returns
void Lexicon::read_lex_from_file(std::string fname){
    surface_forms = std::vector<std::string>();
    semantic_forms = std::vector<SemanticNode *>();
    entries = std::vector<std::vector<int>>();
    pred_to_surface = std::unordered_map<int, vector<int>>();
    std::vector<std::string> fileVec; 
    readFile(fname, fileVec);
    expand_lex_from_strs(fileVec);
}

// check
void Lexicon::expand_lex_from_strs(std::vector<std::string> lines){
    for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        std::string line = *it;
        line = strip(line);
        if (line.length() == 0 || line[0] == '#') continue;
        //split into two
        std::vector<std::string> lineParts(split(line, " :- ")); 
        std::string lhs = lineParts[0];
        std::string rhs = lineParts[1];
        std::string surface_form = strip(lhs);
        auto itr = find(surface_forms.begin(), surface_forms.end(), surface_form);
        size_t sur_idx;
        if(itr != surface_forms.end()){
            sur_idx = (size_t)std::distance(surface_forms.begin(), itr);
        }
        else{
            sur_idx = surface_forms.size();
            surface_forms.push_back(surface_form);
            entries.push_back(std::vector<int>());
        }
        std::vector<boost::variant<int, SemanticNode*>> ret(read_syn_sem(rhs));
        int cat_idx = boost::get<int>(ret[0]);
        SemanticNode* semantic_form = boost::get<SemanticNode *>(ret[1]);

        std::vector<SemanticNode*>::iterator semItr = find(semantic_forms.begin(), semantic_forms.end(), semantic_form);
        size_t sem_idx;
        if(semItr != semantic_forms.end()){
            sem_idx = (size_t)std::distance(semantic_forms.begin(), semItr);
        }
        else{
            sem_idx = semantic_forms.size();
            semantic_forms.push_back(semantic_form);
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

        // another version.
        for (int pred : preds_in_semantic_form) {
            if (pred_to_surface.find(pred) != pred_to_surface.end()) {
                pred_to_surface[pred].push_back(sur_idx);
            } else {
                pred_to_surface[pred] = std::vector<int>{(int)sur_idx};
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
    std::vector<std::string> split_string = split(str, " : ");
    lhs = split_string[0];
    rhs = split_string[1];
    int cat_idx = read_category_from_str(strip(lhs));
    std::vector<std::string> scoped;
    SemanticNode *semantic_form = read_semantic_form_from_str(strip(rhs), cat_idx, NULL, scoped);
    std::vector<boost::variant<int, SemanticNode *>> returns;
    returns.push_back(cat_idx);
    returns.push_back(semantic_form);
    return returns;
}


std::vector<int> Lexicon::get_all_preds_from_semantic_form(SemanticNode* node){
    std::vector<int> node_preds;
    if (!node->is_lambda_) {
        node_preds.push_back(node->idx_);
    }
    if (node->children_.size() == 0) {
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
    if (s[0] == '(') {
        p = 1;
        for (i = 1; i < s.length() - 1; i++) {
            if (s[i] == '(') {
                p += 1;
            } else if (s[i] == ')') {
                p -= 1;
            }
            if (p == 0) {
                break;
            }
        }
        if (i == s.length() - 1 && p == 1 && s[s.length() - 1] == ')') {
            s = s.substr(1, s.length() - 2);
        }
    }
    p = 0;
    int fin_slash_idx = s.length() - 1;
    int direction;
    while (fin_slash_idx >= 0) {
        if (s[fin_slash_idx] == ')') {
            p += 1;
        } else if (s[fin_slash_idx] == '(') {
            p -= 1;
        } else if (p == 0) {
            if (s[fin_slash_idx] == '/') {
                direction = 1;
                break;
            } else if (s[fin_slash_idx] == '\\') {
                direction = 0;
                break;
            }
        }
        fin_slash_idx -= 1;
    }
    boost::variant<std::string, std::vector<int>> category;
    if (fin_slash_idx > 0) {
        int output_category_idx = read_category_from_str(s.substr(0, fin_slash_idx));
        int input_category_idx = read_category_from_str(s.substr(fin_slash_idx + 1, s.length() - (fin_slash_idx + 1)));
        std::vector<int> temp;
        temp.push_back(output_category_idx);
        temp.push_back(direction);
        temp.push_back(input_category_idx);
        category = boost::variant<std::string, std::vector<int>>(temp);
    } else {
        if (s.find("(") != std::string::npos || s.find(")") != std::string::npos || s.find("\\") != std::string::npos) {
            std::cout << "Invalid atomic category '" << s << "'";
            exit (EXIT_FAILURE);
        }
        category = boost::variant<std::string, std::vector<int>>(s);
    }
    int idx;
    auto it = find(categories.begin(), categories.end(), category); 
    // If element was found 
    if (it != categories.end()) { 
        idx = distance(categories.begin(), it);
    } else {
        idx = categories.size();
        categories.push_back(category);
    }
    return idx;
}

SemanticNode* Lexicon::read_semantic_form_from_str(std::string s, int category, SemanticNode *parent, std::vector<std::string> scoped_lambdas){
    s = strip(s);
    SemanticNode *node;
    std::string str_remaining;
    bool is_scoped_lambda = false;
    if(s.substr(0, 6) == "lambda") {
        std::vector<std::string> str_parts = split(strip(s.substr(6, s.length() - 6)), ".");
        std::string info = str_parts[0];
        std::vector<std::string> name_type = split(info, ":");
        std::string name = name_type[0];
        std::string type_str = name_type[1];
        scoped_lambdas.push_back(name);
        int name_idx = scoped_lambdas.size();
        int t = ontology->read_type_from_str(type_str);
        node = new SemanticNode(parent, t, category, name_idx, true, std::vector<SemanticNode *>());
        for(int i = 1; i < str_parts.size(); i++) {
            str_remaining += str_parts[i];
            if (i < str_parts.size() - 1) {
                str_remaining += '.';
            }
        }
        str_remaining = str_remaining.substr(1, str_remaining.length() - 2);
    } else {
        int end_of_pred = 1;
        while (end_of_pred < s.length()) {
            if (s[end_of_pred] == '(') {
                break;
            }
            end_of_pred += 1;
        }
        std::string pred = s.substr(0, end_of_pred);

        SemanticNode *curr = parent;
        is_scoped_lambda = false;
        int pred_idx;
        while (curr != NULL && !is_scoped_lambda) {
            auto it = find(scoped_lambdas.begin(), scoped_lambdas.end(), pred); 
            // If element was found 
            if (it != scoped_lambdas.end()) { 
                pred_idx = distance(scoped_lambdas.begin(), it) + 1;
                if (curr->is_lambda_ && curr->lambda_name_ == pred_idx)
                    is_scoped_lambda = true;
            } else {
                pred_idx = -1;
                is_scoped_lambda = false;
            }
            if (is_scoped_lambda) {
                break;
            }
            curr = curr->parent_;
        }
        if (is_scoped_lambda) {
            node = new SemanticNode(parent, curr->type_, -1, curr->lambda_name_, false);
        } else {
            auto it = find(ontology->preds_.begin(), ontology->preds_.end(), pred); 
            // If element was found 
            if (it != ontology->preds_.end()) { 
                pred_idx = distance(ontology->preds_.begin(), it);
            } else {
                std::cout << "Symbol not found within ontology or lambdas in scope: '" << pred << "'";
                exit (EXIT_FAILURE);
            }
            node = new SemanticNode(parent, ontology->entries_[pred_idx], category, pred_idx);
        }
        if(end_of_pred < s.length()-1){
            str_remaining = s.substr(end_of_pred + 1, s.length() - (end_of_pred + 2));
        }
        else
        {
            str_remaining = "";
        }
    }
    if (str_remaining.length() > 0) {
        std::vector<int> delineating_comma_idxs;
        int p = 0;
        int d = 0;
        for (int i = 0; i < str_remaining.length(); i++) {
            if (str_remaining[i] == '(') {
                p += 1;
            }
            else if (str_remaining[i] == ')') {
                p -= 1;
            }
            else if (str_remaining[i] == '<') {
                d += 1;
            }
            else if (str_remaining[i] == '>') {
                d -= 1;
            }
            else if (str_remaining[i] == ',' && p == 0 && d == 0) {
                delineating_comma_idxs.push_back(i);
            }
        }
        std::vector<SemanticNode *> children;
        std::vector<int> splits;
        splits.push_back(-1);
        splits.insert(splits.end(), delineating_comma_idxs.begin(), delineating_comma_idxs.end());
        splits.push_back(str_remaining.length());
        std::vector<int> expected_child_cats;
        int curr_cat = category;
        while (curr_cat != -1 && categories[curr_cat].type() == typeid(std::vector<int>)) {
            curr_cat = boost::get<std::vector<int>>(categories[curr_cat])[0];
            expected_child_cats.push_back(curr_cat);
        }
        for (int i = 1; i < splits.size(); i++) {
            int e_cat;
            if (expected_child_cats.size() >= i) {
                e_cat = expected_child_cats[i - 1];
            } else {
                e_cat = -1;
            }
            children.push_back(read_semantic_form_from_str(str_remaining.substr(splits[i - 1] + 1, splits[i] - (splits[i-1] + 1)), e_cat, node, scoped_lambdas));
        }
        node->children_ = children;
    }
    
        // try:
        //     node.set_return_type(self.ontology)
        // except TypeError as e:
        //     print(e)
        //     sys.exit("Offending string: '" + s + "'")


    node->set_return_type(*ontology);
    if (!node->validate_tree_structure()) {
        std::cout << "ERROR: read in invalidly linked semantic node from string '" << s << "'";
        exit (EXIT_FAILURE);
    }
    node = instantiate_wild_type(node);
    return node;

}

SemanticNode *Lexicon::instantiate_wild_type(SemanticNode *root){
    bool debug = false;

    int index = -1;
    auto it = find(ontology->preds_.begin(), ontology->preds_.end(), std::string("and")); 
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
    if (root->children_.size() != 0) {
        for (int cidx = 0; cidx < root->children_.size(); cidx++) {
            root->children_[cidx] = instantiate_wild_type(root->children_[cidx]);
        }
    }
    return root;
}

/* void Lexicon::delete_semantic_form_for_surface_form(std::string surface_form, int ont_idx){
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
    sur_idx = distance(surface_forms.begin(), it); 
    int sem_idx = 0;
    auto itr = find(semantic_forms.begin(), semantic_forms.end(), matching_semantic_form); 
    sem_idx = distance(semantic_forms.begin(), itr); 

    // check if iterator erase equivalent to Python .remove
    if ((std::find(entries.begin(), entries.end(), sur_idx) != entries.end())) {
        if (std::find(entries[sur_idx].begin(), entries[sur_idx].end(), sem_idx) != entries[sur_idx].end()) {
            entries[sur_idx].erase(std::find(entries[sur_idx].begin(), entries[sur_idx].end(), sem_idx));
        }
    }

    if ((std::find(pred_to_surface.begin(), pred_to_surface.end(), ont_idx) != pred_to_surface.end())) {
        if ((std::find(pred_to_surface[ont_idx].begin(), pred_to_surface[ont_idx].end(), sur_idx)) != pred_to_surface[ont_idx].end()) {
            // del self.pred_to_surface[sur_idx] ??? dictionary
            pred_to_surface.erase (sur_idx);
        }
    }

    if ((std::find(reverse_entries.begin(), reverse_entries.end(), sem_idx) != reverse_entries.end())) {
        if ((std::find(reverse_entries[sem_idx].begin(), reverse_entries[sem_idx].end(), sur_idx)) != reverse_entries[sem_idx].end()) {
            reverse_entries.erase(std::find(reverse_entries.begin(), reverse_entries.end(), sur_idx));
        }
    }
} */