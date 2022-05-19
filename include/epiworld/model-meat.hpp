#ifndef EPIWORLD_MODEL_MEAT_HPP
#define EPIWORLD_MODEL_MEAT_HPP

#define CHECK_INIT() if (!initialized) \
        throw std::logic_error("Model not initialized.");

template<typename TSeq>
inline void Model<TSeq>::actions_add(
    Person<TSeq> * person_,
    epiworld_fast_uint new_status_,
    ActionFun<TSeq> call_,
    epiworld_fast_int queue_
) {
    
    if (person->locked)
        throw std::logic_error(
            "The person with id " + std::to_string(person->id) + 
            " has already an action in progress. Only one action per agent" +
            " per iteration is allowed."
            );

    // Locking the agent
    person_->locked = true;

    nactions++;

    if (nactions > actions.size())
    {

        actions.push_back(Action<TSeq>(person_, new_status_, call_, queue_));

    }
    else 
    {
        actions[nactions] = Action<TSeq>(person_, new_status_, call_, queue_);
    }

    return;

}

template<typename TSeq>
inline void Model<TSeq>::actions_run()
{
    // Making the call
    size_t nactions_ = nactions;
    for (size_t i = 0u; i < nactions_; ++i)
    {

        Action<TSeq> & a = dat[i];
        Person<TSeq> * p = a.person;

        // Applying function
        if (a.call)
        {
            a.call(p, this);
        }

        // Updating status
        if (p->status != a.new_status)
        {

            if (a.new_status >= nstatus)
                throw std::range_error(
                    "The proposed status " + std::to_string(a.new_status) + " is out of range. " +
                    "The model currently has " + std::to_string(nstatus - 1) + " statuses.");

            // Updating accounting
            db.update_accounting(p->status, a.new_status);

            for (size_t v = 0u; v < p->n_viruses; ++v)
                db.update_virus(p->viruses[v]->id, p->status, a.new_status);

            for (size_t t = 0u; t < p->n_tools; ++t)
                db.update_tool(p->tools[t]->id, p->status, a.new_status);

            p->status = a.new_status;
        }

        // Reduce the counter
        nactions--;

        // Updating queue
        if (a.queue > 0)
            queue += p;
        else if (a.queue < 0)
            queue -= p;

        // Unlocking person
        a.person->locked = false;

    }

    return;
    
}

/**
 * @name Default function for combining susceptibility_reduction levels
 * 
 * @tparam TSeq 
 * @param pt 
 * @return epiworld_double 
 */
///@{
template<typename TSeq>
inline epiworld_double susceptibility_reduction_mixer_default(
    Person<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq> * m
)
{
    epiworld_double total = 1.0;
    for (auto & tool : p->get_tools())
        total *= (1.0 - tool->get_susceptibility_reduction(v));

    return 1.0 - total;
    
}

template<typename TSeq>
inline epiworld_double transmission_reduction_mixer_default(
    Person<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
)
{
    epiworld_double total = 1.0;
    for (auto & tool : p->get_tools())
        total *= (1.0 - tool->get_transmission_reduction(v));

    return (1.0 - total);
    
}

template<typename TSeq>
inline epiworld_double recovery_enhancer_mixer_default(
    Person<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
)
{
    epiworld_double total = 1.0;
    for (auto & tool : p->get_tools())
        total *= (1.0 - tool->get_recovery_enhancer(v));

    return 1.0 - total;
    
}

template<typename TSeq>
inline epiworld_double death_reduction_mixer_default(
    Person<TSeq>* p,
    VirusPtr<TSeq> v,
    Model<TSeq>* m
)
{
    epiworld_double total = 1.0;
    for (auto & tool : p->get_tools())
    {
        total *= (1.0 - tool->get_death_reduction(v));
    } 

    return 1.0 - total;
    
}
///@}

