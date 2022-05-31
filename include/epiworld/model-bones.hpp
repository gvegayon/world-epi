#ifndef EPIWORLD_MODEL_HPP
#define EPIWORLD_MODEL_HPP

template<typename TSeq>
class Agent;

template<typename TSeq>
class Virus;

template<typename TSeq>
class Tool;

class AdjList;

template<typename TSeq>
class DataBase;

template<typename TSeq>
class Queue;

template<typename TSeq>
struct Action;

template<typename TSeq>
inline epiworld_double susceptibility_reduction_mixer_default(
    Agent<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
    );
template<typename TSeq>
inline epiworld_double transmission_reduction_mixer_default(
    Agent<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
    );
template<typename TSeq>
inline epiworld_double recovery_enhancer_mixer_default(
    Agent<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
    );
template<typename TSeq>
inline epiworld_double death_reduction_mixer_default(
    Agent<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
    );

// template<typename TSeq>
// class VirusPtr;

// template<typename TSeq>
// class ToolPtr;

/**
 * @brief Core class of epiworld.
 * 
 * The model class provides the wrapper that puts together `Agent`, `Virus`, and
 * `Tools`.
 * 
 * @tparam TSeq Type of sequence. In principle, users can build models in which
 * virus and human sequence is represented as numeric vectors (if needed.)
 */
template<typename TSeq = int>
class Model {
    friend class Agent<TSeq>;
    friend class DataBase<TSeq>;
    friend class Queue<TSeq>;
private:

    DataBase<TSeq> db = DataBase<TSeq>(*this);

    std::vector< Agent<TSeq> > population;
    bool directed = false;
    
    std::vector< VirusPtr<TSeq> > viruses;
    std::vector< epiworld_double > prevalence_virus; ///< Initial prevalence_virus of each virus
    std::vector< bool > prevalence_virus_as_proportion;
    
    std::vector< ToolPtr<TSeq> > tools;
    std::vector< epiworld_double > prevalence_tool;
    std::vector< bool > prevalence_tool_as_proportion;

    std::shared_ptr< std::mt19937 > engine =
        std::make_shared< std::mt19937 >();
    
    std::shared_ptr< std::uniform_real_distribution<> > runifd =
        std::make_shared< std::uniform_real_distribution<> >(0.0, 1.0);

    std::shared_ptr< std::normal_distribution<> > rnormd =
        std::make_shared< std::normal_distribution<> >(0.0);

    std::shared_ptr< std::gamma_distribution<> > rgammad = 
        std::make_shared< std::gamma_distribution<> >();

    std::function<void(std::vector<Agent<TSeq>>*,Model<TSeq>*,epiworld_double)> rewire_fun;
    epiworld_double rewire_prop;
        
    std::map<std::string, epiworld_double > parameters;
    unsigned int ndays;
    Progress pb;

    std::vector< UpdateFun<TSeq> >    status_fun = {};
    std::vector< std::string >        status_labels = {};
    epiworld_fast_uint nstatus = 0u;
    
    bool verbose     = true;
    bool initialized = false;
    int current_date = 0;

    void dist_tools();
    void dist_virus();

    std::chrono::time_point<std::chrono::steady_clock> time_start;
    std::chrono::time_point<std::chrono::steady_clock> time_end;

    // std::chrono::milliseconds
    std::chrono::duration<epiworld_double,std::micro> time_elapsed = 
        std::chrono::duration<epiworld_double,std::micro>::zero();
    unsigned int n_replicates = 0u;
    void chrono_start();
    void chrono_end();

    std::unique_ptr< Model<TSeq> > backup = nullptr;

    std::vector<std::function<void(Model<TSeq>*)>> global_action_functions;
    std::vector< int > global_action_dates;

    Queue<TSeq> queue;
    bool use_queuing   = true;

    /**
     * @brief Variables used to keep track of the actions
     * to be made regarding viruses.
     */
    std::vector< Action<TSeq> > actions = {};
    epiworld_fast_uint nactions = 0u;

