#include "SemanticNode.h"
#include "Ontology.h"

SemanticNode::SemanticNode(SemanticNode *parent, int type, int category, int idx, std::vector<SemanticNode *> children) : parent_(parent), type_(type), category_(category), idx_(idx), children_(children)
{
   is_lambda_ = false;
   is_lambda_instantiation_ = false;
   lambda_name_ = 0;
   categories_used_.push_back(idx_);
}

SemanticNode::SemanticNode(SemanticNode *parent, int type, int category, int lambda_name, bool is_lambda_instantiation, std::vector<SemanticNode *> children) : parent_(parent), type_(type), category_(category), lambda_name_(lambda_name), is_lambda_instantiation_(is_lambda_instantiation), children_(children)
{
   is_lambda_ = true;
   idx_ = 0;
}

SemanticNode::SemanticNode(SemanticNode &a): type_(a.type_), category_(a.category_), lambda_name_(a.lambda_name_), is_lambda_(a.is_lambda_), is_lambda_instantiation_(a.is_lambda_instantiation_), idx_(a.idx_){
   if(a.parent_ != NULL)
      parent_ = new SemanticNode(*a.parent_);
   children_ = std::vector<SemanticNode *>();
   for(int i = 0; i < a.children_.size(); i++){
      SemanticNode* child = new SemanticNode(*a.children_[0]);
      children_.push_back(child);
   }
}
SemanticNode::~SemanticNode(){
   for(SemanticNode *c : children_)
      delete(c);
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
      for(SemanticNode *c : children_)
         free(c);
      children_ = std::vector<SemanticNode *>();
      for(int i = 0; i < a.children_.size(); i++){
         SemanticNode *child = new SemanticNode(this, 0, 0, 0, std::vector<SemanticNode *>());
         child->copy_attributes(*a.children_[i], lambda_map =lambda_map, preserve_children = true, lambda_enumeration=lambda_enumeration);
         children_.push_back(child);
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
      for(SemanticNode *c : children_)
         c->renumerate_lambdas(lambdas);
   }
}

bool SemanticNode::validate_tree_structure (){
   if(children_.size() != 0)
      return true;
   for(int i =0; i< children_.size();i++){
      if(children_[i]->parent_ != this)
         return false;
      if(!children_[i]->validate_tree_structure())
         return false;
   }
   return true;
}

void SemanticNode::increment_lambdas(int inc){
   if(is_lambda_)
      lambda_name_ += inc;
   if(children_.size() > 0){
      for(SemanticNode *c: children_)
         c->increment_lambdas(inc);
   }
}

bool SemanticNode::equal_allowing_commutativity(SemanticNode &other, Ontology &ontology, bool ignore_syntax){
   SemanticNode a(*this);
   SemanticNode b(*this);
   a.commutative_raise_node(ontology);
   b.commutative_raise_node(ontology);
   return a.equal_ignoring_syntax(b, ignore_syntax=ignore_syntax);
}

bool compare_by_idx(SemanticNode &a, SemanticNode &b){
   return a.idx_ < b.idx_;
}

void SemanticNode::commutative_raise_node(Ontology &ontology){
   std::vector<SemanticNode *> to_expand = std::vector<SemanticNode *>();
   to_expand.push_back(this);
   while( to_expand.size() > 0){
      SemanticNode* curr = to_expand.back();
      to_expand.pop_back();
      if(std::find(ontology.commutative_.begin(), ontology.commutative_.end(), curr->idx_) != ontology.commutative_.end()){
         std::vector<SemanticNode *> new_c = std::vector<SemanticNode *>();
         if(curr->children_.size() > 0){
            for(SemanticNode *c : curr->children_){
                  std::vector<SemanticNode *> temp = commutative_raise(*c, curr->idx_);
                  new_c.insert(new_c.end(), temp.begin(), temp.end());
            }
            for(SemanticNode *nc : new_c)
               nc->parent_ = curr;
            std::sort(new_c.begin(), new_c.end(), compare_by_idx);
            curr->children_ = new_c;
            typesBoost pred = ontology.preds_[curr->idx_];
            if((pred.type() == typeid(std::string)) && boost::get<std::string>(pred) == "and")
               curr->set_type_from_children_return_types(curr->children_[0]->return_type_, ontology);
         }
      }
      if(curr->children_.size() > 0)
         to_expand.insert(to_expand.end(), curr->children_.begin(), curr->children_.end());
   }
}