template<typename TSeq>
inline Model<TSeq>::Model(const Model<TSeq> & model) :
    db(model.db),
    viruses(model.viruses),
    prevalence_virus(model.prevalence_virus),
    tools(model.tools),
    prevalence_tool(model.prevalence_tool),
    engine(model.engine),
    runifd(model.runifd),
    parameters(model.parameters),
    ndays(model.ndays),
    pb(model.pb),
    status_susceptible(model.status_susceptible),
    status_susceptible_labels(model.status_susceptible_labels),
    status_exposed(model.status_exposed),
    status_exposed_labels(model.status_exposed_labels),
    status_removed(model.status_removed),
    status_removed_labels(model.status_removed_labels),
    nstatus(model.nstatus),
    baseline_status_susceptible(model.baseline_status_susceptible),
    baseline_status_exposed(model.baseline_status_exposed),
    baseline_status_removed(model.baseline_status_removed),
    verbose(model.verbose),
    initialized(model.initialized),
    current_date(model.current_date),
    global_action_functions(model.global_action_functions),
    global_action_dates(model.global_action_dates),
    queue(model.queue),
    use_queuing(model.use_queuing)
{

    // Pointing to the right place
    db.set_model(*this);

    // Removing old neighbors
    model.clone_population(
        population,
        population_ids,
        directed,
        this
        );

    // Figure out the queuing
    if (use_queuing)
        queue.set_model(this);

    // Finally, seeds are resetted automatically based on the original
    // engine
    seed(floor(runif() * UINT_MAX));

}

template<typename TSeq>
inline Model<TSeq>::Model(Model<TSeq> && model) :
    db(std::move(model.db)),
    viruses(std::move(model.viruses)),
    prevalence_virus(std::move(model.prevalence_virus)),
    tools(std::move(model.tools)),
    prevalence_tool(std::move(model.prevalence_tool)),
    engine(std::move(model.engine)),
    runifd(std::move(model.runifd)),
    parameters(std::move(model.parameters)),
    ndays(std::move(model.ndays)),
    pb(std::move(model.pb)),
    verbose(std::move(model.verbose)),
    initialized(std::move(model.initialized)),
    current_date(std::move(model.current_date)),
    population(std::move(model.population)),
    population_ids(std::move(model.population_ids)),
    directed(std::move(model.directed)),
    global_action_functions(std::move(model.global_action_functions)),
    global_action_dates(std::move(model.global_action_dates)),
    status_susceptible(std::move(model.status_susceptible)),
    status_susceptible_labels(std::move(model.status_susceptible_labels)),
    status_exposed(std::move(model.status_exposed)),
    status_exposed_labels(std::move(model.status_exposed_labels)),
    status_removed(std::move(model.status_removed)),
    status_removed_labels(std::move(model.status_removed_labels)),
    baseline_status_susceptible(model.baseline_status_susceptible),
    baseline_status_exposed(model.baseline_status_exposed),
    baseline_status_removed(model.baseline_status_removed),
    nstatus(model.nstatus),
    queue(std::move(model.queue)),
    use_queuing(model.use_queuing)
{

    // // Pointing to the right place
    // db.set_model(*this);

    // // Removing old neighbors
    // model.clone_population(
    //     population,
    //     population_ids,
    //     directed,
    //     this
    //     );

}

template<typename TSeq>
inline void Model<TSeq>::clone_population(
    std::vector< Person<TSeq> > & p,
    std::map<int,int> & p_ids,
    bool & d,
    Model<TSeq> * model
) const {

    // Copy and clean
    p     = population;
    p_ids = population_ids;
    d     = directed;

    for (auto & p: p)
        p.neighbors.clear();
    
    // Relinking individuals
    for (unsigned int i = 0u; i < size(); ++i)
    {
        // Making room
        const Person<TSeq> & person_this = population[i];
        Person<TSeq> & person_res        = p[i];

        // Person pointing to the right model and person
        if (model != nullptr)
            person_res.model        = model;
        person_res.viruses.host = &person_res;
        person_res.tools.person = &person_res;

        // Readding
        std::vector< Person<TSeq> * > neigh = person_this.neighbors;
        for (unsigned int n = 0u; n < neigh.size(); ++n)
        {
            // Point to the right neighbors
            int loc = p_ids[neigh[n]->get_id()];
            person_res.add_neighbor(&p[loc], true, true);

        }

    }
}

template<typename TSeq>
inline void Model<TSeq>::clone_population(const Model<TSeq> & m)
{
    m.clone_population(
        population,
        population_ids,
        directed,
        this
    );
}

template<typename TSeq>
inline DataBase<TSeq> & Model<TSeq>::get_db()
{
    return db;
}

template<typename TSeq>
inline std::vector<Person<TSeq>> * Model<TSeq>::get_population()
{
    return &population;
}

