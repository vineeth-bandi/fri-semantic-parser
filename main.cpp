#include "ParseNode.h"
#include "Ontology.h"

int main(int argc, char **argv){
    ParseNode x(nullptr, nullptr);
    ParseNode y(&x, nullptr);
    ParseNode z(&x, nullptr);
    x.children_.push_back(&y);
    x.children_.push_back(&z);
    std::vector<ParseNode *> temp = x.get_leaves();
    std::cout << temp[0] << " " << temp[1] << std::endl;
    Ontology ont("ont.txt");
    return 0;
}