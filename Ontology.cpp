#include "Ontology.h"

int Ontology::calc_num_pred_args(int idx)
{
    int num_args = 0;
    typesBoost curr_type = types_[entries_[idx]];
    while (curr_type.type() == typeid(std::vector<int>))
    {
        num_args += 1;
        curr_type = types_[boost::get<std::vector<int>>(curr_type)[1]];
    }
    return num_args;
}
void Ontology::read_sem_fromfile(std::string fname)
{
    preds_.push_back("*and");
    entries_.push_back(Ontology::read_type_from_str("'<*,<*,*>>", true));
    std::ifstream file;
    file.open(fname);
    if (!file)
    {
        std::cout << "File not found" << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(file, line))
    {
        std::size_t start = line.find_first_not_of(WHITESPACE);
        std::size_t end = line.find_last_not_of(WHITESPACE);
        line = line.substr(start, end - start + 1);
        if (line.length() == 0 || line[0] == '#')
            continue;
        std::string intermediate;
        std::stringstream token_stream(line);
        std::getline(token_stream, intermediate, ':');
        std::string name = intermediate;
        std::getline(token_stream, intermediate, ':');
        std::string type_str = intermediate;
        typesBoost nameBoosted = name;
        int index = find_index(nameBoosted);
        if (index != -1)
        {
            std::cout << "Multiple predicates";
            exit(0);
        }
        entries_.push_back(Ontology::read_type_from_str(type_str));
        preds_.push_back(name);
    }
    file.close();
}
int Ontology::read_type_from_str(std::string s, bool allow_wild)
{
    if (s[0] == '<' && s[s.size() - 1] == '>')
    {
        int d = 0;
        int split_idx = 1;
        for (; split_idx < s.size() - 1; split_idx++)
        {
            if (s[split_idx] == '<')
                d += 1;
            else if (s[split_idx] == '>')
                d -= 1;
            else if (s[split_idx] == ',' && d == 0)
                break;
        }
        std::vector<int> comp_type;
        std::string s1;
        std::string s2;
        s1 = s.substr(1, split_idx - 1);
        s2 = s.substr(split_idx + 1, s.size() - split_idx - 2);
        comp_type.push_back(Ontology::read_type_from_str(s1, allow_wild));
        comp_type.push_back(Ontology::read_type_from_str(s2, allow_wild));
        typesBoost comp_type_converted = comp_type;
        int index = Ontology::find_index(comp_type_converted);
        if (index != -1)
            return index;
        types_.push_back(comp_type_converted);
        return types_.size() - 1;
    }
    typesBoost str = s;
    int index = Ontology::find_index(str);
    if (index == -1)
    {
        types_.push_back(str);
        index = types_.size() - 1;
    }
    if (!allow_wild && !s.compare("*"))
    {
        std::cerr << "The * types only has internal support" << std::endl;
        exit(0);
    }
    return index;
}
std::string Ontology::compose_str_from_type(int t)
{
    std::string s = "";
    if (types_[t].type() == typeid(std::vector<int>))
    {
        std::vector<int> temp = boost::get<std::vector<int>>(types_[t]);
        s = s.append("<");
        s = s.append(Ontology::compose_str_from_type(temp[0]));
        s = s.append(",");
        s = s.append(Ontology::compose_str_from_type(temp[1]));
        s = s.append(">");
        return s;
    }
    else
    {
        s = s.append(boost::get<std::string>(types_[t]));
    }
    return s;
}
bool Ontology::types_equal(int tidx, int tjdx)
{
    typesBoost ti = types_[tidx];
    typesBoost tj = types_[tjdx];
    if (ti.type() == typeid(std::vector<int>) && tj.type() == typeid(std::vector<int>))
        return true;
    else if (tidx == tjdx || tidx == 0 || tjdx == 0)
        return true;
    return false;
}
int Ontology::find_index(typesBoost x)
{
    auto it = std::find(types_.begin(), types_.end(), x);
    if (it == types_.end())
        return -1;
    int index = std::distance(types_.begin(), it);
    return index;
}

Ontology::Ontology(std::string ont_fname)
{
    typesBoost temp = "*";
    types_.push_back(temp);
    Ontology::read_sem_fromfile(ont_fname);
    for (int pidx = 0; pidx < preds_.size(); pidx++)
        if (preds_[pidx][0] == '*')
        {
            preds_[pidx] = preds_[pidx].substr(1, preds_[pidx].size() - 1);
            commutative_.push_back(pidx);
        }
    for (int i = 0; i < preds_.size(); i++)
        num_args_.push_back(Ontology::calc_num_pred_args(i));
}
