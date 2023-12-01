#include "tests.hpp"

using namespace epiworld;

EPIWORLD_TEST_CASE("Cloning", "[clone]") {

    epiworld::Model<bool> m;

    m.add_state("Susceptible", default_update_susceptible<bool>);
    m.add_state("Recovered");

    epiworld::Virus<bool> v;
    epiworld::Tool<bool> t;
    v.set_state(0, 1);

    m.seed(1333);
    m.agents_smallworld(1000);

    m.add_virus(v, .5);
    m.add_tool(t, .5);

    // Cloning
    epiworld::Model<bool> m2 = m;

    // Printing the addresses
    std::cout << 
        "Model           : " << &m <<", " << &m2 << std::endl <<
        "DataBases.model : " << m.get_db().get_model() <<", " << m2.get_db().get_model() << std::endl;

    // Looking at people
    std::cout << "Neighbors agent[0] in m : " ;
    for (auto & n : m.get_agents()[0u].get_neighbors())
        std::cout << n << ", ";
    std::cout << std::endl;


    std::cout << "Neighbors agent[0] in m2 : " ;
    for (auto & n : m2.get_agents()[0u].get_neighbors())
        std::cout << n << ", ";
    std::cout << std::endl;

    // std::cout << "Agent[0] in m tools  : " <<
    //     // m.get_agents()[0u].get_virus()->get_agent() << ", " <<
    //     m.get_agents()[0u].get_tool(0u)->get_agent() << std::endl;

    // std::cout << "Agent[0] in m2 tools : " <<
    //     // m2.get_agents()[0u].get_virus()->get_agent() << ", " <<
    //     m2.get_agents()[0u].get_tool(0u)->get_agent() << std::endl;
            

    #ifndef CATCH_CONFIG_MAIN
    return 0;
    #endif

}