template<typename TSeq>
inline void Model<TSeq>::population_smallworld(
    unsigned int n,
    unsigned int k,
    bool d,
    epiworld_double p
)
{
    population_from_adjlist(
        rgraph_smallworld(n, k, p, d, *this)
    );
}

template<typename TSeq>
inline void Model<TSeq>::set_rand_engine(std::mt19937 & eng)
{
    engine = std::make_shared< std::mt19937 >(eng);
}

template<typename TSeq>
inline void Model<TSeq>::set_rand_gamma(epiworld_double alpha, epiworld_double beta)
{
    rgammad = std::make_shared<std::gamma_distribution<>>(alpha,beta);
}

template<typename TSeq>
inline epiworld_double & Model<TSeq>::operator()(std::string pname) {

    if (parameters.find(pname) == parameters.end())
        throw std::range_error("The parameter "+ pname + "is not in the model.");

    return parameters[pname];

}

template<typename TSeq>
inline size_t Model<TSeq>::size() const {
    return population.size();
}

template<typename TSeq>
inline void Model<TSeq>::init(
    unsigned int ndays,
    unsigned int seed
    ) {

    if (initialized) 
        throw std::logic_error("Model already initialized.");

    // Setting up the number of steps
    this->ndays = ndays;

    // Initializing population
    for (auto & p : population)
        p.model = this;

    engine->seed(seed);
    array_double_tmp.resize(size()/2, 0.0);
    array_virus_tmp.resize(size());

    initialized = true;

    queue.set_model(this);

    // Checking whether the proposed status in/out/removed
    // are valid
    int _init, _end, _removed;
    for (auto & v : viruses)
    {
        v->get_status(&_init, &_end, &_removed)
        
        // Negative unspecified status
        if (((_init != -99) && (_init < 0)) || (_init >= nstatus))
            throw std::range_error("Statuses must be between 0 and " +
                std::to_string(nstatus - 1));

        // Negative unspecified status
        if (((_end != -99) && (_end < 0)) || (_end >= nstatus))
            throw std::range_error("Statuses must be between 0 and " +
                std::to_string(nstatus - 1));

        if (((_removed != -99) && (_removed < 0)) || (_removed >= nstatus))
            throw std::range_error("Statuses must be between 0 and " +
                std::to_string(nstatus - 1));
        
    }

    for (auto & t : tools)
    {
        t->get_status(&_init, &_end)
        
        // Negative unspecified status
        if (((_init != -99) && (_init < 0)) || (_init >= nstatus))
            throw std::range_error("Statuses must be between 0 and " +
                std::to_string(nstatus - 1));

        // Negative unspecified status
        if (((_end != -99) && (_end < 0)) || (_end >= nstatus))
            throw std::range_error("Statuses must be between 0 and " +
                std::to_string(nstatus - 1));

    }

    // Starting first infection and tools
    reset();



}

template<typename TSeq>
inline void Model<TSeq>::dist_virus()
{


    // Starting first infection
    int n = size();
    std::vector< size_t > idx(n);
    for (unsigned int v = 0; v < viruses.size(); ++v)
    {
        // Picking how many
        int nsampled;
        if (prevalence_virus_as_proportion[v])
        {
            nsampled = static_cast<int>(std::floor(prevalence_virus[v] * size()));
        }
        else
        {
            nsampled = static_cast<int>(prevalence_virus[v]);
        }

        if (nsampled > static_cast<int>(size()))
            throw std::range_error("There are only " + std::to_string(size()) + 
            " individuals in the population. Cannot add the virus to " + std::to_string(nsampled));


        VirusPtr<TSeq> virus = viruses[v];
        
        int n_left = n;
        std::iota(idx.begin(), idx.end(), 0);
        while (nsampled > 0)
        {

            int loc = static_cast<unsigned int>(floor(runif() * (n_left--)));

            Person<TSeq> & person = population[idx[loc]];
            
            // Adding action
            person.add_virus(virus, virus->status_init, virus->queue_init);

            // Adjusting sample
            nsampled--;
            std::switch(idx[loc], idx[n_left]);


        }
    }      

    // Apply the actions
    actions_run();

}