void SemanticNode::commutative_lower_node(Ontology &ontology){
   std::vector<SemanticNode *> to_explore = std::vector<SemanticNode *>();
   to_explore.push_back(this);
   while(to_explore.size() > 0){
      SemanticNode *curr = to_explore.back();
      to_explore.pop_back();
      if(std::find(ontology.commutative_.begin(), ontology.commutative_.end(), curr->idx_) != ontology.commutative_.end()){
         if(curr->children_.size() > 0  && curr->children_.size() > 2){
            std::vector<SemanticNode *> children_to_nest = std::vector<SemanticNode *>();
            for(int i = 1; i < curr->children_.size(); i++)
               children_to_nest.push_back( new SemanticNode (*curr->children_[i]));
            SemanticNode *temp = curr->children_[0];
            curr->children_ = std::vector<SemanticNode *>();
            curr->children_.push_back(temp);
            curr->children_.push_back(new SemanticNode(curr, curr->type_, curr->category_,curr->idx_, children_to_nest));
         }
      }
      if(curr->children_.size() > 0)
         to_explore.insert(to_explore.end(), curr->children_.begin(), curr->children_.end());
   }
}

void SemanticNode::set_type_from_children_return_types(int r, Ontology &ontology){
   int new_type = r;
   for(int i =0; i< children_.size(); i++){
      std::vector<int> new_type_form = std::vector<int>();
      new_type_form.push_back(children_[i]->return_type_);
      new_type_form.push_back(new_type);
      if(std::find(ontology.types_.begin(), ontology.types_.end(), new_type_form) != ontology.types_.end()){
         ontology.types_.push_back(new_type_form);
      }
      new_type = std::distance(ontology.types_.begin(), std::find(ontology.types_.begin(), ontology.types_.end(), new_type_form));
   }
   type_ = new_type;
}

std::vector<SemanticNode *> SemanticNode::commutative_raise(SemanticNode &node, int idx){
   if(node.idx_ == idx && node.children_.size() > 0){
      std::vector<SemanticNode *> new_c = std::vector<SemanticNode *>();
      for(SemanticNode *c : node.children_){
         std::vector<SemanticNode *> temp = commutative_raise(*c, idx);
         new_c.insert(new_c.end(), temp.begin(), temp.end());
      }
      return new_c;
   }
   std::vector<SemanticNode *> ret = std::vector<SemanticNode *>();
   ret.push_back(&node);
   return ret;
}

bool SemanticNode::equal_ignoring_syntax(SemanticNode &other, bool ignore_syntax){
   if(is_lambda_ == other.is_lambda_ && idx_ == other.idx_ && lambda_name_ == lambda_name_ &&(ignore_syntax || category_ == other.category_)){
      if(children_.size() == other.children_.size()){
         for(int i =0; i< children_.size(); i++){
            if(!children_[i]->equal_ignoring_syntax(*other.children_[i], ignore_syntax=ignore_syntax))
               return false;
         }
         return true;
      }
   }
   return false;
}

bool SemanticNode::operator== (SemanticNode &a){
   return equal_ignoring_syntax(a, false);
}

nodeTuple SemanticNode::key(){
   std::vector<size_t> c_keys = std::vector<size_t>();
   for(SemanticNode *c : children_){
      c_keys.push_back(c->hash());
   }
   nodeTuple ret = std::make_tuple(type_, category_, is_lambda_, idx_, lambda_name_, is_lambda_instantiation_, c_keys);
   return ret;
}

size_t SemanticNode::hash(){
   return tuplehash(key());
}

static size_t
tuplehash(const nodeTuple v)
{
   size_t x;  /* Unsigned for defined overflow behavior. */
   size_t y;
   size_t len = 6;
   size_t mult = 0x6a09e667;
   x = 0x345678UL;

   y = std::hash<int>()(std::get<0>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);

   y = std::hash<int>()(std::get<1>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);


   y = std::hash<bool>()(std::get<2>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);

   y = std::hash<int>()(std::get<3>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);

   y = std::hash<int>()(std::get<4>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);

   y = std::hash<bool>()(std::get<5>(v));
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);
   y = 0;
   for(int i : std::get<6>(v)){
      y ^= std::hash<int>()(i);
   }
   x = (x ^ y) * mult;
   /* the cast might truncate len; that doesn't change hash stability */
   mult += (size_t)(82520UL + len + len);

   x += 97531UL;
   if (x == (size_t)-1)
      x = -2;
   return x;
}







