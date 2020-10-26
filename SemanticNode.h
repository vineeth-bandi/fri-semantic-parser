#ifndef SEMANTIC_NODE_H
#define SEMANTIC_NODE_H
#include <vector>
#include "Ontology.h"
#include <stdio.h>
typedef std::tuple<int, int, bool, int, int, bool, std::vector<size_t>> nodeTuple;
class SemanticNode
{
public:
    SemanticNode *parent_;
    int type_;
    int category_;
    bool is_lambda_;
    int idx_;
    int lambda_name_;
    bool is_lambda_instantiation_;
    int return_type_;
    int candidate_type_;
    std::vector<SemanticNode *> children_;
    std::vector<int> categories_used_;
    SemanticNode(SemanticNode *parent, int type, int category, int idx, std::vector<SemanticNode *> children);
    SemanticNode(SemanticNode *parent, int type, int category, int lambda_name, bool is_lambda_instantiation, std::vector<SemanticNode *> children);
    SemanticNode(SemanticNode &a);
    ~SemanticNode();
    void set_category(int idx);
    void set_return_type(Ontology &ontology);
    void copy_attributes(SemanticNode &a, std::vector<int> *lambda_map = NULL, bool preserve_parent = false,
                         bool preserve_children = false, int lambda_enumeration = 0);
    void renumerate_lambdas(std::vector<int> &lambdas);
    bool validate_tree_structure();
    void increment_lambdas(int inc = 1);
    bool equal_allowing_commutativity(SemanticNode &other, Ontology &ontology, bool ignore_syntax = true);
    void commutative_raise_node(Ontology &ontology);
    void commutative_lower_node(Ontology &ontology);
    void set_type_from_children_return_types(int r, Ontology &ontology);
    std::vector<SemanticNode *> commutative_raise(SemanticNode &node, int idx);
    bool equal_ignoring_syntax(SemanticNode &other, bool ignore_synatx = true);
    std::string print_little()
    {
        std::string s = "(";
        if (is_lambda_)
            s += std::to_string(is_lambda_) + ',' + std::to_string(type_) + ',' + std::to_string(lambda_name_);
        else
            s += std::to_string(idx_);
        s += ")";
        return s;
    }
    /* printing object */
    bool
    operator==(SemanticNode &other); /* equality check */
    nodeTuple key();                 /* figure out how to make tuple */
    size_t hash();                   /* hash code for object */
};

#endif