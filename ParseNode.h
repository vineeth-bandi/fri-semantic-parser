#ifndef PARSENODE_H
#define PARSENODE_H

class ParseNode
{
private:
    ParseNode *parent_;
    ParseNode *node_;
    std::vector<ParseNode *> children_;
    std::vector<int> semantic_form_;
    std::vector<int> surface_form_;

public:
    ParseNode(ParseNode *parent, ParseNode *node, std::vector<ParseNode *> children = {}, std::vector<int> surface_form = {}, std::vector<int> semantic_form = {});
    std::vector<ParseNode *> get_leaves();
};

#endif