#include "SemanticNode.h"
#include "Ontology.h"
SemanticNode::SemanticNode(SemanticNode *parent, int type, int category, int idx, std::vector<SemanticNode *> children) : parent_(parent), type_(type), category_(category), idx_(idx), children_(children)
{
   is_lambda_ = false;
   is_lambda_instantiation_ = false;
   categories_used_.push_back(idx_);
}
SemanticNode::SemanticNode(SemanticNode *parent, int type, int category, int lambda_name, bool is_lambda_instantiation, std::vector<SemanticNode *> children) : parent_(parent), type_(type), category_(category), lambda_name_(lambda_name), is_lambda_instantiation_(is_lambda_instantiation), children_(children)
{
   is_lambda_ = true;
}

void SemanticNode::set_category(int idx){
   category_ = idx;
   if (std::find(categories_used_.begin(), categories_used_.end(), idx) == categories_used_.end())
      categories_used_.push_back(idx);
}

void SemanticNode::set_return_type(Ontology &ontology){
   bool debug = false;
   if(is_lambda_instantiation_){
      std::string type_str = ontology.compose_str_from_type(type_);
      children_[0]->set_return_type(ontology);
      std::string child_return_type_str = ontology.compose_str_from_type(children_[0]->return_type_);
      return_type_ = ontology.read_type_from_str("<" + type_str + "," + child_return_type_str + ">");
      if(debug)
         std::cout << "set_return_type: lambda instantiation set return" << std::endl;
   }
   else if (children_.size() == 0)
   {
      return_type_ = type_;
      if(debug)
         printf("set_return_type: leaf set return\n");
   }
   else
   {
      candidate_type_ = type_;
      for(SemanticNode *c : children_){
         c->set_return_type(ontology);
         int temp_type = boost::get<std::vector<int>>(ontology.types_[candidate_type_])[0];
         if(ontology.types_equal(temp_type, c->return_type_)){
            candidate_type_ = boost::get<std::vector<int>>(ontology.types_[candidate_type_])[1];
         }
         else
            std::cerr << "Incompatible child types" << std::endl;
      }
      return_type_ = candidate_type_;
      if(debug)
         printf("set return type: set return by consuming children");
   }
}

void SemanticNode::copy_attributes(SemanticNode &a, std::vector<int> *lambda_map, bool preserve_parent, bool preserve_children, int lambda_enumeration){
   set_category(a.category_);
   type_ = a.type_;
   is_lambda_ = a.is_lambda_;
   idx_ = a.idx_;
   lambda_name_ = a.lambda_name_ + lambda_enumeration;
   if(lambda_map != NULL){
      if(std::find(lambda_map->begin(), lambda_map->end(), lambda_name_) != lambda_map->end()){
         lambda_name_ = (*lambda_map)[a.lambda_name_];
      }
   }
   is_lambda_instantiation_ = a.is_lambda_instantiation_;
   if(!preserve_parent){
      parent_ = a.parent_;
   }
   if(!preserve_children){
      std::vector<SemanticNode*> temp();
         for(int i = 0; i < a.children_.size(); i++){
            SemanticNode child(this, 0, 0, 0, std::vector<SemanticNode *>());
            child.copy_attributes(*a.children_[i], lambda_map =lambda_map, preserve_children = true, lambda_enumeration=lambda_enumeration);
         }
   }
   return_type_ = a.return_type_;
}

std::string SemanticNode::print_little(){
   std::string s = "(";
   if(is_lambda_)
      s += std::to_string(is_lambda_) + ',' + std::to_string(type_) + ',' + std::to_string(lambda_name_);
   else
      s += std::to_string(idx_);
   s += ")";
   return s;
}

void SemanticNode::renumerate_lambdas(std::vector<int> lambdas){
   if(is_lambda_){
      if(is_lambda_instantiation_){
         lambdas.push_back(lambda_name_);
         lambda_name_ = lambdas.size();
      }
      else
      {
         auto it = std::find(lambdas.begin(), lambdas.end(), lambda_name_);
         lambda_name_ = std::distance(lambdas.begin(), it) + 1;
      }
   }
   if(children_.size() != 0){
      for(SemanticNode* c : children_)
         c->renumerate_lambdas(lambdas);
   }
}




