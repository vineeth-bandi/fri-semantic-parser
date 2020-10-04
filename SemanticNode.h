#ifndef SEMANTIC_NODE_H
#define SEMANTIC_NODE_H
#include <vector>
#include "Ontology.h"
typedef std::tuple<int, int, bool, int, int, bool, std::vector<size_t>> nodeTuple;
class SemanticNode
{
public:
    SemanticNode* parent;
    int type;
    int category;
    bool is_lambda;
    int idx;
    int lambda_name;
    bool is_lambda_instantiation;
    std::vector<SemanticNode *> children;
    std::vector<int> categories_used;
    SemanticNode(SemanticNode *parent, int type, int category, bool is_lambda, 
        int idx, int lambda_name, bool is_lambda_instantiation, std::vector<SemanticNode *> children);
    void set_category(int idx);
    void set_return_type(Ontology &ontology);
    void copy_attributes(SemanticNode &a, std::vector<int> &lambda_map, bool preserve_parent = false, 
        bool preserve_children = false, int lambda_enumeration = 0);
    std::string print_little();
    void renumerate_lambdas(std::vector<int> &lambdas);
    bool validate_tree_structure();
    void increment_lambdas(int inc = 1);
    bool equal_allowing_commutativity(SemanticNode &other, Ontology &ontology, bool ignore_syntax = true);
    void commutative_raise_node(Ontology &ontology);
    void commutative_lower_node(Ontology &ontology);
    void set_type_from_children_return_types(int r, Ontology &ontology);
    std::vector<SemanticNode*> commutative_raise(SemanticNode node, int idx);
    bool equal_ignoring_syntax(SemanticNode &other, bool ignore_synatx = true);
    /* printing object */
    std::ostream& operator<< (std::ostream &strm, const SemanticNode& a);
    bool operator== (SemanticNode &other);
    std::tuple<nodeTuple> key(); /* figure out how to make tuple */
    size_t hash(); /* hash code for object */
};
/* equality check */

#endif