    /**
     * @brief Construct a new Action object
     * 
     * @param agent_ Agent over which the action will be called
     * @param new_status_ New state of the agent
     * @param call_ Function the action will call
     * @param queue_ Change in the queue
     */
    void actions_add(
        Agent<TSeq> * agent_,
        VirusPtr<TSeq> virus_,
        ToolPtr<TSeq> tool_,
        epiworld_fast_uint new_status_,
        epiworld_fast_int queue_,
        ActionFun<TSeq> call_
        );

    /**
     * @brief Executes the stored action
     * 
     * @param model_ Model over which it will be executed.
     */
    void actions_run();

    /**
     * @name Tool Mixers
     * 
     * These functions combine the effects tools have to deliver
     * a single effect. For example, wearing a mask, been vaccinated,
     * and the immune system combine together to jointly reduce
     * the susceptibility for a given virus.
     * 
     */
    MixerFun<TSeq> susceptibility_reduction_mixer = susceptibility_reduction_mixer_default<TSeq>;
    MixerFun<TSeq> transmission_reduction_mixer = transmission_reduction_mixer_default<TSeq>;
    MixerFun<TSeq> recovery_enhancer_mixer = recovery_enhancer_mixer_default<TSeq>;
    MixerFun<TSeq> death_reduction_mixer = death_reduction_mixer_default<TSeq>;

public:

    std::vector<epiworld_double> array_double_tmp;
    std::vector<Virus<TSeq> * > array_virus_tmp;

    Model() {};
    Model(const Model<TSeq> & m);
    Model(Model<TSeq> && m);
    Model<TSeq> & operator=(const Model<TSeq> & m);

    void clone_population(
        std::vector< Agent<TSeq> > & p,
        bool & d,
        Model<TSeq> * m = nullptr
    ) const ;

    void clone_population(
        const Model<TSeq> & m
    );

    /**
     * @name Set the backup object
     * @details `backup` can be used to restore the entire object
     * after a run. This can be useful if the user wishes to have
     * individuals start with the same network from the beginning.
     * 
     */
    ///@{
    void set_backup();
    void restore_backup();
    ///@}

    DataBase<TSeq> & get_db();
    epiworld_double & operator()(std::string pname);

    size_t size() const;

    /**
     * @name Random number generation
     * 
     * @param eng Random number generator
     * @param s Seed
     */
    ///@{
    void set_rand_engine(std::mt19937 & eng);
    std::mt19937 * get_rand_endgine();
    void seed(unsigned int s);
    void set_rand_gamma(epiworld_double alpha, epiworld_double beta);
    epiworld_double runif();
    epiworld_double rnorm();
    epiworld_double rnorm(epiworld_double mean, epiworld_double sd);
    epiworld_double rgamma();
    epiworld_double rgamma(epiworld_double alpha, epiworld_double beta);
    ///@}

    /**
     * @name Add Virus/Tool to the model
     * 
     * This is done before the model has been initialized.
     * 
     * @param v Virus to be added
     * @param t Tool to be added
     * @param preval Initial prevalence (initial state.) It can be
     * specified as a proportion (between zero and one,) or an integer
     * indicating number of individuals.
     */
    ///@{
    void add_virus(Virus<TSeq> v, epiworld_double preval);
    void add_virus_n(Virus<TSeq> v, unsigned int preval);
    void add_tool(Tool<TSeq> t, epiworld_double preval);
    void add_tool_n(Tool<TSeq> t, unsigned int preval);
    ///@}

    /**
     * @name Accessing population of the model
     * 
     * @param fn std::string Filename of the edgelist file.
     * @param skip int Number of lines to skip in `fn`.
     * @param directed bool Whether the graph is directed or not.
     * @param size Size of the network.
     * @param al AdjList to read into the model.
     */
    ///@{
    void agents_from_adjlist(
        std::string fn,
        int size,
        int skip = 0,
        bool directed = false
        );
    void agents_from_adjlist(AdjList al);
    bool is_directed() const;
    std::vector< Agent<TSeq> > * get_agents();
    void agents_smallworld(
        unsigned int n = 1000,
        unsigned int k = 5,
        bool d = false,
        epiworld_double p = .01
        );
    ///@}

