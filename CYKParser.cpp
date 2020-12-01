#include "CYKParser.h"
#include <math.h>

Parameters::Parameters(Ontology ont, Lexicon lex, bool allow_merge, bool use_language_model = false, double lexicon_weight = 1.0){
    ont_ = ont;
    lex_ = lex;
    use_language_model_ = use_language_model;
    lexicon_weight_ = lexicon_weight;

    _CCG_given_token_counts = init_ccg_given_token(lexicon_weight);
    _CCG_production_counts = init_ccg_production(lexicon_weight, allow_merge);
    _lexicon_entry_given_token_counts = init_lexicon_entry(lexicon_weight);
    _skipwords_given_surface_form = init_skipwords_given_surface_form(lexicon_weight);

    update_probabilities();
}

void Parameters::update_probabilities(){
    int missing_entry_mass = 0;

    for(auto const& pair : _skipwords_given_surface_form) {
        skipwords_given_surface_form[pair.first] = log(1/(1 + exp(-pair.second)));
    }

    _CCG_root_counts.insert(std::make_pair(1, missing_entry_mass));

    std::vector<double> nums;
    double num_min = 0;
    double mass = 0;
    for(auto const& pair : _CCG_root_counts) {
        nums.push_back(pair.second);
        mass += pair.second;
    }
    double size = nums.size();
    if (size > 0){
        num_min = *min_element(nums.begin(), nums.end());
        mass = mass - num_min*size + size;
    }

    for(auto const& pair : _CCG_root_counts) {
        if (mass <= 0)
            CCG_root[pair.first] = log(1.0 / size);
        else CCG_root[pair.first] = (log(1 + pair.second + num_min) / mass);
    }
    
    for (int cat_idx = 0; cat_idx < lex_.categories.size(); cat_idx++){
        tuple2 index(cat_idx, -1);
        nums.clear();
        num_min = 0;
        mass = 0;
        _CCG_given_token_counts[index] = missing_entry_mass;
        for (int sf_idx = -1; sf_idx < lex_.surface_forms.size(); sf_idx++){
            tuple2 inner_index(cat_idx, sf_idx);
            auto it = _CCG_given_token_counts.find(inner_index);
            if (it != _CCG_given_token_counts.end()){
                nums.push_back(it->second);
                mass += it->second;
            }
        }
        size = nums.size();
        if (size > 0){
            num_min = *min_element(nums.begin(), nums.end());
            mass -= num_min*size;
        }
        for (int sf_idx = -1; sf_idx < lex_.surface_forms.size(); sf_idx++){
            tuple2 key(cat_idx, sf_idx);
            auto it = _CCG_given_token_counts.find(key);
            if (it != _CCG_given_token_counts.end()){
                if (mass <= 0)
                    CCG_given_token[key] = log(1.0 / size);
                else CCG_given_token[key] = (log(1 + it->second + num_min) / mass);
            }

            for (int sem_idx = 0; sem_idx < lex_.surface_forms.size(); sem_idx++){
                tuple2 key2(sem_idx, sf_idx);
                it = _lexicon_entry_given_token_counts.find(key);
                if (it == _lexicon_entry_given_token_counts.end()){
                    _lexicon_entry_given_token_counts[key2] = missing_entry_mass;
                }
            }

            std::vector<double> entry_nums;
            double entry_num_min = 0;
            double entry_mass = 0;
            if (sf_idx > -1){
                for (int sem_idx : lex_.entries[sf_idx]){
                    tuple2 key2(sem_idx, sf_idx);
                    entry_nums.push_back(_lexicon_entry_given_token_counts[key2]);
                }
            }
            else {
                for (double i = 0; i < lex_.semantic_forms.size(); i++){
                    entry_nums.push_back(i);
                }
            }
            
            size = entry_nums.size();
            if (size > 0){
            entry_num_min = *min_element(entry_nums.begin(), entry_nums.end());
            entry_mass = entry_mass - num_min*size + size;
            }
            if (sf_idx > -1){
                for (int sem_idx : lex_.entries[sf_idx]){
                    tuple2 key2(sem_idx, sf_idx);
                    if (mass <= 0)
                        lexicon_entry_given_token[key2] = log(1.0 / size);
                    else lexicon_entry_given_token[key2] = (log(1 + _lexicon_entry_given_token_counts[key2] + entry_num_min) / entry_mass);
                }
            }
            else {
                for (double i = 0; i < lex_.semantic_forms.size(); i++){
                    tuple2 key2(i, sf_idx);
                    if (mass <= 0)
                        lexicon_entry_given_token[key2] = log(1.0 / size);
                    else lexicon_entry_given_token[key2] = (log(1 + _lexicon_entry_given_token_counts[key2] + entry_num_min) / entry_mass);
                }
            }
        }

        nums.clear();
        mass = 0;
        num_min = 0;
        size = lex_.categories.size();
        for (int l_idx; l_idx < size; l_idx++){
            for (int r_idx; r_idx < size; r_idx++){
                tuple3 key(cat_idx, l_idx, r_idx);
                auto it = _CCG_production_counts.find(key);
                if (it != _CCG_production_counts.end()){
                    nums.push_back(it->second);
                    mass += it->second;
                }
            }
        }
        size = nums.size();
        if (size == 0)
            continue;
        num_min = *min_element(nums.begin(), nums.end());
        mass = mass - num_min*size + size;
        for (int l_idx; l_idx < lex_.categories.size(); l_idx++){
            for (int r_idx; r_idx < lex_.categories.size(); r_idx++){
                tuple3 key(cat_idx, l_idx, r_idx);
                auto it = _CCG_production_counts.find(key);
                if (it != _CCG_production_counts.end()){
                    if (mass <= 0)
                        CCG_production[key] = log(1.0 / size);
                    else CCG_production[key] = (log(1 + _CCG_production_counts[key] + num_min) / mass);
                }
            }
        }
    }

    size = ont_.preds_.size();
    for (int arg_idx = 0; arg_idx < size; arg_idx++){
        nums.clear();
        mass = 0;
        num_min = 0;
        for (int pred_idx = 0; pred_idx < size; pred_idx++){
            for (int pos = 0; pos < ont_.num_args[pred_idx]; pos++){
                tuple3 key(pred_idx, arg_idx, pos);
                auto it = _semantic_counts.find(key);
                if (it != _semantic_counts.end()){
                    nums.push_back(it->second);
                    mass += it->second;
                }
            }
        }
        num_min = *min_element(nums.begin(), nums.end());
        mass = mass - num_min*size + size;
        if (nums.size() == 0)
            continue;
        for (int pred_idx = 0; pred_idx < size; pred_idx++){
            for (int pos = 0; pos < ont_.num_args[pred_idx]; pos++){
                tuple3 key(pred_idx, arg_idx, pos);
                auto it = _semantic_counts.find(key);
                if (it != _semantic_counts.end()){
                    if (mass <= 0)
                        semantic[key] = log(1.0 / nums.size());
                    else semantic[key] = (log(1 + _semantic_counts[key] + num_min) / mass);
                }
            }
        }
    }
}