template<typename TSeq>
inline void Model<TSeq>::dist_tools()
{

    // Starting first infection
    int n = size();
    std::vector< size_t > idx(n);
    for (unsigned int t = 0; t < tools.size(); ++t)
    {
        // Picking how many
        int nsampled;
        if (prevalence_tool_as_proportion[t])
        {
            nsampled = static_cast<int>(std::floor(prevalence_tool[t] * size()));
        }
        else
        {
            nsampled = static_cast<int>(prevalence_tool[t]);
        }

        if (nsampled > static_cast<int>(size()))
            throw std::range_error("There are only " + std::to_string(size()) + 
            " individuals in the population. Cannot add the tool to " + std::to_string(nsampled));
        
        ToolPtr<TSeq> tool = tools[t];

        int n_left = n;
        std::iota(idx.begin(), idx.end(), 0);
        while (nsampled > 0)
        {
            int loc = static_cast<unsigned int>(floor(runif() * n_left--));
            
            population[idx[loc]].add_tool(tool, tool->state_init, tool->queue_init);
            
            nsampled--;

            std::switch(idx[loc], idx[n_left]);

        }
    }

    // Apply the actions
    actions_run();

}

template<typename TSeq>
inline void Model<TSeq>::chrono_start() {
    time_start = std::chrono::steady_clock::now();
}

template<typename TSeq>
inline void Model<TSeq>::chrono_end() {
    time_end = std::chrono::steady_clock::now();
    time_elapsed += (time_end - time_start);
    n_replicates++;
}

template<typename TSeq>
inline void Model<TSeq>::set_backup()
{

    backup = std::unique_ptr<Model<TSeq>>(new Model<TSeq>(*this));

}

template<typename TSeq>
inline void Model<TSeq>::restore_backup()
{

    if (backup != nullptr)
    {

        clone_population(*backup);

        db = backup->db;
        db.set_model(*this);

    }

}