    /**
     * @name Functions to run the model
     * 
     * @param seed Seed to be used for Pseudo-RNG.
     * @param ndays Number of days (steps) of the simulation.
     * @param fun In the case of `run_multiple`, a function that is called
     * after each experiment.
     * 
     */
    ///@{
    void init(unsigned int ndays, unsigned int seed);
    void update_status();
    void mutate_variant();
    void next();
    void run(); ///< Runs the simulation (after initialization)
    void run_multiple( ///< Multiple runs of the simulation
        unsigned int nexperiments,
        std::function<void(Model<TSeq>*)> fun,
        bool reset,
        bool verbose
        );
    ///@}

    size_t get_n_variants() const;
    size_t get_n_tools() const;
    unsigned int get_ndays() const;
    unsigned int get_n_replicates() const;
    void set_ndays(unsigned int ndays);
    bool get_verbose() const;
    void verbose_off();
    void verbose_on();
    int today() const; ///< The current time of the model

    /**
     * @name Rewire the network preserving the degree sequence.
     *
     * @details This implementation assumes an undirected network,
     * thus if {(i,j), (k,l)} -> {(i,l), (k,j)}, the reciprocal
     * is also true, i.e., {(j,i), (l,k)} -> {(j,k), (l,i)}.
     * 
     * @param proportion Proportion of ties to be rewired.
     * 
     * @result A rewired version of the network.
     */
    ///@{
    void set_rewire_fun(std::function<void(std::vector<Agent<TSeq>>*,Model<TSeq>*,epiworld_double)> fun);
    void set_rewire_prop(epiworld_double prop);
    epiworld_double get_rewire_prop() const;
    void rewire();
    ///@}

    /**
     * @brief Wrapper of `DataBase::write_data`
     * 
     * @param fn_variant_info Filename. Information about the variant.
     * @param fn_variant_hist Filename. History of the variant.
     * @param fn_tool_info Filename. Information about the tool.
     * @param fn_tool_hist Filename. History of the tool.
     * @param fn_total_hist   Filename. Aggregated history (status)
     * @param fn_transmission Filename. Transmission history.
     * @param fn_transition   Filename. Markov transition history.
     */
    void write_data(
        std::string fn_variant_info,
        std::string fn_variant_hist,
        std::string fn_tool_info,
        std::string fn_tool_hist,
        std::string fn_total_hist,
        std::string fn_transmission,
        std::string fn_transition
        ) const;

    /**
     * @name Export the network data in edgelist form
     * 
     * @param fn std::string. File name.
     * @param source Integer vector
     * @param target Integer vector
     * 
     * @details When passing the source and target, the function will
     * write the edgelist on those.
     */
    ///@{
    void write_edgelist(
        std::string fn
        ) const;

    void write_edgelist(
        std::vector< unsigned int > & source,
        std::vector< unsigned int > & target
        ) const;
    ///@}

    std::map<std::string, epiworld_double> & params();

    /**
     * @brief Reset the model
     * 
     * @details Resetting the model will:
     * - clear the database
     * - restore the population (if `set_backup()` was called before)
     * - re-distribute tools
     * - re-distribute viruses
     * - set the date to 0
     * 
     */
    void reset();
    void print() const;

    Model<TSeq> && clone() const;

    /**
     * @name Manage status (states) in the model
     * 
     * @details
     * 
     * The functions `get_status` return the current values for the 
     * statuses included in the model.
     * 
     * @param lab `std::string` Name of the status.
     * 
     * @return `add_status*` returns nothing.
     * @return `get_status_*` returns a vector of pairs with the 
     * statuses and their labels.
     */
    ///@{
    void add_status(std::string lab, UpdateFun<TSeq> fun = nullptr);
    const std::vector< std::string > & get_status() const;
    const std::vector< UpdateFun<TSeq> > & get_status_fun() const;
    void print_status_codes() const;
    ///@}

