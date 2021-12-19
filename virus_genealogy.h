#include <memory>
#include <set>
#include <map>
#include <vector>

class VirusNotFound : public std::exception {
public:
    virtual const char* what() const noexcept {return "VirusNotFound\n";}
};

class VirusAlreadyCreated : public std::exception {
public:
    virtual const char* what() const noexcept {return "VirusAlreadyCreated\n";}
};

class TriedToRemoveStemVirus : public std::exception {
public:
    virtual const char* what() const noexcept {return "TriedToRemoveStemVirus\n";}
};

template<typename Virus>
class VirusGenealogy {
private:
    struct VirusNode {
        std::shared_ptr<Virus> ptr;
        std::set<std::shared_ptr<Virus>> childs;
        std::set<std::shared_ptr<Virus>> parents;

        explicit VirusNode(typename Virus::id_type id) : ptr(std::make_shared<Virus>(id)) {}
    };

    using nodes_t = std::map<typename Virus::id_type, std::shared_ptr<VirusNode>>;
    nodes_t nodes;
    typename Virus::id_type stem_id;

public:
    class children_iterator {
    private:
        typename std::set<std::shared_ptr<Virus>>::iterator it;

    public:
        explicit children_iterator() = default;

        explicit children_iterator(typename std::set<std::shared_ptr<Virus>>::iterator it)
                : it(it) {}

        children_iterator operator++() {
            ++it;
            return *this;
        }

        children_iterator operator++(int) {
            auto buf = *this;
            it++;
            return buf;
        }

        children_iterator operator--() {
            --it;
            return *this;
        }

        children_iterator operator--(int) {
            auto buf = *this;
            it--;
            return buf;
        }

        Virus &operator*() const {
            return **it;
        }

        std::shared_ptr<Virus> operator->() const {
            return *it;
        }

        bool operator==(const children_iterator &other) const {
            return it == other.it;
        }

        bool operator!=(const children_iterator &other) const {
            return it != other.it;
        }
    };

    // Tworzy nową genealogię.
    // Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
    explicit VirusGenealogy(typename Virus::id_type const &stem_id) :
    nodes{{stem_id, std::make_shared<VirusNode>(stem_id)}},
    stem_id(stem_id) {}

    // Zwraca identyfikator wirusa macierzystego.
    typename Virus::id_type get_stem_id() const {return stem_id;}

    // Zwraca iterator pozwalający przeglądać listę identyfikatorów
    // bezpośrednich następników wirusa o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    // Iterator musi spełniać koncept bidirectional_iterator oraz
    // typeid(*v.get_children_begin()) == typeid(const Virus &).
    VirusGenealogy<Virus>::children_iterator get_children_begin(typename Virus::id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();
        return children_iterator(nodes.find(id)->second->childs.begin());
    }

    // Iterator wskazujący na element za końcem wyżej wspomnianej listy.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    VirusGenealogy<Virus>::children_iterator get_children_end(typename Virus::id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();
        return children_iterator(nodes.find(id)->second->childs.end());
    }

    // Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
    // o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const {
        auto it = nodes.find(id);
        if (it == nodes.end())
            throw VirusNotFound();
        std::vector<typename Virus::id_type> res;
        for (auto &it : it->second->parents) {
            res.push_back(it->get_id());
        }
        return res;
    };

    // Sprawdza, czy wirus o podanym identyfikatorze istnieje.
    bool exists(typename Virus::id_type const &id) const {
        return (nodes.find(id) != nodes.end());
    };

    // Zwraca referencję do obiektu reprezentującego wirus o podanym
    // identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    const Virus& operator[](typename Virus::id_type const &id) const {
        auto it = nodes.find(id);
        if (it == nodes.end())
            throw VirusNotFound();
        return *(it->second->ptr);
    };

    // Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
    // powstały z wirusów o podanym identyfikatorze parent_id lub
    // podanych identyfikatorach parent_ids.
    // Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
    // id już istnieje.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
    // poprzedników nie istnieje.
    void create(typename Virus::id_type const &id, typename Virus::id_type const &parent_id) {
        typename nodes_t::iterator child_it = nodes.find(id);
        if (child_it != nodes.end())
            throw VirusAlreadyCreated();
        typename nodes_t::iterator parent_it = nodes.find(parent_id);
        if (parent_it == nodes.end())
            throw VirusNotFound();
        
        VirusNode& child = *nodes.insert({id, std::make_shared<VirusNode>(id)})
                           .first->second;
        VirusNode& parent = *parent_it->second;
        parent.childs.insert(child.ptr);
        child.parents.insert(parent.ptr);
    };

    void create(typename Virus::id_type const &id, std::vector<typename Virus::id_type> const &parent_ids) {
        typename nodes_t::iterator child_it = nodes.find(id);
        if (child_it != nodes.end())
            throw VirusAlreadyCreated();

        std::vector<typename nodes_t::iterator> parent_its;
        for (const typename Virus::id_type &parent_id : parent_ids) {
            typename nodes_t::iterator parent_it = nodes.find(parent_id);
            if (parent_it == nodes.end())
                throw VirusNotFound();
            parent_its.push_back(parent_it);
        }
        VirusNode& child = *nodes.insert({id, std::make_shared<VirusNode>(id)})
                           .first->second;
        for (typename nodes_t::iterator parent_it : parent_its) {
            VirusNode& parent = *parent_it->second;
            parent.childs.insert(child.ptr);
            child.parents.insert(parent.ptr);
        }
    }; 

    // Dodaje nową krawędź w grafie genealogii.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
    void connect(typename Virus::id_type const &child_id, typename Virus::id_type const &parent_id) {
        typename nodes_t::iterator child_it = nodes.find(child_id),
                                   parent_it = nodes.find(parent_id);
        
        if (child_it == nodes.end() || parent_it == nodes.end())
            throw VirusAlreadyCreated();
        VirusNode& child = *child_it->second, parent = *parent_it->second;
        parent.childs.insert(child.ptr);
        child.parents.insert(parent.ptr);
    };

    // Usuwa wirus o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    // Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
    // wirusa macierzystego.
    void remove(typename Virus::id_type const &id) {
        if (!exists(id))
            throw VirusNotFound();
        if (id == get_stem_id())
            throw TriedToRemoveStemVirus();
        auto virus_it = nodes[id];
        for (auto child : virus_it->childs) {
            if (nodes[child->get_id()]->parents.size() == 1) {
                remove(child->get_id());
            }
        }
        for (auto parent : virus_it->parents) {
            nodes[parent->get_id()]->childs.erase(virus_it->ptr);
        }
        nodes.erase(id);
    }
};