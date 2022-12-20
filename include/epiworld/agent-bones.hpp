#ifndef EPIWORLD_PERSON_BONES_HPP
#define EPIWORLD_PERSON_BONES_HPP

template<typename TSeq>
class Model;

template<typename TSeq>
class Virus;

template<typename TSeq>
class Viruses;

template<typename TSeq>
class Viruses_const;

template<typename TSeq>
class Tool;

template<typename TSeq>
class Tools;

template<typename TSeq>
class Tools_const;

template<typename TSeq>
class Queue;

template<typename TSeq>
struct Action;

template<typename TSeq>
class Entity;

template<typename TSeq>
class Entities;

template<typename TSeq>
inline void default_add_virus(Action<TSeq> & a, Model<TSeq> * m);

template<typename TSeq>
inline void default_add_tool(Action<TSeq> & a, Model<TSeq> * m);

template<typename TSeq>
inline void default_add_entity(Action<TSeq> & a, Model<TSeq> * m);

template<typename TSeq>
inline void default_rm_virus(Action<TSeq> & a, Model<TSeq> * m);

template<typename TSeq>
inline void default_rm_tool(Action<TSeq> & a, Model<TSeq> * m);

template<typename TSeq>
inline void default_rm_entity(Action<TSeq> & a, Model<TSeq> * m);



/**
 * @brief Agent (agents)
 * 
 * @tparam TSeq Sequence type (should match `TSeq` across the model)
 */
template<typename TSeq>
class Agent {
    friend class Model<TSeq>;
    friend class Virus<TSeq>;
    friend class Viruses<TSeq>;
    friend class Viruses_const<TSeq>;
    friend class Tool<TSeq>;
    friend class Tools<TSeq>;
    friend class Tools_const<TSeq>;
    friend class Queue<TSeq>;
    friend class Entities<TSeq>;
    friend class AgentsSample<TSeq>;
    friend void default_add_virus<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
    friend void default_add_tool<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
    friend void default_add_entity<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
    friend void default_rm_virus<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
    friend void default_rm_tool<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
    friend void default_rm_entity<TSeq>(Action<TSeq> & a, Model<TSeq> * m);
private:
    
    std::vector< Agent<TSeq> * > neighbors;
    std::vector< Entity<TSeq> * > entities;
    std::vector< size_t > entities_locations;
    epiworld_fast_uint n_entities = 0u;

    epiworld_fast_uint status = 0u;
    epiworld_fast_uint status_prev = 0u; ///< For accounting, if need to undo a change.
    
    int status_last_changed = -1; ///< Last time the agent was updated.
    int id = -1;
    
    std::vector< VirusPtr<TSeq> > viruses;
    epiworld_fast_uint n_viruses = 0u;

    std::vector< ToolPtr<TSeq> > tools;
    epiworld_fast_uint n_tools = 0u;

    ActionFun<TSeq> add_virus_  = default_add_virus<TSeq>;
    ActionFun<TSeq> add_tool_   = default_add_tool<TSeq>;
    ActionFun<TSeq> add_entity_ = default_add_entity<TSeq>;

    ActionFun<TSeq> rm_virus_  = default_rm_virus<TSeq>;
    ActionFun<TSeq> rm_tool_   = default_rm_tool<TSeq>;
    ActionFun<TSeq> rm_entity_ = default_rm_entity<TSeq>;
    
    epiworld_fast_uint action_counter = 0u;

    std::vector< Agent<TSeq> * > sampled_agents;
    size_t sampled_agents_n = 0u;
    std::vector< size_t > sampled_agents_left;
    size_t sampled_agents_left_n = 0u;
    int date_last_build_sample = -99;

public:

    Agent();
    Agent(Agent<TSeq> && p);
    Agent(const Agent<TSeq> & p);
    Agent<TSeq> & operator=(const Agent<TSeq> & other_agent);

    /**
     * @name Add/Remove Virus/Tool
     * 
     * Any of these is ultimately reflected at the end of the iteration.
     * 
     * @param tool Tool to add
     * @param virus Virus to add
     * @param status_new Status after the change
     * @param queue 
     */
    ///@{
    void add_tool(
        ToolPtr<TSeq> tool,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
        );

    void add_tool(
        Tool<TSeq> tool,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
        );

    void add_virus(
        VirusPtr<TSeq> virus,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
        );

    void add_virus(
        Virus<TSeq> virus,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
        );

    void add_entity(
        Entity<TSeq> & entity,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
        );

    void rm_tool(
        epiworld_fast_uint tool_idx,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_tool(
        ToolPtr<TSeq> & tool,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_virus(
        epiworld_fast_uint virus_idx,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_virus(
        VirusPtr<TSeq> & virus,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_entity(
        epiworld_fast_uint entity_idx,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_entity(
        Entity<TSeq> & entity,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    );

    void rm_agent_by_virus(
        epiworld_fast_uint virus_idx,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    ); ///< Agent removed by virus

    void rm_agent_by_virus(
        VirusPtr<TSeq> & virus,
        Model<TSeq> * model,
        epiworld_fast_int status_new = -99,
        epiworld_fast_int queue = -99
    ); ///< Agent removed by virus
    ///@}
    
    /**
     * @name Get the rates (multipliers) for the agent
     * 
     * @param v A pointer to a virus.
     * @return epiworld_double 
     */
    ///@{
    epiworld_double get_susceptibility_reduction(VirusPtr<TSeq> v, Model<TSeq> * model);
    epiworld_double get_transmission_reduction(VirusPtr<TSeq> v, Model<TSeq> * model);
    epiworld_double get_recovery_enhancer(VirusPtr<TSeq> v, Model<TSeq> * model);
    epiworld_double get_death_reduction(VirusPtr<TSeq> v, Model<TSeq> * model);
    ///@}

    int get_id() const; ///< Id of the individual

    VirusPtr<TSeq> & get_virus(int i);
    Viruses<TSeq> get_viruses();
    const Viruses_const<TSeq> get_viruses() const;
    size_t get_n_viruses() const noexcept;

    ToolPtr<TSeq> & get_tool(int i);
    Tools<TSeq> get_tools();
    const Tools_const<TSeq> get_tools() const;
    size_t get_n_tools() const noexcept;

    void mutate_variant();
    void add_neighbor(
        Agent<TSeq> * p,
        bool check_source = true,
        bool check_target = true
        );

    std::vector< Agent<TSeq> * > & get_neighbors();

    void change_status(
        Model<TSeq> * model,
        epiworld_fast_uint new_status,
        epiworld_fast_int queue = 0
        );

    const epiworld_fast_uint & get_status() const;

    void reset();

    bool has_tool(epiworld_fast_uint t) const;
    bool has_tool(std::string name) const;
    bool has_virus(epiworld_fast_uint t) const;
    bool has_virus(std::string name) const;

    void print(Model<TSeq> * model, bool compressed = false) const;

    /**
     * @brief Access the j-th column of the agent
     * 
     * If an external array has been specified, then these two
     * functions can be used to access additional agent's features 
     * not included in the model.
     * 
     * The `operator[]` method is with no boundary check, whereas
     * the `operator()` method checks boundaries. The former can result
     * in a segfault.
     * 
     * 
     * @param j 
     * @return double& 
     */
    ///@{
    // double & operator()(size_t j);
    // double & operator[](size_t j);
    ///@}

    Entities<TSeq> get_entities();
    const Entities_const<TSeq> get_entities() const;

};



#endif