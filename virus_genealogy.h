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
    virtual const char* what() const noexcept {return "VirusAlreadyCreated\n";}
};

// todo: to do wywalenia
class Virus {
public:
    using id_type = int;
};

template<typename Virus>
class VirusGenealogy {
private:
    struct VirusNode {
        std::shared_ptr<Virus> ptr;
        std::set<std::shared_ptr<Virus>> kids;
        std::set<std::shared_ptr<Virus>> parents;

        VirusNode(typename Virus::id_type id) : ptr(std::make_shared<Virus>(id)) {}
    };

    std::map<typename Virus::id_type, std::unique_ptr<VirusNode>> nodes;

public:
    using children_iterator = set<std::shared_ptr<Virus>>::iterator;

    // Tworzy nową genealogię.
    // Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
    VirusGenealogy(typename Virus::id_type const &stem_id) :
    nodes{{stem_id, std::make_unique<VirusNode>(stem_id)}} {}

    // Zwraca identyfikator wirusa macierzystego.
    typename Virus::id_type get_stem_id() const;

    // Zwraca iterator pozwalający przeglądać listę identyfikatorów
    // bezpośrednich następników wirusa o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    // Iterator musi spełniać koncept bidirectional_iterator oraz
    // typeid(*v.get_children_begin()) == typeid(const Virus &).
    VirusGenealogy<Virus>::children_iterator get_children_begin(typename Virus::id_type const &id) const;

    // Iterator wskazujący na element za końcem wyżej wspomnianej listy.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    VirusGenealogy<Virus>::children_iterator get_children_end(typename Virus::id_type const &id) const;

    // Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
    // o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const;

    // Sprawdza, czy wirus o podanym identyfikatorze istnieje.
    bool exists(typename Virus::id_type const &id) const {

    }

    // Zwraca referencję do obiektu reprezentującego wirus o podanym
    // identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    const Virus& operator[](typename Virus::id_type const &id) const;

    // Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
    // powstały z wirusów o podanym identyfikatorze parent_id lub
    // podanych identyfikatorach parent_ids.
    // Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
    // id już istnieje.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
    // poprzedników nie istnieje.
    void create(typename Virus::id_type const &id, typename Virus::id_type const &parent_id) {
        if (nodes.find(id) != nodes.end())
            throw VirusAlreadyCreated();
        if (nodes.find(parent_id) != nodes.end())
            throw VirusNotFound();
        nodes.insert({id, std::make_unique<Virus>(id)});
        
    };
    void create(typename Virus::id_type const &id, std::vector<typename Virus::id_type> const &parent_ids);

    // Dodaje nową krawędź w grafie genealogii.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
    void connect(typename Virus::id_type const &child_id, typename Virus::id_type const &parent_id) {
        if (!exists(child_id) || !exists(parent_id))
            throw VirusNotFound();
        auto child = nodes[child_id], parent = nodes[parent_id];
        parent->kids.insert(child->ptr);
        child->parents.insert(parent->ptr);
    }

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
        for (auto kid : virus_it->kids) {
            if (nodes[kid.get_id()]->parents.size() == 1) {
                remove(kid.get_id());
            }
        }
        for (auto parent : virus_it->parents) {
            nodes[parent.get_id()]->kids.erase(virus_it->ptr);
        }
        nodes.erase(id);
    }
};

// remove
// connect
// exists
// get_children_begin
// get_children_end