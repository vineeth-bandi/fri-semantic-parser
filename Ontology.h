#ifndef ONTOLOGY_H
#define ONTOLOGY_H
#include <string.h>
#include <iostream>
#include <vector>
#include <boost/variant.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>

const std::string WHITESPACE = " \n\r\t\f\v";
typedef boost::variant<std::string, std::vector<int>> typesBoost;
class Ontology
{
public:
    Ontology(std::string ont_fname);
    void read_sem_fromfile(std::string fname);
    int calc_num_pred_args(int idx);
    int read_type_from_str(std::string s, bool allow_wild = false);
    std::string compose_str_from_type(int t);
    bool types_equal(int tidx, int tjdx);
    int find_index(typesBoost x);
    std::vector<std::string> preds_;
    std::vector<int> entries_;
    std::vector<typesBoost> types_;
    std::vector<int> commutative_;
    std::vector<int> num_args_;
};
#endif