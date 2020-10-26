#include "ParseNode.h"
#include "Ontology.h"
#include "SemanticNode.h"
int main(int argc, char **argv){
    ParseNode x(nullptr, nullptr);
    ParseNode y(&x, nullptr);
    ParseNode z(&x, nullptr);
    x.children_.push_back(&y);
    x.children_.push_back(&z);
    std::vector<ParseNode *> temp = x.get_leaves();
    std::cout << temp[0] << " " << temp[1] << std::endl;
    Ontology ont("ont.txt");
    SemanticNode* parent1 = new SemanticNode(NULL,1, 2, 1, std::vector<SemanticNode*>());
    SemanticNode* parent2 = new SemanticNode(NULL, 1, 2, 2, true, std::vector<SemanticNode*>());
    SemanticNode* child1 = new SemanticNode(NULL, 1,2, 1, std::vector<SemanticNode *>());
    parent1->children_.push_back(child1);
    child1->parent_ = parent1;
    std::cout << parent1->print_little() << std::endl;
    std::cout << parent2->print_little() << std::endl;
    std::cout << child1->print_little() << std::endl;
    nodeTuple key = parent1->key();
    std::cout << std::get<0>(key) << std::endl;
    std::cout << std::get<1>(key) << std::endl;
    std::cout << std::get<2>(key) << std::endl;
    std::cout << std::get<3>(key) << std::endl;
    std::cout << std::get<4>(key) << std::endl;
    std::cout << std::get<5>(key) << std::endl;
    for (auto x : std::get<6>(key)){
        std::cout << x << std::endl;
    }
    std::cout << parent1->hash() << std::endl;
    std::cout << parent2->hash() << std::endl;
    std::cout << child1->hash() << std::endl;

    parent1->increment_lambdas();
    
    return 0;
}