std::unordered_map<int, double> Parameters::init_skipwords_given_surface_form(double lexicon_weight){
    std::unorderd_map<int, double> res;
    for (int sf_idx = 0; sf_idx < lex_.surface_forms.size(); sf_idx++){
        res[sf_idx] = -lexicon_weight;
    }
    return res;
}

std::unordered_map<tuple2, double> Parameters::init_ccg_given_token(double lexicon_weight){
    std::unorderd_map<tuple2, double> res;
    for (int sf_idx = 0; sf_idx < lex_.entries.size(); sf_idx++){
        for (int sem_idx : lex_.entries[sf_idx]){
            tuple2 key(lex_.semantic_forms[sem_idx].category, sf_idx));
            res[key] = lexicon_weight;
        }
    }
    return res;
}

std::unordered_map<tuple3, double> init_ccg_production(double lexicon_weight, bool allow_merge){
    std::unorderd_map<tuple3, double> res;
    for (int cat_idx = 0; cat_idx < lex_.categories.size(); cat_idx++){
        std::vector<int[]> consumables = lex_.find_consumables_for_cat(cat_idx);
        for (int i = 0; i < consumables.size(); i++){
            int[] con = consumables[i];
            int d = con[0];
            int child = con[1];
            int l;
            int r;
            tuple3 key(cat_idx, d, child);
            if (d == 0){
                l = child;
                r = lex_.categories.find(key);
            }
            else {
                r = child;
                l = lex_.categories.find(key);
            }
            tuple3 key2(cat_idx, l, r);
            res[key2] = lexicon_weight;
        }
        if (allow_merge){
            tuple3 key(cat_idx, cat_idx, cat_idx);
            res[key] = lexicon_weight;
        }
    }
    return res;
}

