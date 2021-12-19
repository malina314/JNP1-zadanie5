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

        explicit VirusNode(typename Virus::id_type id) : ptr(std::make_shared<Virus>(id)) {}
    };

    std::map<typename Virus::id_type, std::unique_ptr<VirusNode>> nodes;
    typename Virus::id_type stem;

public:
    using children_iterator = typename std::set<std::shared_ptr<Virus>>::iterator;

    // Tworzy nową genealogię.
    // Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
    explicit VirusGenealogy(typename Virus::id_type const &stem_id) :
    nodes{{stem_id, std::make_unique<VirusNode>(stem_id)}}, stem(stem_id) {}

    // Zwraca identyfikator wirusa macierzystego.
    typename Virus::id_type get_stem_id() const noexcept {
        return stem;
    }

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
        return nodes.find(id) != nodes.end();
    }

    // Zwraca referencję do obiektu reprezentującego wirus o podanym
    // identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    const Virus& operator[](typename Virus::id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();
        
    }

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
    void connect(typename Virus::id_type const &child_id, virus::id_type const &parent_id);

    // Usuwa wirus o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    // Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
    // wirusa macierzystego.
    void remove(typename Virus::id_type const &id);
};