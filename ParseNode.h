#ifndef PARSENODE_H
#define PARSENODE_H
#include <iostream>
#include <vector>
#include "SemanticNode.h"
#include <boost/variant.hpp>
class ParseNode
{
public:
    ParseNode *parent_;
    SemanticNode *node_;
    std::vector<ParseNode *> children_;
    std::vector<int> semantic_form_;
    boost::variant<std::string, int> surface_form_;
    ParseNode(ParseNode *parent,  SemanticNode* node, std::vector<ParseNode *> children = {}, boost::variant<std::string, int> surface_form = "", std::vector<int> semantic_form = {});
    std::vector<ParseNode *> get_leaves();
};

#endif