std::unordered_map<tuple2, double> init_lexicon_entry(double lexicon_weight){
    std::unorderd_map<tuple2, double> res;
    for (int sf_idx = 0; sf_idx < lex_.entries.size(); sf_idx++){
        for (int sem_idx : lex_.entries[sf_idx]){
            tuple2 key(sem_idx, sf_idx));
            res[key] = lexicon_weight;
        }
    }
    return res;
}

double get_semantic_score(ParseNode n){
    std::unordered_map<tuple3, int> counts = count_semantics(n);
    double score = 0;
    for(auto const& pair : counts) {
        auto it = semantic.find(pair.first);
        if (it != semantic.end()){
            int count = counts[pair.first];
            for (int i = 0; i < count; i++)
                score += it->second;
        }
    }
    return score;
}

std::unordered_map<vvTuple2, int> count_token_bigrams(ParseNode y){
    std::unordered_map<vvTuple2, int> res;
    std::vector<int> t;
    for (ParseNode* n : y.get_leaves()){
        t.push_back(lex_.surface_forms.at(n->surface_form_));
    }
    t.insert(0, -2);
    t.push_back(-3);
    for (int i = 0; i < t.size() - 1; i++){
        tuple2 key(t[i], t[i+1]);
        auto it = res.find(key);
        if (it == res.end()){
            res[key] = 0;
        }
        res[key]++;
    }
    return res;
}

std::unordered_map<ltuple3, int> count_semantics(boostNode sn){
    if (sn.type() == typeid(ParseNode)){
        sn = *sn.node_;
    }
    std::unordered_map<ltuple3, int> counts;
    if (sn.children_.size() > 0){
        lambda pred = sn.idx_;
        if (sn.is_lambda_)
            if (sn.is_lambda_instantiation_)
                pred = "lambda_inst";
            else pred = "lambda"
        for (int i = 0; i < sn.children_.size(); i++){
            int arg = sn.children_[pos]->idx_;
            if (sn.children_[pos]->is_lambda_)
                if (sn.children_[pos]->is_lambda_instantiation_)
                    arg = "lambda_inst";
                else arg = "lambda"
            ltuple3 key(pred, arg, pos);
            if (counts.find(key) == counts.end()){
                counts[key] = 0;
            }
            counts[key]++;
        }
        for (int i = 0; i < sn.children_.size(); i++){
            std::unordered_map<ltuple3, int> child_counts = count_semantics(*sn.children_[i]);
            for(auto const& pair : child_counts) {
                auto it = counts.find(pair.first);
                if (it == counts.end()){
                    counts[pair.first] = 0;
                }
                counts[pair.first] += child_counts[pair.first];
            }
        }
    }
    return counts;
}

