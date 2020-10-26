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
        t.push_back(n->surface_form_);
    }
    for (int i = 0; i < t.size(); i++){
        if (t[i] != NULL){

        }
    }
    for (int i = 0; i < t.size() - 1; i++){
        key(t[i], t[i+1]);
        auto it = res.find(key);
        if (it == res.end()){
            res[key] = 0;
        }
        res[key] += 1;
    }
    return res;
}