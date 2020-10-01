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
    virtual void read_sem_fromfile(std::string fname) = 0;
    virtual int calc_num_pred_args(int idx) = 0;
    virtual int read_type_from_str(std::string s, bool allow_wild = false) = 0;
    virtual std::string compose_str_from_type(int t) = 0;
    virtual bool types_equal(int tidx, int tjdx) = 0;
    virtual int find_index(typesBoost x) = 0;
    std::vector<std::string> preds_;
    std::vector<int> entries_;
    std::vector<typesBoost> types_;
    std::vector<int> commutative_;
    std::vector<int> num_args_;
};
#endif