// TODO: instead of searching if not in list then finding index just find index or append
void update_learned_parameters(std::vector<boostT> t){
    double lr = 1.0 / sqrt(t.size());

    for (int i = 0; i < t.size(); i += 7){
        auto x = t[i]; // string?
        auto y = t[i+1]; //ParseNode?
        auto z = t[i+2]; // same
        auto y_lex = t[i+3]; //vectors of lexion entries
        auto z_lex = t[i+4]; //this too
        auto y_skipped = t[i+5]; // if it matters i think this is a vector of strings
        auto z_skipped = t[i+6]; // same for this
        for (int j = 0; j < y_skipped.size(); j++){
            auto y_key = y_skipped[j];
            if (std::find(z_skipped.begin(), z_skipped.end(), y_key) == z_skipped.end()){
                if (std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), y_key) == lex_.surface_forms.end()){
                    lex_.surface_forms.push_back(y_key);
                    std::vector<int> blank;
                    lex_.entries.push_back(blank);
                }
            }
            int y_sidx = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), y_key) - lex_.surface_forms.begin();
            if (_skipwords_given_surface_form.find(y_sidx) == _skipwords_given_surface_form.end())
                _skipwords_given_surface_form[y_sidx] = 0;
            _skipwords_given_surface_form[y_sidx] -= lr
            }
        }
        for (int j = 0; j < z_skipped.size(); j++){
            auto z_key = z_skipped[j];
            if (std::find(y_skipped.begin(), y_skipped.end(), z_key) == y_skipped.end()){
                if (std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), z_key) == lex_.surface_forms.end()){
                    lex_.surface_forms.push_back(z_key);
                    std::vector<int> blank;
                    lex_.entries.push_back(blank);
                }
            }
            int z_sidx = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), z_key) - lex_.surface_forms.begin();
            if (_skipwords_given_surface_form.find(z_sidx) == _skipwords_given_surface_form.end())
                _skipwords_given_surface_form[z_sidx] = 0;
            _skipwords_given_surface_form[z_sidx] -= lr
            }
        }

        ParseNode form = y;
        for (int j = 0; j < y_lex.size(); j += 2){
            lex_entries surface_form = y_lex[j];
            SemanticNode sem_node = y_lex[j+1];
            if (surface_form.type() == typeid(std::string) && std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), surface_form) == lex_.surface_forms.end())
                lex_.surface_forms.push_back(surface_form);
            int sf_idx = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), surface_form) - lex_.surface_forms.begin();
            if (_skipwords_given_surface_form.find(sf_idx) == _skipwords_given_surface_form.end())
                _skipwords_given_surface_form[sf_idx] = 0;
            if (std::find(lex_.semantic_forms.begin(), lex_.semantic_forms.end(), *sem_node) == lex_.semantic_forms.end()){
                lex_.semantic_forms.push_back(*sem_node);
            }
            int sem_idx = std::find(lex_.semantic_forms.begin(), lex_.semantic_forms.end(), *sem_node) - lex_.semantic_forms.begin();
            if (sf_idx == lex_.entries.size()){
                std::vector<int> blank;
                lex_.entries.push_back(blank);
            }
            if (std::find(lex_.entries[sf_idx].begin(), lex_.entries[sf_idx].end(), sf_idx) == lex_.entries[sf_idx].end())
                lex_.entries[sf_idx].push_back(sf_idx);
            tuple2 key(lex_.semantic_forms[sem_idx]->category_, sf_idx);
            if (_CCG_given_token_counts.find(key) == _CCG_given_token_counts.end())
                _CCG_given_token_counts[key] = 1;
            tuple2 key2(sf_idx, sem_idx);
            if (_lexicon_entry_given_token_counts.find(key2) == _lexicon_entry_given_token_counts.end())
                _lexicon_entry_given_token_counts[key2] = 1;
        }
        std::vector<ParseNode *> form_leaves = form.get_leaves();
        for (int j = 0; j < form_leaves.size(); j++){ 
            if (form_leaves[j]->surface_form.type() == typeid(std::string))
                form_leaves[j]->surface_form = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), form_leaves[j]->surface_form) - lex_.surface_forms.begin();
        }

        ParseNode form = z;
        for (int j = 0; j < z_lex.size(); j += 2){
            lex_entries surface_form = z_lex[j];
            SemanticNode sem_node = z_lex[j+1];
            if (surface_form.type() == typeid(std::string) && std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), surface_form) == lex_.surface_forms.end())
                lex_.surface_forms.push_back(surface_form);
            int sf_idx = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), surface_form) - lex_.surface_forms.begin();
            if (_skipwords_given_surface_form.find(sf_idx) == _skipwords_given_surface_form.end())
                _skipwords_given_surface_form[sf_idx] = 0;
            if (std::find(lex_.semantic_forms.begin(), lex_.semantic_forms.end(), *sem_node) == lex_.semantic_forms.end()){
                lex_.semantic_forms.push_back(*sem_node);
            }
            int sem_idx = std::find(lex_.semantic_forms.begin(), lex_.semantic_forms.end(), *sem_node) - lex_.semantic_forms.begin();
            if (sf_idx == lex_.entries.size()){
                std::vector<int> blank;
                lex_.entries.push_back(blank);
            }
            if (std::find(lex_.entries[sf_idx].begin(), lex_.entries[sf_idx].end(), sf_idx) == lex_.entries[sf_idx].end())
                lex_.entries[sf_idx].push_back(sf_idx);
            tuple2 key(lex_.semantic_forms[sem_idx]->category_, sf_idx);
            if (_CCG_given_token_counts.find(key) == _CCG_given_token_counts.end())
                _CCG_given_token_counts[key] = 1;
            tuple2 key2(sf_idx, sem_idx);
            if (_lexicon_entry_given_token_counts.find(key2) == _lexicon_entry_given_token_counts.end())
                _lexicon_entry_given_token_counts[key2] = 1;
        }
        std::vector<ParseNode *> form_leaves = form.get_leaves();
        for (int j = 0; j < form_leaves.size(); j++){ 
            if (form_leaves[j]->surface_form.type() == typeid(std::string))
                form_leaves[j]->surface_form = std::find(lex_.surface_forms.begin(), lex_.surface_forms.end(), form_leaves[j]->surface_form) - lex_.surface_forms.begin();
        }

        auto y_keys = count_ccg_surface_form_pairs(y);
        auto z_keys = count_ccg_surface_form_pairs(z);
        std::vector<ivTuple2> seen_keys;
        for (int j = 0; j < z_keys.size(); j++){
            z_val = z_keys[z_keys[j]];
            if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                y_val = y_keys[z_keys[j]];
                seen_keys.push_back(z_keys[j]);
            }
            else y_val = 0;
            if (std::find(_CCG_given_token_counts.begin(), _CCG_given_token_counts.end(), z_keys[j]) == _CCG_given_token_counts.end())
                _CCG_given_token_counts[z_keys[j]] = 0;
            _CCG_given_token_counts[z_keys[j]] += lr * (z_val - y_val);
        }
        for (int j = 0; j < y_keys.size(); j++){
            if (std::find(seen_keys.begin(), seen_keys.end(), y_keys[j]) != seen_keys.end())
                continue;
            y_val = y_keys[y_keys[j]];
            z_val = 0;
            if (std::find(_CCG_given_token_counts.begin(), _CCG_given_token_counts.end(), y_keys[j]) == _CCG_given_token_counts.end())
                _CCG_given_token_counts[y_keys[j]] = 0;
            _CCG_given_token_counts[y_keys[j]] += lr * (z_val - y_val);
        }

        y_keys = count_ccg_productions(y);
        z_keys = count_ccg_productions(z);
        std::vector<tuple3> seen_keys2;
        for (int j = 0; j < z_keys.size(); j++){
            z_val = z_keys[z_keys[j]];
            if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                y_val = y_keys[z_keys[j]];
                seen_keys2.push_back(z_keys[j]);
            }
            else y_val = 0;
            if (std::find(_CCG_production_counts.begin(), _CCG_production_counts.end(), z_keys[j]) == _CCG_production_counts.end())
                _CCG_production_counts[z_keys[j]] = 0;
            _CCG_production_counts[z_keys[j]] += lr * (z_val - y_val);
        }
        for (int j = 0; j < y_keys.size(); j++){
            if (std::find(seen_keys2.begin(), seen_keys2.end(), y_keys[j]) != seen_keys2.end())
                continue;
            y_val = y_keys[y_keys[j]];
            z_val = 0;
            if (std::find(_CCG_production_counts.begin(), _CCG_production_counts.end(), y_keys[j]) == _CCG_production_counts.end())
                _CCG_production_counts[y_keys[j]] = 0;
            _CCG_production_counts[y_keys[j]] += lr * (z_val - y_val);
        }

        y_keys = count_lexical_entries(y);
        z_keys = count_lexical_entries(z);
        std::vector<svTuple2> seen_keys3;
        for (int j = 0; j < z_keys.size(); j++){
            z_val = z_keys[z_keys[j]];
            if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                y_val = y_keys[z_keys[j]];
                seen_keys3.push_back(z_keys[j]);
            }
            else y_val = 0;
            if (std::find(_lexicon_entry_given_token_counts.begin(), _lexicon_entry_given_token_counts.end(), z_keys[j]) == _lexicon_entry_given_token_counts.end())
                _lexicon_entry_given_token_counts[z_keys[j]] = 0;
            _lexicon_entry_given_token_counts[z_keys[j]] += lr * (z_val - y_val);
        }
        for (int j = 0; j < y_keys.size(); j++){
            if (std::find(seen_keys3.begin(), seen_keys3.end(), y_keys[j]) != seen_keys3.end())
                continue;
            y_val = y_keys[y_keys[j]];
            z_val = 0;
            if (std::find(_lexicon_entry_given_token_counts.begin(), _lexicon_entry_given_token_counts.end(), y_keys[j]) == _lexicon_entry_given_token_counts.end())
                _lexicon_entry_given_token_counts[y_keys[j]] = 0;
            _lexicon_entry_given_token_counts[y_keys[j]] += lr * (z_val - y_val);
        }

        y_keys = count_semantics(y);
        z_keys = count_semantics(z);
        std::vector<ltuple3> seen_keys4;
        for (int j = 0; j < z_keys.size(); j++){
            z_val = z_keys[z_keys[j]];
            if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                y_val = y_keys[z_keys[j]];
                seen_keys4.push_back(z_keys[j]);
            }
            else y_val = 0;
            if (std::find(_semantic_counts.begin(), _semantic_counts.end(), z_keys[j]) == _semantic_counts.end())
                _semantic_counts[z_keys[j]] = 0;
            _semantic_counts[z_keys[j]] += lr * (z_val - y_val);
        }
        for (int j = 0; j < y_keys.size(); j++){
            if (std::find(seen_keys4.begin(), seen_keys4.end(), y_keys[j]) != seen_keys4.end())
                continue;
            y_val = y_keys[y_keys[j]];
            z_val = 0;
            if (std::find(_semantic_counts.begin(), _semantic_counts.end(), y_keys[j]) == _semantic_counts.end())
                _semantic_counts[y_keys[j]] = 0;
            _semantic_counts[y_keys[j]] += lr * (z_val - y_val);
        }

        y_keys = count_ccg_root(y);
        z_keys = count_ccg_root(z);
        std::vector<int> seen_keys5;
        for (int j = 0; j < z_keys.size(); j++){
            z_val = z_keys[z_keys[j]];
            if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                y_val = y_keys[z_keys[j]];
                seen_keys5.push_back(z_keys[j]);
            }
            else y_val = 0;
            if (std::find(_CCG_root_counts.begin(), _CCG_root_counts.end(), z_keys[j]) == _CCG_root_counts.end())
                _CCG_root_counts[z_keys[j]] = 0;
            _CCG_root_counts[z_keys[j]] += lr * (z_val - y_val);
        }
        for (int j = 0; j < y_keys.size(); j++){
            if (std::find(seen_keys5.begin(), seen_keys5.end(), y_keys[j]) != seen_keys5.end())
                continue;
            y_val = y_keys[y_keys[j]];
            z_val = 0;
            if (std::find(_CCG_root_counts.begin(), _CCG_root_counts.end(), y_keys[j]) == _CCG_root_counts.end())
                _CCG_root_counts[y_keys[j]] = 0;
            _CCG_root_counts[y_keys[j]] += lr * (z_val - y_val);
        }

        if (use_language_model){
            y_keys = count_token_bigrams(y);
            z_keys = count_token_bigrams(z);
            std::vector<vvTuple2> seen_keys6;
            for (int j = 0; j < z_keys.size(); j++){
                z_val = z_keys[z_keys[j]];
                if (std::find(y_keys.begin(), y_keys.end(), z_keys[j]) != y_keys.end()){
                    y_val = y_keys[z_keys[j]];
                    seen_keys6.push_back(z_keys[j]);
                }
                else y_val = 0;
                if (std::find(_token_given_token_counts.begin(), _token_given_token_counts.end(), z_keys[j]) == _token_given_token_counts.end())
                    _token_given_token_counts[z_keys[j]] = 0;
                _token_given_token_counts[z_keys[j]] += lr * (z_val - y_val);
            }
            for (int j = 0; j < y_keys.size(); j++){
                if (std::find(seen_keys6.begin(), seen_keys6.end(), y_keys[j]) != seen_keys6.end())
                    continue;
                y_val = y_keys[y_keys[j]];
                z_val = 0;
                if (std::find(_token_given_token_counts.begin(), _token_given_token_counts.end(), y_keys[j]) == _token_given_token_counts.end())
                    _token_given_token_counts[y_keys[j]] = 0;
                _token_given_token_counts[y_keys[j]] += lr * (z_val - y_val);
            }
        }
    }

    update_probabilities();
}