    /**
     * @name Setting and accessing parameters from the model
     * 
     * @details Tools can incorporate parameters included in the model.
     * Internally, parameters in the tool are stored as pointers to
     * an std::map<> of parameters in the model. Using the `unsigned int`
     * method directly fetches the parameters in the order these were
     * added to the tool. Accessing parameters via the `std::string` method
     * involves searching the parameter directly in the std::map<> member
     * of the model (so it is not recommended.)
     * 
     * The function `set_param()` can be used when the parameter already
     * exists in the model.
     * 
     * The `par()` function members are aliases for `get_param()`.
     * 
     * @param initial_val 
     * @param pname Name of the parameter to add or to fetch
     * @return The current value of the parameter
     * in the model.
     * 
     */
    ///@{
    epiworld_double add_param(epiworld_double initial_val, std::string pname);
    epiworld_double set_param(std::string pname);
    epiworld_double get_param(unsigned int k);
    epiworld_double get_param(std::string pname);
    epiworld_double par(unsigned int k);
    epiworld_double par(std::string pname);
    epiworld_double 
        *p0,*p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8,*p9,
        *p10,*p11,*p12,*p13,*p14,*p15,*p16,*p17,*p18,*p19,
        *p20,*p21,*p22,*p23,*p24,*p25,*p26,*p27,*p28,*p29,
        *p30,*p31,*p32,*p33,*p34,*p35,*p36,*p37,*p38,*p39;
    unsigned int npar_used = 0u;
    ///@}

    void get_elapsed(
        std::string unit = "auto",
        epiworld_double * last_elapsed = nullptr,
        epiworld_double * total_elapsed = nullptr,
        std::string * unit_abbr = nullptr,
        bool print = true
    ) const;

    /**
     * @name Set the user data object
     * 
     * @param names string vector with the names of the variables.
     */
    ///[@
    void set_user_data(std::vector< std::string > names);
    void add_user_data(unsigned int j, epiworld_double x);
    void add_user_data(std::vector< epiworld_double > x);
    UserData<TSeq> & get_user_data();
    ///@}

    /**
     * @brief Set a global action
     * 
     * @param fun A function to be called on the prescribed dates
     * @param date Integer indicating when the function is called (see details)
     * 
     * @details When date is less than zero, then the function is called
     * at the end of every day. Otherwise, the function will be called only
     * at the end of the indicated date.
     */
    void add_global_action(
        std::function<void(Model<TSeq>*)> fun,
        int date = -99
        );

    void run_global_actions();

    void clear_status_set();

    /**
     * @name Queuing system
     * @details When queueing is on, the model will keep track of which agents
     * are either in risk of exposure or exposed. This then is used at each 
     * step to act only on the aforementioned agents.
     * 
     */
    ////@{
    void queuing_on(); ///< Activates the queuing system (default.)
    void queuing_off(); ///< Deactivates the queuing system.
    bool is_queuing_on() const; ///< Query if the queuing system is on.
    Queue<TSeq> & get_queue(); ///< Retrieve the `Queue` object.
    ///@}

    /**
     * @name Get the susceptibility reduction object
     * 
     * @param v 
     * @return epiworld_double 
     */
    ///@{
    void set_susceptibility_reduction_mixer(MixerFun<TSeq> fun);
    void set_transmission_reduction_mixer(MixerFun<TSeq> fun);
    void set_recovery_enhancer_mixer(MixerFun<TSeq> fun);
    void set_death_reduction_mixer(MixerFun<TSeq> fun);
    ///@}

    const std::vector< VirusPtr<TSeq> > & get_viruses() const;
    const std::vector< ToolPtr<TSeq> > & get_tools() const;

};

#endif