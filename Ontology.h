#ifndef ONTOLOGY_H
#define ONTOLOGY_H
#include <string.h>
#include <iostream>
#include <vector>
class Ontology
{
private:
    std::vector<std::string> preds;
    std::vector<std::string> entries;
    virtual std::tuple<std::vector<std::string>> read_sem_fromfile(std::string fname) = 0;
    virtual int calc_num_pred_args(int idx) = 0;
    virtual int read_type_from_str(std::string s, bool allow_wild) = 0;
    virtual std::string compose_str_from_type(int t) = 0;
    virtual bool types_equal(int tidx, int tjdx) = 0;
public:
};
#endif