std::unordered_map<svTuple2, int> count_lexical_entries(ParseNode y){
    std::unordered_map<svTuple2, int> pairs;
    std::vector<ParseNode *> token_assignments = y.get_leaves();
    for (int i = 0; i < token_assignments.size(); i++){
        svTuple2 key(token_assignments[i]->surface_form_, token_assignments[i]->semantic_form_);
        auto it = pairs.find(key);
        if (it == pairs.end()){
            pairs[key] = 0;
        }
        pairs[key]++;
    }
    return pairs;
}

std::unordered_map<tuple3, int> count_ccg_productions(ParseNode y){
    std::unordered_map<tuple3, int> productions;
    std::vector<ParseNode> to_explore;
    to_explore.push_back(y);
    while(to_explore.size() > 0){
        ParseNode n = to_explore.back();
        to_explore.pop_back();
        if (n.children_.size() > 0){
            tuple3 key(n.node->category_, n.children[0]->category_, n.children[1]->category_);
            if (productions.find(key) == productions.end())
                productions[key] = 0;
            productions[key]++;
            to_explore.reserve(to_explore.size() + n.children_.size());
            for (int i = 0; i < n.children_.size(); i++){
                to_explore.push_back(*n.children_[i]);
            }
        }
    }
    return productions;
}

std::unordered_map<int, int> count_ccg_root(ParseNode y){
    std::unordered_map<int, int> root;
    root[y.node->category_] = 1;
    return root;
}

std::unordered_map<ivTuple2, int> count_ccg_surface_form_pairs(ParseNode y){
    std::unordered_map<ivTuple2, int> pairs;
    std::vector<ParseNode *> token_assignments = y.get_leaves();
    for (int i = 0; i < token_assignments.size(); i++){
        ivTuple2 key(token_assignments[i]->category_, token_assignments[i]->surface_form_);
        auto it = pairs.find(key);
        if (it == pairs.end()){
            pairs[key] = 0;
        }
        pairs[key]++;
    }
    return pairs;
}