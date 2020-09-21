#ifndef PARSENODE_H
#define PARSENODE_H
#include "ParseNode.h"
class ParseNode
{
private:
    ParseNode *parent_ = 0;
    ParseNode *node_ = 0;
    std::vector<ParseNode *> children_ = 0;
    std::vector<int> semantic_form_ = 0;
    std::vector<int> surface_form_ = 0;

public:
    virtual std::vector<ParseNode> get_leaves() = 0;
};

#endif