#ifndef PARSENODE_H
#define PARSENODE_H
#include <iostream>
#include <vector>
#include <SemanticNode.h>
class ParseNode
{
public:
    ParseNode *parent_;
    SemanticNode *node_;
    std::vector<ParseNode *> children_;
    std::vector<int> semantic_form_;
    std::string surface_form_;
    ParseNode(ParseNode *parent,  SemanticNode* node, std::vector<ParseNode *> children = {}, std::string surface_form = "", std::vector<int> semantic_form = {});
    std::vector<ParseNode *> get_leaves();
};

#endif