#ifndef COMP_ALGO_H
#define COMP_ALGO_H
#include <QDateTime>
#include <QDebug>
#include <game.h>
#include <QtMath>

/*comp_algo is the base class:
-means all algorithms are derived from it (get a own copy of the class, so it cant have global params for all algorithms)

*/

class IterParam
{
public:
    int iteration;
    int plnum;
    Game *game;
    QVector<int> past_act;
    QVector <int> past_rews;
};


class Comp_algo //base class from which the algorithms are derived
{
public:
    Comp_algo();
    virtual ~Comp_algo();
    // pure virtual: =0; we have to pass the same paramaters to all functions.
    //(void)params->plnum; //too supress a variable never used warning^^
    virtual int action(IterParam *params)=0;   
private:    
};


class Helpers //helper for the greedy classes
{
public:
    //for NGreedy, EGreedy
    int greedy_max_action(int game_act,QVector<int>Greedy_past_rew,QVector<int> Greedy_pastact);
    //no regret, qlearning
    int get_max_value_d(QVector<double> history);
};

class Best_resp : public Comp_algo
{
  public:
    int action(IterParam *params);
    int Best_response(Game *game,QVector<int> binplayer,QVector<int> Steps, int plnum);
};

class Bully_own :  public Best_resp
{
public:
     int action(IterParam *params);
};

class Bully :  public Best_resp
{
public:
     Bully(IterParam *params);//init
     int action(IterParam *params);
     int Bully_act;
};


class Determ : public Comp_algo
{
public:
    Determ(IterParam *params);//init
    //void determined_init(Game *game,int plnum) ;
    //int determined_action(int plnum) ;
    int action(IterParam *params);
    QVector<double> Det_optimal_act;//determined optimal action
};

class Egreedy : Helpers, public Comp_algo
{
public:
    Egreedy(IterParam *params,int probexplore);//,int rounds_explore
    int action(IterParam *params);
    QVector<int> Greedy_pastact;
    QVector<int> Greedy_past_rew;
    //int EGreed_init_expl;
    int EGr_probexplore;
};

class Fict_pl : public Best_resp
{
public:   
    Fict_pl(IterParam *params);//init
    int action(IterParam *params);
    int get_max_value_i(QVector<int> history);
    QVector<QVector<int>> Fict_pl_act_distib;//ficticious play
};

class Maxmin :  virtual public Comp_algo
{
public:
     Maxmin(){}//default constructor, only used by Godfather
     Maxmin(IterParam *params);
     int action(IterParam *params);
     void find_min_reward_given_act(Game *game,QVector<int> binplayer, QVector<int>Steps, int plnum, int &minpayoff);
     int Minmax_act;//minmax optimal action
     int Sec_lvl_op_act;//pass opponent action for your security level payoff too godfather
};

class Nash: public Comp_algo
{
public:
    Nash(IterParam *params);
    int action(IterParam *params);
    QVector<double> NE_player_act;//nash player action, as NE can be mixed it needs a vector
};

class Ngreedy : Helpers, public Comp_algo
{
public:
    Ngreedy(IterParam *params);
    int action(IterParam *params);
    QVector<int> Greedy_pastact;
    QVector<int> Greedy_past_rew;
};

class Noreg : Helpers, public Comp_algo
{
public:
     Noreg(IterParam *params);
     int action(IterParam *params);
     QVector<int> regret;
};

class Pareto_opt : virtual public Comp_algo
{
public:
    Pareto_opt(){}//default constructor, only used by Godfather
    Pareto_opt(IterParam *params, bool par_comp);//init
    int action(IterParam *params);
    void Par_opt_init(IterParam *params, bool par_comp);
    int Par_optimal_act;//pareto optimal algorithm
    int TFT_pcoop_ind;//to pass the index of the cooperative pareto optimal action from par-opt to TFT
};

class QL : Helpers, public Comp_algo
{
public:
     QL(IterParam *params,double learnrate, double discountrate, double explorerate, double decay_learn, double decay_explore);
     int action(IterParam *params);
     QVector<double> q_value;
     double q_learnrate;
     double q_explrate;
     double q_discrate;
     double q_decay_expl;
     double q_decay_learn;
};

class Random : public Comp_algo
{
public:
    int action(IterParam *params);
};

class SatisF :  public Comp_algo
{
public:
     SatisF(double init_aspir,double pers_rate);
     int action(IterParam *params);
     double Saspir_level;//aspiration levels for the satisfysing player
     double Spers_rate;
};

class Godfather :  public Maxmin, public Pareto_opt
{
public:
    Godfather(IterParam *params);
     int action(IterParam *params);
     QVector<int>Godfa_val;//Godfather player action.
};

class TitforTat :  public Maxmin, public Pareto_opt
{
public:
    TitforTat(IterParam *params,bool smart);
     int action(IterParam *params);
     QVector<int>TFT_act;//TFT player action.
     bool Smart;
};
/*
class IGA : public Comp_algo
{
public:
    IGA(double alpha_i,double beta_i,double learn_rate);//init
    //void determined_init(Game *game,int plnum) ;
    //int determined_action(int plnum) ;
    int action(IterParam *params);
    double Learn_r;
    double Alpha; //alpha & beta params
    double Beta;
};
*/

class Markov : public Comp_algo
{
public:
    Markov(IterParam *params,int windowsize);//init
    //void determined_init(Game *game,int plnum) ;
    //int determined_action(int plnum) ;
    int action(IterParam *params);
    //QVector<int> States;
    QHash<int, int> States;
    QVector<int> Multipl_steps;
    int Windowsize;
    QList<int> Curentwindow;
};


#endif // COMP_ALGO_H
