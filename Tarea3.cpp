#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <unordered_map>
#include <set>
#include <forward_list>
#include <list>
#include <string>
#include <functional>

using namespace std;

// Tarea 3 de Fabrizio Enrique Leiva Novoa

class Entity {          // Instancia entity
    string nombre;
    int x, y;
    int vida;
    int nivel;
    friend ostream& operator<<(ostream& os, const Entity& e);   // Función amiga para el comando de imprimir
public:
    Entity(string n): nombre(n), x(0), y(0), vida(100), nivel(1) {}   // Constructor (Solo el nombre de entrada)

    // Registro de los Comandos

    void move(int _x, int _y) {x += _x; y += _y;}
    void heal(int v) {vida += v; if (vida > 100) vida = 100;}   // Es curación, no sobrepasa la vida máxima
    void damage(int d) {vida -= d; if (vida < 0) vida = 0;}     // No existe vida negativa
    void reset() {x = y = 0; vida = 100; nivel = 1;}
};

ostream& operator<<(ostream& os, const Entity& e) {     // Sobrecarga para imprimir resultados
    os << "Nombre: " << e.nombre << ", "
       << "Posicion: (" << e.x << "," << e.y << "), "
       << "Vida :" << e.vida << ", "
       << "Nivel :" << e.nivel;
    return os;
}   // En una línea para que no ocupe tanto espacio

// El Comando de función libre
void cmd_move(Entity& e, const list<string>& args) {
    if (args.size() != 2) {     // Se valida la cantidad de argumentos
        cout << "Uso: move x y\n";
        return;
    }
    auto it = args.begin();
    int dx = stoi(*it++);
    int dy = stoi(*it);
    e.move(dx, dy);
}

// Comando con Functor
class DamageCommand {
    Entity& entity;
    int contador = 0;
public:
    DamageCommand(Entity& e) : entity(e) {}

    void operator()(const list<string>& args) {
        if (args.size() != 1) {     // Se valida la cantidad de argumentos
            cout << "Uso: damage" << endl;
            return;
        }
        int d = stoi(args.front());
        entity.damage(d);
        contador++;
        cout << "Damage ejecutado " << contador << " veces" << endl;
    }
};

class CommandCenter {
public:
    using Command = function<void(const list<string>&)>;
private:
    list<pair<Entity, Entity>> historial;   // Se coloca dentro del command center para que los registre
    unordered_map<string, Command> comandos;    // No se si cambia algo que esté desordenado, pero funciona
    unordered_map<string, list<pair<string, list<string>>>> macros;
    Entity& entity;
public:
    CommandCenter(Entity& e) : entity(e) {}

    void registerCommand(const string& n, Command c) {comandos[n] = c;}

    void removeCommand(const string& name) {    // Eliminación dinámica de los comandos
        auto it = comandos.find(name);
        if (it != comandos.end())
            comandos.erase(it);
    }

    void execute(const string& name, const list<string>& args) {
        auto it = comandos.find(name);
        if (it == comandos.end()) {     // Mensaje para la implementación de comandos inválidos
            cout << "ERROR: Comando '" << name << "' no existe\n";
            return;
        }

        Entity antes = entity;  // Impacto sobre la entidad: Antes y despues
        it->second(args);
        Entity despues = entity;

        historial.push_back(make_pair(antes, despues));
    }

    void printHistory() const {     // Mostrar el historial de antes y después
        cout << endl << "Historial" << endl << endl;
        for (auto it = historial.begin(); it != historial.end(); ++it) {
            cout << "ANTES: " << it->first << "   DESPUES: " << it->second << endl;
        }
    }

    // Comandos Compuestos (Macros)
    void registerMacro(
        const string& name,
        const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it = macros.find(name);
        if (it == macros.end()) {       // Por si se registra un macro no registrado
            cout << "ERROR: Macro '" << name << "' no existe" << endl;
            return;
        }

        for (auto stepIt = it->second.begin();
             stepIt != it->second.end(); ++stepIt) {

            auto cmdIt = comandos.find(stepIt->first);
            if (cmdIt == comandos.end()) {      // Por si el macro existe; pero el comando dentro, no
                cout << "ERROR en macro: comando '"
                     << stepIt->first << "' no existe" << endl;
                return;
            }
            execute(stepIt->first, stepIt->second);
        }
    }
};

int main() {

    // Creación de la entidad
    Entity personaje("Hero");
    // Creación del Command Center
    CommandCenter comandos(personaje);

    // Creación del registro de comandos

    // Uso de la función libre
    comandos.registerCommand("move",[&](const list<string>& args) {
            cmd_move(personaje, args);
    });

    // Uso de la función Lambda
    comandos.registerCommand("heal",[&](const list<string>& args) {
            if (args.size() != 1) {     // Se valida la cantidad de argumentos
                cout << "Uso: heal n\n";
                return;
            }
            personaje.heal(stoi(args.front()));
    });

    DamageCommand dmg(personaje);     // El comando que usa functor
    comandos.registerCommand("damage", dmg);

    comandos.registerCommand("reset",
        [&](const list<string>&) {
            personaje.reset();
        });

    comandos.registerCommand("status",
        [&](const list<string>&) {
            cout << personaje << endl;
        });

    // Definir los macros los 3 macros que se pide
    comandos.registerMacro("heal_and_status", {     // Curar y estatus
        {"heal", {"20"}},
        {"status", {}}
    });

    comandos.registerMacro("batalla", {      // Simula batalla y pequeña curación (y muestra)
        {"damage", {"30"}},
        {"heal", {"10"}},
        {"status", {}}
    });

    comandos.registerMacro("restart", {     // Resetea y lo muestra
        {"reset", {}},
        {"status", {}}
    });

    comandos.registerMacro("error3", {     // Diseñado para fallar
        {"invalido3", {}},
        {"status", {}}
    });

    // Implementación Final

    // T-odo funciona con execute (No me deja colocar "to.do" bien porque se pone en amarillo) (4 comandos)
    comandos.execute("status", {});
    comandos.execute("move", {"5", "3"});
    comandos.execute("heal", {"15"});
    comandos.execute("damage", {"40"});

    // T-odo funciona con executeMacro (los 3 macros)
    comandos.executeMacro("batalla");
    comandos.executeMacro("heal_and_status");
    comandos.executeMacro("restart");

    // Para mostrar que pasa si se ejecuta un comando inválido (Los 3 casos de error)
    comandos.execute("invalido", {});
    comandos.executeMacro("invalido2");
    comandos.executeMacro("error3");

    comandos.printHistory();

    return 0;
}
