
#ifndef EPIWORLD_TOOLS_MEAT_HPP
#define EPIWORLD_TOOLS_MEAT_HPP

#define DEFAULT_EFFICACY        0.0
#define DEFAULT_TRANSMISIBILITY 1.0
#define DEFAULT_RECOVERY        0.0
#define DEFAULT_DEATH           1.0

template<typename TSeq>
inline Tool<TSeq>::Tool(TSeq d) {
    sequence = std::make_shared<TSeq>(d);
}

template<typename TSeq>
inline void Tool<TSeq>::set_sequence(TSeq d) {
    sequence = std::make_shared<TSeq>(d);
}

template<typename TSeq>
inline void Tool<TSeq>::set_sequence_unique(TSeq d) {
    sequence_unique = d;
}

template<typename TSeq>
inline void Tool<TSeq>::set_sequence(std::shared_ptr<TSeq> d) {
    sequence = d;
}

template<typename TSeq>
inline std::shared_ptr<TSeq> Tool<TSeq>::get_sequence() {
    return sequence;
}

template<typename TSeq>
inline TSeq & Tool<TSeq>::get_sequence_unique() {
    return sequence_unique;
}

template<typename TSeq>
inline double Tool<TSeq>::get_efficacy(
    Virus<TSeq> * v
) {

    if (!efficacy)
        return DEFAULT_EFFICACY;
    
    return efficacy(this, person, v, person->model);

}

template<typename TSeq>
inline double Tool<TSeq>::get_transmisibility(
    Virus<TSeq> * v
) {

    if (transmisibility)
        return transmisibility(this, this->person, v, person->get_model());
    
    return DEFAULT_TRANSMISIBILITY;

}

template<typename TSeq>
inline double Tool<TSeq>::get_recovery(
    Virus<TSeq> * v
) {

    if (!recovery)
        return DEFAULT_RECOVERY;
    
    return recovery(this, person, v, person->model);

}

template<typename TSeq>
inline double Tool<TSeq>::get_death(
    Virus<TSeq> * v
) {

    if (!death)
        return DEFAULT_DEATH;
    
    return death(this, person, v, person->model);

}

template<typename TSeq>
inline void Tool<TSeq>::set_efficacy(
    ToolFun<TSeq> fun
) {
    efficacy = fun;
}

template<typename TSeq>
inline void Tool<TSeq>::set_recovery(
    ToolFun<TSeq> fun
) {
    recovery = fun;
}

template<typename TSeq>
inline void Tool<TSeq>::set_death(
    ToolFun<TSeq> fun
) {
    death = ToolFun<TSeq>(fun);
}

template<typename TSeq>
inline void Tool<TSeq>::set_transmisibility(
    ToolFun<TSeq> fun
) {
    transmisibility = ToolFun<TSeq>(fun);
}

#undef DEFAULT_EFFICACY
#undef DEFAULT_RECOVERY
#undef DEFAULT_DEATH
#endif