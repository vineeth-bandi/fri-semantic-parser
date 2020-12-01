
#include "ParseNode.h"

ParseNode::ParseNode(ParseNode *parent, SemanticNode *node, std::vector<ParseNode *> children, boost::variant<std::string, int> surface_form, std::vector<int> semantic_form) : parent_(parent), node_(node), children_(children), surface_form_(surface_form), semantic_form_(semantic_form) {

}

std::vector<ParseNode *> ParseNode::get_leaves() {
    std::vector<ParseNode *> leaves;
    if (children_.size() != 0) {
        leaves = children_[0]->get_leaves();
        std::vector<ParseNode *> leaves2 = children_[1]->get_leaves();
        leaves.insert(leaves.end(), leaves2.begin(), leaves2.end());
        return leaves;
    }
    leaves.push_back(this);
    return leaves;
}
