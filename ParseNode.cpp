#include <iostream>
#include <vector>
#include "ParseNode.h"
class ParseNode
{
private:
    ParseNode *parent_ = nullptr;
    ParseNode *node_ = nullptr;
    std::vector<ParseNode *> children_;
    std::vector<int> semantic_form_;
    std::vector<int> surface_form_;
    ParseNode::ParseNode(ParseNode *parent, ParseNode *node, std::vector<ParseNode *> children = {}, std::vector<int> surface_form= {}, std::vector<int> semantic_form = {}) : parent_(parent), node_(node), children_(children), surface_form_(surface_form), semantic_form_(surface_form)
    {
    
    }
    std::vector<ParseNode *> get_leaves()
    {
        if (children_.size() != 0)
        {
            std::vector<ParseNode *> leaves = children_[0]->get_leaves();
            std::vector<ParseNode *> leaves2 = children_[1]->get_leaves();
            leaves.insert(leaves.end(), leaves2.begin(), leaves2.end());
            return leaves;
        }
        std::vector<ParseNode *> leaves;
        leaves.push_back(this);
        return leaves;
    }
};