template<typename TSeq>
inline std::mt19937 * Model<TSeq>::get_rand_endgine()
{
    return engine.get();
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::runif() {
    // CHECK_INIT()
    return runifd->operator()(*engine);
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::rnorm() {
    // CHECK_INIT()
    return (rnormd->operator()(*engine));
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::rnorm(epiworld_double mean, epiworld_double sd) {
    // CHECK_INIT()
    return (rnormd->operator()(*engine)) * sd + mean;
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::rgamma() {
    return rgammad->operator()(*engine);
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::rgamma(epiworld_double alpha, epiworld_double beta) {
    auto old_param = rgammad->param();
    rgammad->param(std::gamma_distribution<>::param_type(alpha, beta));
    epiworld_double ans = rgammad->operator()(*engine);
    rgammad->param(old_param);
    return ans;
}

template<typename TSeq>
inline void Model<TSeq>::seed(unsigned int s) {
    this->engine->seed(s);
}

template<typename TSeq>
inline void Model<TSeq>::add_virus(Virus<TSeq> v, epiworld_double preval)
{

    if (preval > 1.0)
        throw std::range_error("Prevalence of virus cannot be above 1.0");

    if (preval < 0.0)
        throw std::range_error("Prevalence of virus cannot be negative");

    // Setting the id
    v.set_id(viruses.size());
    
    // Adding new virus
    viruses.push_back(std::make_shared< Virus<TSeq> >(v));
    prevalence_virus.push_back(preval);
    prevalence_virus_as_proportion.push_back(true);

}

template<typename TSeq>
inline void Model<TSeq>::add_virus_n(Virus<TSeq> v, unsigned int preval)
{

    // Setting the id
    v.set_id(viruses.size());

    // Adding new virus
    viruses.push_back(std::make_shared< Virus<TSeq> >(v));
    prevalence_virus.push_back(preval);
    prevalence_virus_as_proportion.push_back(false);

}

template<typename TSeq>
inline void Model<TSeq>::add_tool(Tool<TSeq> t, epiworld_double preval)
{

    if (preval > 1.0)
        throw std::range_error("Prevalence of tool cannot be above 1.0");

    if (preval < 0.0)
        throw std::range_error("Prevalence of tool cannot be negative");

    t.id = tools.size();
    tools.push_back(std::make_shared< Tool<TSeq> >(t));
    prevalence_tool.push_back(preval);
    prevalence_tool_as_proportion.push_back(true);

}

template<typename TSeq>
inline void Model<TSeq>::add_tool_n(Tool<TSeq> t, unsigned int preval)
{
    t.id = tools.size();
    tools.push_back(t);
    prevalence_tool.push_back(preval);
    prevalence_tool_as_proportion.push_back(false);
}

template<typename TSeq>
inline void Model<TSeq>::population_from_adjlist(
    std::string fn,
    int skip,
    bool directed,
    int min_id,
    int max_id
    ) {

    AdjList al;
    al.read_edgelist(fn, skip, directed, min_id, max_id);
    this->population_from_adjlist(al);

}

template<typename TSeq>
inline void Model<TSeq>::population_from_adjlist(AdjList al) {

    // Resizing the people
    population.clear();
    population_ids.clear();
    population.resize(al.vcount(), Person<TSeq>());

    const auto & tmpdat = al.get_dat();
    
    int loc;
    for (const auto & n : tmpdat)
    {
        if (population_ids.find(n.first) == population_ids.end())
            population_ids[n.first] = population_ids.size();

        loc = population_ids[n.first];

        population[loc].model = this;
        population[loc].id    = n.first;
        population[loc].index = loc;

        for (const auto & link: n.second)
        {

            if (population_ids.find(link.first) == population_ids.end())
                population_ids[link.first] = population_ids.size();

            unsigned int loc_link   = population_ids[link.first];
            population[loc_link].id    = link.first;
            population[loc_link].index = loc_link;

            population[loc].add_neighbor(
                &population[population_ids[link.first]],
                true, true
                );

        }

    }

}

template<typename TSeq>
inline bool Model<TSeq>::is_directed() const
{
    if (population.size() == 0u)
        throw std::logic_error("The population hasn't been initialized.");

    return directed;
}

template<typename TSeq>
inline int Model<TSeq>::today() const {
    return this->current_date;
}

template<typename TSeq>
inline void Model<TSeq>::next() {

    ++this->current_date;
    db.record();
    
    // Advancing the progress bar
    if (verbose)
        pb.next();

    #ifdef EPI_DEBUG
    // A possible check here
    #endif

    return ;
}

template<typename TSeq>
inline void Model<TSeq>::run() 
{

    // Initializing the simulation
    chrono_start();
    EPIWORLD_RUN((*this))
    {

        // We can execute these components in whatever order the
        // user needs.
        this->update_status();       
    
        // We start with the global actions
        this->run_global_actions();

        // In this case we are applying degree sequence rewiring
        // to change the network just a bit.
        this->rewire();

        // This locks all the changes
        this->next();

        // Mutation must happen at the very end of all
        this->mutate_variant();

    }
    chrono_end();

}

template<typename TSeq>
inline void Model<TSeq>::run_multiple(
    unsigned int nexperiments,
    std::function<void(Model<TSeq>*)> fun,
    bool reset,
    bool verbose
)
{

    if (reset)
        set_backup();

    bool old_verb = this->verbose;
    verbose_off();

    Progress pb_multiple(
        nexperiments,
        EPIWORLD_PROGRESS_BAR_WIDTH
        )
        ;
    if (verbose)
    {

        printf_epiworld(
            "Starting multiple runs (%i)\n", 
            static_cast<int>(nexperiments)
        );

        pb_multiple.start();

    }

    for (unsigned int n = 0u; n < nexperiments; ++n)
    {
        
        run();

        fun(this);

        if (n < (nexperiments - 1u) && reset)
            this->reset();

        if (verbose)
            pb_multiple.next();
    
    }

    if (verbose)
        pb_multiple.end();

    if (old_verb)
        verbose_on();

    return;

}

template<typename TSeq>
inline void Model<TSeq>::update_status() {

    // Next status
    if (use_queuing)
    {
        
        for (unsigned int p = 0u; p < size(); ++p)
            if (queue[p] > 0)
            {
                if (status_fun[population[p]->status])
                    status_fun[population[p]->status](&population[p], this);
            }

    }
    else
    {

        for (auto & p: population)
            if (status_fun[population[p]->status])
                    status_fun[population[p]->status](&population[p], this);

    }

    actions_run();
    
}

template<typename TSeq>
inline void Model<TSeq>::mutate_variant() {

    for (auto & p: population)
    {

        if (p.n_viruses > 0u)
            for (auto & v : p.viruses)
                v->mutate();

    }

}

template<typename TSeq>
inline void Model<TSeq>::record_variant(VirusPtr<TSeq> v) {

    // Updating registry
    db.record_variant(v);
    return;
    
} 

template<typename TSeq>
inline int Model<TSeq>::get_nvariants() const {
    return db.size();
}

template<typename TSeq>
inline unsigned int Model<TSeq>::get_ndays() const {
    return ndays;
}

template<typename TSeq>
inline unsigned int Model<TSeq>::get_n_replicates() const
{
    return n_replicates;
}

template<typename TSeq>
inline void Model<TSeq>::set_ndays(unsigned int ndays) {
    this->ndays = ndays;
}

template<typename TSeq>
inline bool Model<TSeq>::get_verbose() const {
    return verbose;
}

template<typename TSeq>
inline void Model<TSeq>::verbose_on() {
    verbose = true;
}

template<typename TSeq>
inline void Model<TSeq>::verbose_off() {
    verbose = false;
}

template<typename TSeq>
inline void Model<TSeq>::set_rewire_fun(
    std::function<void(std::vector<Person<TSeq>>*,Model<TSeq>*,epiworld_double)> fun
    ) {
    rewire_fun = fun;
}

template<typename TSeq>
inline void Model<TSeq>::set_rewire_prop(epiworld_double prop)
{

    if (prop < 0.0)
        throw std::range_error("Proportions cannot be negative.");

    if (prop > 1.0)
        throw std::range_error("Proportions cannot be above 1.0.");

    rewire_prop = prop;
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::get_rewire_prop() const {
    return rewire_prop;
}

template<typename TSeq>
inline void Model<TSeq>::rewire() {

    if (rewire_fun)
        rewire_fun(&population, this, rewire_prop);
}


template<typename TSeq>
inline void Model<TSeq>::write_data(
    std::string fn_variant_info,
    std::string fn_variant_hist,
    std::string fn_total_hist,
    std::string fn_transmission,
    std::string fn_transition
    ) const
{

    db.write_data(fn_variant_info,fn_variant_hist,fn_total_hist,fn_transmission,fn_transition);

}

template<typename TSeq>
inline void Model<TSeq>::write_edgelist(
    std::string fn
    ) const
{



    std::ofstream efile(fn, std::ios_base::out);
    efile << "source target\n";
    for (const auto & p : population)
    {
        for (auto & n : p.neighbors)
            efile << p.id << " " << n->id << "\n";
    }



}

template<typename TSeq>
inline std::map<std::string,epiworld_double> & Model<TSeq>::params()
{
    return parameters;
}

template<typename TSeq>
inline void Model<TSeq>::reset() {
    
    // Restablishing people
    pb = Progress(ndays, 80);

    if (backup != nullptr)
    {
        backup->clone_population(
            population,
            population_ids,
            directed,
            this
        );
    }

    for (auto & p : population)
    {
        p.reset();
        
        if (update_susceptible)
            p.set_update_susceptible(update_susceptible);
        else if (!p.update_susceptible)
            throw std::logic_error("No update_susceptible function set.");
        if (update_exposed)
            p.set_update_exposed(update_exposed);
        else if (!p.update_exposed)
            throw std::logic_error("No update_exposed function set.");
        if (update_removed)
            p.set_update_removed(update_removed);
        
    }
    
    current_date = 0;

    db.set_model(*this);

    // Recording variants
    for (Virus<TSeq> & v : viruses)
        record_variant(&v);

    if (use_queuing)
        queue.set_model(this);

    // Re distributing tools and virus
    dist_virus();
    dist_tools();

    // Recording the original state
    db.record();

}

// Too big to keep here
#include "model-meat-print.hpp"

template<typename TSeq>
inline Model<TSeq> && Model<TSeq>::clone() const {

    // Step 1: Regen the individuals and make sure that:
    //  - Neighbors point to the right place
    //  - DB is pointing to the right place
    Model<TSeq> res(*this);

    // Pointing to the right place
    res.get_db().set_model(res);

    // Removing old neighbors
    for (auto & p: res.population)
        p.neighbors.clear();
    
    // Rechecking individuals
    for (unsigned int p = 0u; p < size(); ++p)
    {
        // Making room
        const Person<TSeq> & person_this = population[p];
        Person<TSeq> & person_res  = res.population[p];

        // Person pointing to the right model and person
        person_res.model        = &res;
        person_res.viruses.host = &person_res;
        person_res.tools.person = &person_res;

        // Readding
        std::vector< Person<TSeq> * > neigh = person_this.neighbors;
        for (unsigned int n = 0u; n < neigh.size(); ++n)
        {
            // Point to the right neighbors
            int loc = res.population_ids[neigh[n]->get_id()];
            person_res.add_neighbor(&res.population[loc], true, true);

        }

    }

    return res;

}



template<typename TSeq>
inline void Model<TSeq>::add_status(
    std::string lab, 
    UpdateFun<TSeq> fun
)
{
    if (this->initialized)
        throw std::logic_error("Cannot add status once the model has been initialized.");

    // Checking it doesn't match
    for (auto & s : status_labels)
        if (s == lab)
            throw std::logic_error("Status \"" + s + "\" already registered.");

    status_labels.push_back(lab);
    status_fun(fun);
    nstatus++;

}


template<typename TSeq>
inline const std::vector< std::string > &
Model<TSeq>::get_status() const
{
    return status;
}

template<typename TSeq>
inline const std::vector< UpdateFun<TSeq> > &
Model<TSeq>::get_status_fun() const
{
    return status_fun;
}

template<typename TSeq>
inline void Model<TSeq>::print_status_codes() const
{

    // Horizontal line
    std::string line = "";
    for (unsigned int i = 0u; i < 80u; ++i)
        line += "_";

    printf_epiworld("\n%s\nSTATUS CODES\n\n", line.c_str());

    unsigned int nchar = 0u;
    for (auto & p : status_labels)
        if (p.length() > nchar)
            nchar = p.length();
    
    std::string fmt = " %2i = %-" + std::to_string(nchar + 1 + 4) + "s\n";
    for (unsigned int i = 0u; i < status.size(); ++i)
    {

        printf_epiworld(
            fmt.c_str(),
            status[i],
            (status_labels[i] + " (S)").c_str()
        );

    }

}

#define CASE_PAR(a,b) case a: b = &(parameters[pname]);break;
#define CASES_PAR(a) \
    switch (a) \
    { \
    CASE_PAR(0u, p0) CASE_PAR(1u, p1) CASE_PAR(2u, p2) CASE_PAR(3u, p3) \
    CASE_PAR(4u, p4) CASE_PAR(5u, p5) CASE_PAR(6u, p6) CASE_PAR(7u, p7) \
    CASE_PAR(8u, p8) CASE_PAR(9u, p9) \
    CASE_PAR(10u, p10) CASE_PAR(11u, p11) CASE_PAR(12u, p12) CASE_PAR(13u, p13) \
    CASE_PAR(14u, p14) CASE_PAR(15u, p15) CASE_PAR(16u, p16) CASE_PAR(17u, p17) \
    CASE_PAR(18u, p18) CASE_PAR(19u, p19) \
    CASE_PAR(20u, p20) CASE_PAR(21u, p21) CASE_PAR(22u, p22) CASE_PAR(23u, p23) \
    CASE_PAR(24u, p24) CASE_PAR(25u, p25) CASE_PAR(26u, p26) CASE_PAR(27u, p27) \
    CASE_PAR(28u, p28) CASE_PAR(29u, p29) \
    CASE_PAR(30u, p30) CASE_PAR(31u, p31) CASE_PAR(32u, p22) CASE_PAR(33u, p23) \
    CASE_PAR(34u, p34) CASE_PAR(35u, p35) CASE_PAR(36u, p26) CASE_PAR(37u, p27) \
    CASE_PAR(38u, p38) CASE_PAR(39u, p39) \
    default: \
        break; \
    }

template<typename TSeq>
inline epiworld_double Model<TSeq>::add_param(
    epiworld_double initial_value,
    std::string pname
    ) {

    if (parameters.find(pname) == parameters.end())
        parameters[pname] = initial_value;

    CASES_PAR(npar_used++)
    
    return initial_value;

}

template<typename TSeq>
inline epiworld_double Model<TSeq>::set_param(
    std::string pname
    ) {

    if (parameters.find(pname) == parameters.end())
        throw std::logic_error("The parameter " + pname + " does not exists.");

    CASES_PAR(npar_used++)

    return parameters[pname];
    
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::get_param(std::string pname)
{
    if (parameters.find(pname) == parameters.end())
        throw std::logic_error("The parameter " + pname + " does not exists.");

    return parameters[pname];
}

template<typename TSeq>
inline epiworld_double Model<TSeq>::par(std::string pname)
{
    return parameters[pname];
}

#define DURCAST(tunit,txtunit) {\
        elapsed       = std::chrono::duration_cast<std::chrono:: tunit>(\
            time_end - time_start).count(); \
        elapsed_total = std::chrono::duration_cast<std::chrono:: tunit>(time_elapsed).count(); \
        abbr_unit     = txtunit;}

template<typename TSeq>
inline void Model<TSeq>::get_elapsed(
    std::string unit,
    epiworld_double * last_elapsed,
    epiworld_double * total_elapsed,
    std::string * unit_abbr,
    bool print
) const {

    // Preparing the result
    epiworld_double elapsed, elapsed_total;
    std::string abbr_unit;

    // Figuring out the length
    if (unit == "auto")
    {

        size_t tlength = std::to_string(
            static_cast<int>(floor(time_elapsed.count()))
            ).length();
        
        if (tlength <= 1)
            unit = "nanoseconds";
        else if (tlength <= 3)
            unit = "microseconds";
        else if (tlength <= 6)
            unit = "milliseconds";
        else if (tlength <= 8)
            unit = "seconds";
        else if (tlength <= 9)
            unit = "minutes";
        else 
            unit = "hours";

    }

    if (unit == "nanoseconds")       DURCAST(nanoseconds,"ns")
    else if (unit == "microseconds") DURCAST(microseconds,"\xC2\xB5s")
    else if (unit == "milliseconds") DURCAST(milliseconds,"ms")
    else if (unit == "seconds")      DURCAST(seconds,"s")
    else if (unit == "minutes")      DURCAST(minutes,"m")
    else if (unit == "hours")        DURCAST(hours,"h")
    else
        throw std::range_error("The time unit " + unit + " is not supported.");


    if (last_elapsed != nullptr)
        *last_elapsed = elapsed;
    if (total_elapsed != nullptr)
        *total_elapsed = elapsed_total;
    if (unit_abbr != nullptr)
        *unit_abbr = abbr_unit;

    if (!print)
        return;

    if (n_replicates > 1u)
    {
        printf_epiworld("last run elapsed time : %.2f%s\n",
            elapsed, abbr_unit.c_str());
        printf_epiworld("total elapsed time    : %.2f%s\n",
            elapsed_total, abbr_unit.c_str());
        printf_epiworld("total runs            : %i\n",
            static_cast<int>(n_replicates));
        printf_epiworld("mean run elapsed time : %.2f%s\n",
            elapsed_total/static_cast<epiworld_double>(n_replicates), abbr_unit.c_str());

    } else {
        printf_epiworld("last run elapsed time : %.2f%s.\n", elapsed, abbr_unit.c_str());
    }
}

template<typename TSeq>
inline void Model<TSeq>::set_user_data(std::vector< std::string > names)
{
    db.set_user_data(names);
}

template<typename TSeq>
inline void Model<TSeq>::add_user_data(unsigned int j, epiworld_double x)
{
    db.add_user_data(j, x);
}

template<typename TSeq>
inline void Model<TSeq>::add_user_data(std::vector<epiworld_double> x)
{
    db.add_user_data(x);
}

template<typename TSeq>
inline UserData<TSeq> & Model<TSeq>::get_user_data()
{
    return db.get_user_data();
}

template<typename TSeq>
inline void Model<TSeq>::add_global_action(
    std::function<void(Model<TSeq>*)> fun,
    int date
)
{

    global_action_functions.push_back(fun);
    global_action_dates.push_back(date);

}

template<typename TSeq>
inline void Model<TSeq>::run_global_actions()
{

    for (unsigned int i = 0u; i < global_action_dates.size(); ++i)
    {

        if (global_action_dates[i] < 0)
        {

            global_action_functions[i](this);

        }
        else if (global_action_dates[i] == today())
        {

            global_action_functions[i](this);

        }

        actions_run();

    }

}

template<typename TSeq>
inline void Model<TSeq>::queuing_on()
{
    use_queuing = true;
}

template<typename TSeq>
inline void Model<TSeq>::queuing_off()
{
    use_queuing = false;
}

template<typename TSeq>
inline bool Model<TSeq>::is_queuing_on() const
{
    return use_queuing;
}

template<typename TSeq>
inline Queue<TSeq> & Model<TSeq>::get_queue()
{
    return queue;
}

#undef DURCAST

#undef CASES_PAR
#undef CASE_PAR

#undef CHECK_INIT
#endif