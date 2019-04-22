#include "comp_algo.h"

Comp_algo::Comp_algo(){}
Comp_algo::~Comp_algo(){}


//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//helper function for greedy algorithms: get the highest reward action
int Helpers::greedy_max_action(int game_act,QVector<int> Greedy_past_rew,QVector<int> Greedy_pastact)
{
    QVector<double>history(game_act,0);
    //calculate wich action gave highest reward over time.
    for(int i=0;i<game_act;i++)
    {
        history[i]=static_cast<double>(Greedy_past_rew[i])/Greedy_pastact[i];
    }
    //pick highest reward
    double max_rew = *std::max_element(history.begin(), history.end());
    return history.indexOf(max_rew);
}

//get index of the max_double of the list
int Helpers::get_max_value_d(QVector<double> history)
{
    int action;
    QVector<int> best_act;
    double max = *std::max_element(history.begin(), history.end());//get_max value
    int ind=history.indexOf(max);
    while(ind>=0)//-1 if not found
    {
        best_act.append(ind);
        ind=history.indexOf(max,(ind+1));//search from index for min
    }
    if(best_act.count()>1){ind=rand()%best_act.count();action=best_act[ind];}//if multiple: pick a random act.
    else action=best_act[0];
    return action;
}

//Best response
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
int Best_resp::action(IterParam *params)
{
    if(params->iteration==0)return rand()%params->game->Actions;
    return Best_response(params->game,params->past_act,params->game->Stepsize,params->plnum);
}
//helper functions
//this function is used to find your highest possible payoff, given the opponents actions
int Best_resp::Best_response(Game *game,QVector<int> binplayer,QVector<int> Steps, int plnum)
{
    int maxpayoff=0;
    int max_ind=0;
    binplayer[plnum]=0;//assums that binplayer (your past action)=0;
    int cel=game->Calculate_bincel(binplayer,Steps);
    for(int i=0;i<game->Actions;i++)//next find your own best action.
    {
        int payoff=game->Matrix_i[cel][plnum];
        if(payoff>maxpayoff){maxpayoff=payoff;max_ind=i;}
        cel+=Steps[plnum];
    }
    return max_ind;
}

//get bully action.
int Bully_own::action(IterParam *params)
{
    //play random first action
    if(params->iteration==0)return rand()%params->game->Actions;
    QVector<int> new_Act=params->past_act;
    for (int i=0;i<params->game->Players;i++)//for all players:compute best responce
    {
     if(i==params->plnum)continue;
     new_Act[i]=Best_resp::Best_response(params->game,params->past_act,params->game->Stepsize,i);//compute for all players there best reponce
    }
    //now compute the best responce to this new action profile
    return Best_resp::Best_response(params->game,new_Act,params->game->Stepsize,params->plnum);
}

//note: only works for two players!!!
Bully::Bully(IterParam *params)
{
    int own_maxpayoff=INT_MIN;
    int max_ind=0;
    //for all own actions
    for(int i=0;i<params->game->Actions;i++)
    {
        QVector<int>maxpayoff(params->game->Players,INT_MIN);//set all too min value
        QVector<int>expt_Act(params->game->Players,0);
        expt_Act[params->plnum]=i;//set own action.

        //qDebug()<<"own_action"<<i;
        for (int j=0;j<2;j++)//compute best responce for opponent
        {
            if(j==params->plnum)continue;//skip self
            //expt_Act[j]=Best_resp::Best_response(params->game,params->past_act,params->Steps,j);//compute for all players there best reponce
            /*special version of best responce*/
            expt_Act[j]=0;//assums that binplayer (your past action)=0;
            //qDebug()<<"makrera"<<expt_Act;
            int cel=params->game->Calculate_bincel(expt_Act,params->game->Stepsize);
            //qDebug()<<"cel"<<cel;
            for(int k=0;k<params->game->Actions;k++)//next find your own best action.
            {
                QVector<int>payoff=params->game->Matrix_i[cel];
                //qDebug()<<"payoff"<<payoff;
                //qDebug()<<payoff[j]<<","<<maxpayoff[j];
                if(payoff[j]>maxpayoff[j]){maxpayoff=payoff;}//qDebug()<<" update"<<payoff;}
                //case where opponent is indifferent, minimise your reward, only works for two players!!!
                else if(payoff[j]==maxpayoff[j] & payoff[1-j]<maxpayoff[1-j]){maxpayoff=payoff;}// qDebug()<<"indiff but update"<<payoff;}
                cel+=params->game->Stepsize[j];
            }
        }

        //compute cell
        if(maxpayoff[params->plnum]>own_maxpayoff){own_maxpayoff=maxpayoff[params->plnum];max_ind=i;}//qDebug()<<"I get higher reward here"<<own_maxpayoff<<","<<maxpayoff; }
    }    
    Bully_act=max_ind;
    //qDebug()<<"final_act"<<Bully_act;
}

int Bully::action(IterParam *params)
{
    return Bully_act;
}


//determined
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//init the determined player for plnum
Determ::Determ(IterParam *params)//init deermined play
{
    QVector<double>max_pay(params->game->Players,0);
    Det_optimal_act=QVector<double>();
    QVector<double> best_act;
    if(params->game->NE_d.count()==0){Det_optimal_act.append(static_cast<double>(rand()%params->game->Actions));qDebug()<<"DET no NE:";return;}//no NE: persistantly play a random action...
    for(QVector<QVector<double>> NE_profile : params->game->NE_d)//for each NE
    {
        int cel=0,act=0;
        QVector<double> payoff(params->game->Players,0);//get reward of each player
        bool mixed_NE=false;
        QList<int> mix_cels;
//        QList<double> mix_mult;
        for(int pl=0;pl<params->game->Players;pl++)
        {
//            int mixed_c=0;
//            int mixed_steps=0;
            for(int i=0;i<params->game->Actions;i++)
            {
                double val=NE_profile[pl][i];
                if(qFabs(val - 1.0) < std::numeric_limits<double>::epsilon())//pure NE: store action

                {
                    cel+=params->game->Stepsize[pl]*i;
                    if(pl==params->plnum)act=i;
                    break;
                }
                else if(val>0.0)//not one, so must be mixed.
                {
                    mixed_NE=true;//mixed NE
                }
                if(mixed_NE==true)break;
            }
            if(mixed_NE==true)break;
        }
        if(mixed_NE==false){for(int i=0;i<params->game->Players;i++)payoff[i]=static_cast<double>(params->game->Matrix_i[cel][i]);}//payoff of a pure NE
        else //compute payoff misex NE
        {
            QVector<int>binplayer=params->game->Binplayer;
            bool done=false;
            payoff[params->plnum]=0.0;
            while(done==false)//loop all possible combinations
            {
                double mult_cel=1;
                for(int i=0;i<binplayer.count();i++){mult_cel*=NE_profile[i][binplayer[i]];}
                int cel=params->game->Calculate_bincel(binplayer,params->game->Stepsize);
                //payoff+=mult_cel*static_cast<double>(params->game->Matrix_i[cel][params->plnum]);
                for(int i=0;i<params->game->Players;i++)payoff[i]=static_cast<double>(params->game->Matrix_i[cel][i]);
                binplayer=params->game->update_binpl(binplayer,&done);
            }
        }
        //qDebug()<<"payoff NE"<<payoff;
        if(payoff[params->plnum]>max_pay[params->plnum]){max_pay=payoff; best_act=(mixed_NE==true)?NE_profile[params->plnum] : QVector<double>(1,act);}
        else if(qFabs(payoff[params->plnum] - max_pay[params->plnum]) < std::numeric_limits<double>::epsilon())//equal payoff
        {
            bool higher=true;
            for(int i=0;i<params->game->Players;i++){;if(payoff[i]<max_pay[i]){higher=false;}}//for all players
            if(higher==true){max_pay=payoff; best_act=(mixed_NE==true)?NE_profile[params->plnum] : QVector<double>(1,act);}
        }
        mixed_NE=false;
    }
    //qDebug()<<"det best_act"<<best_act;
    Det_optimal_act=best_act;//set action.
}

//get determined action for player plnum
int Determ::action(IterParam *params)
{
    //pure ne
    if(Det_optimal_act.count()==1)return static_cast<int>(Det_optimal_act[0]);
    double prob=static_cast<double>(rand()) / RAND_MAX;
    for(int i=0;i<Det_optimal_act.count();i++)
    {
        if(prob<=Det_optimal_act[i]){return i;}
        prob=prob- Det_optimal_act[i];
    }
 qDebug()<<"determined is broken";
 return params->plnum;//we should never get here, but c++ wants a return value
}

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//initialise e-greedy player
Egreedy::Egreedy(IterParam *params,int probexplore)
{
    QVector<int> empty_vector(params->game->Actions,0);//init at 1, to avoid zero devisions.
    Greedy_pastact=empty_vector;
    Greedy_past_rew=empty_vector;
    EGr_probexplore=probexplore;
}

//take e-greedy action, example value 0.1
int Egreedy::action(IterParam *params)
{
    //play first action, but dont update params in iteration 0.
    if(params->iteration==0){return params->iteration;}//
    //update log with past results
    int my_pst_act=params->past_act[params->plnum];
    Greedy_pastact[my_pst_act]+=1;
    Greedy_past_rew[my_pst_act]+=params->past_rews[params->plnum];
    //play each action at least once
    if(params->iteration<params->game->Actions){return params->iteration;}
    //always explore first_x_rounds
    int explore=rand()%100;//value between 0-100
    if(explore<EGr_probexplore)return rand()%params->game->Actions;//take random action
    return greedy_max_action(params->game->Actions,Greedy_past_rew,Greedy_pastact);
}

//fictitious  play
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//helper, get index of the max_int of the list
int Fict_pl::get_max_value_i(QVector<int> history)
{
    int action;
    QVector<int> best_act;
    int max = *std::max_element(history.begin(), history.end());//get_max value
    int ind=history.indexOf(max);
    while(ind>=0)//-1 if not found
    {
        best_act.append(ind);
        ind=history.indexOf(max,(ind+1));//search from index for min
    }
    if(best_act.count()>1){ind=rand()%best_act.count();action=best_act[ind];}//if multiple: pick a random act.
    else action=best_act[0];

    return action;
}

Fict_pl::Fict_pl(IterParam *params)
{
    QVector<int> past_act(params->game->Actions,0);
    Fict_pl_act_distib=QVector<QVector<int>>(params->game->Players,past_act);
}

//get the fictitious  play action for player plnum
//note that stpesize is in reverse order!!!! [0] is lowest reward.
int Fict_pl::action(IterParam *params)
{
    if(params->iteration==0)return rand()%params->game->Actions;
    QVector<int> binplayer=params->game->Binplayer;
    for(int i=0;i<params->game->Players;i++)//find the binary indeces for the opther player:actions
    {
       if(i==params->plnum)continue;//skip own player
       Fict_pl_act_distib[i][params->past_act[i]]++;//update the action distribution
       QVector<int> past_act =Fict_pl_act_distib[i];
       binplayer[i]=get_max_value_i(past_act);
    }
    //we have binplay: test all the possible actions given the opponents actions
    return Best_resp::Best_response(params->game,binplayer,params->game->Stepsize,params->plnum);
}

//helper functions
//this function is used to find your worst possible payoff, given your action
//used in maxmin
void Maxmin::find_min_reward_given_act(Game *game,QVector<int> binplayer, QVector<int>Steps, int plnum, int &minpayoff)
{
    minpayoff=INT32_MAX;//start from highest possible value
    QVector<int>mini_binpl=binplayer;
    int own_act=mini_binpl[plnum];
    mini_binpl.remove(plnum);//one element less: dont have to change own action.
    bool done=false;
    while(done==false)
    {
        binplayer=mini_binpl;
        if(plnum<binplayer.length())binplayer.insert(plnum,own_act);//insert is slow, but better than looping
        else binplayer.append(own_act);//cannot insert outside of the vector^^
        int cel=game->Calculate_bincel(binplayer,Steps);
        int payoff=game->Matrix_i[cel][plnum];
        if(payoff<minpayoff){minpayoff=payoff;if(game->Players==2)Sec_lvl_op_act=binplayer[1-plnum];}
        mini_binpl=game->update_binpl(mini_binpl,&done);//get next combination of opponents actions
        if(done==true)break;
    }
}

//initialise minmax/security level player
Maxmin::Maxmin(IterParam *params)
{
    QVector<int> binplayer=params->game->Binplayer;
    int maxpayoff=INT32_MIN,max_ind=0;
    int minpayoff;

    //step 1: find lowerst reward you get for each action
    for(int i=0;i<params->game->Actions;i++)
    {
        binplayer[params->plnum]=i;//set your action
        find_min_reward_given_act(params->game, binplayer,params->game->Stepsize, params->plnum,minpayoff);
        //step 2, maximise over the minimum
        if(maxpayoff<minpayoff){maxpayoff=minpayoff;max_ind=i;}
    }
    Minmax_act=max_ind;
}

//return action of minmax player
int Maxmin::action(IterParam *params)
{
    (void)params->plnum;//fake argument too supress the unused warning
    return Minmax_act;
}

//Nash
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//initialise a nash player, from Airiau: play a random NE if multiple, play random is non
Nash::Nash(IterParam *params)
{
    std::numeric_limits<double>::epsilon();
    //no NE: persistantly play a random action...
    QVector<QVector<double>>NE_proile;
    if(params->game->NE_d.count()==0){NE_player_act.append(rand()%params->game->Actions);return;}
    else
    {
        //pick a random NE
        NE_proile=params->game->NE_d[rand()%params->game->NE_d.count()];
        double actnum=0;
        for(double NE_act : NE_proile[params->plnum])//check if actions are zero, if not add them to the list
        {
            if(qFabs(NE_act - 1.0) < std::numeric_limits<double>::epsilon()){NE_player_act.append(actnum);return;}//pure NE
            actnum++;
        }
        NE_player_act=NE_proile[params->plnum];//incase of mixed NE: we just take the entire NE
    }
}

//return the NE action, can also handle mixed NE
int Nash::action(IterParam *params)
{
    (void)params->plnum;//fake argument too supress the unused warning
    if(NE_player_act.count()==1)return static_cast<int>(NE_player_act[0]);//pure NE or no NE
    else //mixed NE;
    {
        double prob=static_cast<double>(rand()) / RAND_MAX;
        for(int i=0;i<NE_player_act.count();i++)
        {
            if(prob<=NE_player_act[i]){return i;}
            prob=prob- NE_player_act[i];
        }
    }
    return NE_player_act.count()-1;//we should never get here but incase we do: return last index
}

//initialise n-greedy player
Ngreedy::Ngreedy(IterParam *params)
{
    QVector<int> empty_vector(params->game->Actions,0);//init at 1, to avoid zero devisions.
    Greedy_pastact=empty_vector;
    Greedy_past_rew=empty_vector;
}

//get next Ngreedy action
int Ngreedy::action(IterParam *params)
{
    //first action, play random
    if(params->iteration==0){return params->iteration;}//rand()%params->game->Actions;}
    //update log with past results
    int my_pst_act=params->past_act[params->plnum];
    Greedy_pastact[my_pst_act]+=1;
    Greedy_past_rew[my_pst_act]+=params->past_rews[params->plnum];
    //play each action at least once
    if(params->iteration<params->game->Actions){return params->iteration;}
    int action;
    double r = static_cast<double>(rand()) / RAND_MAX;
    double threshold = static_cast<double>(params->game->Actions) / static_cast<double>(params->iteration);
    //explore with decreasing probability.
    if(r < threshold) action= rand()%params->game->Actions;
    else//exploit
    {
        action=Helpers::greedy_max_action(params->game->Actions,Greedy_past_rew,Greedy_pastact);
    }
    return action;
}

//initialise no-regret
Noreg::Noreg(IterParam *params)
{
    regret=QVector<int>(params->game->Actions,0);//init regret vector
    QVector<bool>reger_first;
}


//get next no-regret action
int Noreg::action(IterParam *params)
{
    //first action, play random
    if(params->iteration==0){return rand()%params->game->Actions;}

    //update regret
    int my_pst_act=params->past_act[params->plnum];
    QVector<int> binplayer=params->past_act;
    for(int i=0;i<params->game->Actions;i++)//for each action, check what reward we would have gotten
    {
        if(i==my_pst_act)continue;//skip own action.
        binplayer[params->plnum]=i;
        int cel=params->game->Calculate_bincel(binplayer,params->game->Stepsize);
        int payoff=params->game->Matrix_i[cel][params->plnum];
        int past_rew=params->past_rews[params->plnum];
        for(int j=0;j<params->game->Actions;j++){if(j!=my_pst_act)regret[j]+=(payoff-past_rew);}//increase regret of all actions but the one you played.
    }
    //qDebug()<<regret;
    //compute average regrets
    QVector<double>av_regret(params->game->Actions,0);
    for(int i=0;i<params->game->Actions;i++)
    {
        av_regret[i]=static_cast<double>(regret[i])/params->iteration;
        if(av_regret[i]<0)av_regret[i]=0;//cap regrets off at 0
    }
    //normal NR will return the move with the highest regret, if two have the same regret pick random.
    return Helpers::get_max_value_d(av_regret);
}

//pareto optimal
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//init for the pareto optimal action for player plnum
Pareto_opt::Pareto_opt(IterParam *params, bool par_comp)
{
    Par_opt_init(params, par_comp);
}
void Pareto_opt::Par_opt_init(IterParam *params, bool par_comp)//done so that GodFather & TFT can acces it as well
{
    int action=0;
    if(par_comp==true)//get own highest reward
    {
        int best_Act=INT_MIN;
        for(int i=0;i<params->game->Pareto_opt.count();i++)
        {
            if(params->game->Pareto_opt[i][params->plnum]>best_Act)
            {
                best_Act=params->game->Pareto_opt[i][params->plnum];
                action=params->game->Pareto_opt_ind[i]; //get the index of the reward in the game matrix
            }
        }
    }
    else //get state that gives both highest reward, in case of ties: go for own benefit.
    {//sort with custom comparator > default one cannot handle multiple players (changing index) [0,1]
        QVector<QVector<int>>sorted_index;
        int last_i=params->game->Players-1;//index of last player that was checked
        for(int i=0;i<params->game->Players;i++) //for all players
        {
            QVector<QVector<int>>sorting=params->game->Pareto_opt;
            for(int j=0;j<sorting.count();j++)sorting[j][last_i]=params->game->Pareto_opt_ind[j];//store the action index in the payoffs

            for(int j=0;j<sorting.count();j++)//sort the list
            {
                int currentMin = sorting[j][i];
                int currentMinIndex = j;
                for(int k = j + 1; k <sorting.count(); k++)
                {
                    if(currentMin > sorting[k][i])
                    {
                        currentMin = sorting[k][i];
                        currentMinIndex = k;
                    }
                }
                if(currentMinIndex != j)
                {
                    QVector<int>temp=sorting[currentMinIndex];
                    sorting[currentMinIndex] = sorting[j];
                    sorting[j] = temp;
                }
            }
            //get the indexes out of the sorted vector
            QVector<int> ind;
            for(int j=0;j<sorting.count();j++)ind.append(sorting[j][last_i]);
            last_i=i;
            sorted_index.append(ind);
        }

        //begin at last index
        int lowest_ind_vers=sorted_index[0].count();
        for(int i=sorted_index[0].count()-1;i>=0;i--)
        {
            int to_check=sorted_index[0][i];
            int ind_vers=0;
            for(int j=1;j<params->game->Players;j++)
            {
                int checking=(sorted_index[j].indexOf(to_check));
                ind_vers+=abs(i-checking);//index verschil
            }
            if(ind_vers<lowest_ind_vers)//smaller difference
            {
                lowest_ind_vers=ind_vers;
                action=to_check;
            }
            else if(ind_vers==lowest_ind_vers)//if both the same:pick highest reward action for self
            {
                if(params->game->Matrix_i[to_check][params->plnum]>params->game->Matrix_i[action][params->plnum])action=to_check;
            }
        }
    }
    //qDebug()<<"pl_ind"<<params->plnum;
    //qDebug()<<"best_act_index:"<<action;
    TFT_pcoop_ind=action;

    //qDebug()<<"par_opt"<<params->game->Stepsize<<action<<params->plnum;
    Par_optimal_act=params->game->Calculate_binact(params->game->Stepsize,action,params->plnum);
    //qDebug()<<"par_opt_b"<<Par_optimal_act;
}

//get the pareto optimal action for player plnum
int Pareto_opt::action(IterParam *params)
{
    (void)params->plnum;//fake argument too supress the unused warning
    return Par_optimal_act;
}

//random
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//create a random action
int Random::action(IterParam *params)
{
    return rand()%params->game->Actions;
}

//init Satisficing play
//note that init_aspir is a value between 0-max rew
//pers_rate is a value between 0-1
SatisF::SatisF(double init_aspir,double pers_rate)
{
    Saspir_level=init_aspir;
    Spers_rate=pers_rate;
}

//get next action Satisficing player, needs only own past action
int SatisF::action(IterParam *params)
{
    //first action, play random
    if(params->iteration==0){return rand()%params->game->Actions;}
    //update aspiration
    int past_rew=params->past_rews[params->plnum];
    Saspir_level=Saspir_level*Spers_rate+static_cast<double>(past_rew)*(1-Spers_rate);
    //if reward >= aspir level: keep playing action, else play anny different action
    if(past_rew>=Saspir_level){return params->past_act[params->plnum];}
    //pick a random action as long as its not the action you just layed
    QVector<int> pas_act(params->game->Actions-1,0);
    for(int i=0;i<params->game->Actions;i++){if(i==params->past_act[params->plnum]){continue;} pas_act.append(i);}
    int random = rand()%pas_act.count();
    return pas_act[random];
    /*do {  //we will mostly do 2 action games, so this methode is inefficient...
       random = rand()%params->game->Actions;
    } while (random == params->past_act[params->plnum]);//pick any action as long as its not the past action.
    return random;
    */
}



//init memory less q_player, has only one state
QL::QL(IterParam *params,double learnrate, double discountrate, double explorerate, double decay_learn, double decay_explore)
{
    QVector<double> q_act(params->game->Actions,0);
    q_value= q_act;//init the q_values.
    q_learnrate=learnrate;
    q_explrate= explorerate;
    q_discrate= discountrate;
    q_decay_expl=decay_explore;
    q_decay_learn=decay_learn;
}

//get next action for memory less q_player
int QL::action(IterParam *params)
{
    //first action, play random
    if(params->iteration==0){return rand()%params->game->Actions;}
    //note that the algorithm plays random annyways as all Q-values are the same
    //update values    
    int my_past_a=params->past_act[params->plnum];
    int past_rew=params->past_rews[params->plnum];
    //only one state so q_discrate * max Q(y,i') is 1..
    q_value[my_past_a]=(1-q_learnrate)*q_value[my_past_a]+ q_learnrate*(static_cast<double>(past_rew)+q_discrate*1);
    q_explrate=q_explrate*q_decay_expl;
    q_learnrate=q_learnrate*q_decay_learn;

    //pick new action, explore with decreasing probability.
    double prob=static_cast<double>(rand()) / RAND_MAX;
    if(prob<q_explrate){return rand()%params->game->Actions;}
    //else play the highest reward action
    return Helpers::get_max_value_d(q_value);
}


//not, only works for two players!!!, Sec_lvl_op_act in maxmin is also only set for two players.
Godfather::Godfather(IterParam *params)
{
   //get security level payoff for the opponent, basically same as maxmin(but since we are looking for the opponent we use his plnum so just calling the function is no option.)
    QVector<int> binplayer=params->game->Binplayer;
    int maxpayoff;//,max_ind;
    int minpayoff=0;
    int punish_act=0;
    QVector<int>sec_lvl_payoffs;
    //step 1: find security payoff of each player
    for(int pl=0;pl<params->game->Players;pl++)
    {
        binplayer=params->game->Binplayer;
        maxpayoff=INT32_MIN;
        for(int i=0;i<params->game->Actions;i++)
        {
            binplayer[pl]=i;//set your action
            Maxmin::find_min_reward_given_act(params->game, binplayer,params->game->Stepsize,pl,minpayoff);
            //step 2, maximise over the minimum
            if(maxpayoff<minpayoff){maxpayoff=minpayoff;}//max_ind=i;}
        }
        sec_lvl_payoffs.append(maxpayoff);
        if(pl!=params->plnum)//opponent
        {
            punish_act=Maxmin::Sec_lvl_op_act;
        }
    }
    //find targettable pairs: all cells with payoff for all players>maxmin payoff
    binplayer=params->game->Binplayer;
    QVector<int>my_pos_target_act;
    QVector<int>my_pos_target_payoff;
    bool done=false,targettable=true;
    //qDebug()<<"sec_lvl_payoffs"<<sec_lvl_payoffs;
    while(done==false)//for all cells
    {
        targettable=true;
        int cel=params->game->Calculate_bincel(binplayer,params->game->Stepsize);
        QVector<int> payoff=params->game->Matrix_i[cel];
        for(int i=0;i<payoff.count();i++)if(payoff[i]<=sec_lvl_payoffs[i])targettable=false;//one player got a lower reward.
        if(targettable==true){my_pos_target_act.append(binplayer[params->plnum]);my_pos_target_payoff.append(payoff[params->plnum]);}
        binplayer=params->game->update_binpl(binplayer,&done);//get next combination of opponents actions
        if(done==true)break;
    }

   // max_ind:highest reward action for opponent.
   //get highest cooperative reward.
   /*Pareto_opt::Par_opt_init(params,false);//false cooperative
   int coop_act=Pareto_opt::Par_optimal_act;
   int cooprew=params->game->Matrix_i[Pareto_opt::TFT_pcoop_ind][params->plnum];//expected reward for cooperation.
   if(coop_act==punish_act)punish_act=-1;//coop action=punishment action..
   qDebug()<<"GOTF_vals_init"<<cooprew<<coop_act<<punish_act;
   */
    int coop_act,coop_rew=0;
    if(my_pos_target_act.count()>0)
    {
        int random_ind= rand()%my_pos_target_act.count();
        coop_act=my_pos_target_act[random_ind];
        coop_rew=my_pos_target_payoff[random_ind];
    }
    else
    {
        coop_act=punish_act;
    }
   Godfa_val={coop_rew,coop_act,punish_act};
   //qDebug()<<"Godfa_val"<<Godfa_val;
}

int Godfather::action(IterParam *params)
{
    //always cooperate on first round.
    if(params->iteration==0)return Godfa_val[1];
    int past_rew=params->past_rews[params->plnum];
    if(past_rew>=Godfa_val[0])return Godfa_val[1];//earned the coop reward or more, keep cooperating
    if(Godfa_val[2]>=0)return Godfa_val[2];//else if there is a punishment action: play it
    //no punishment action?: it is the same as coop, play random
  /*  QVector<int> pas_act(params->game->Actions-1,0);
    for(int i=0;i<params->game->Actions;i++){if(i==params->past_act[params->plnum]){continue;} pas_act.append(i);}
    int random = rand()%pas_act.count();
    return pas_act[random];*/
    /*do {  //we will mostly do 2 action games, so this methode is inefficient...
       random = rand()%params->game->Actions;
    } while (random == params->past_act[params->plnum]);//pick any action as long as its not the past action.
    return random;
    */
    qDebug()<<"Godfather broken";
    return 0;
}

TitforTat::TitforTat(IterParam *params, bool smart)
{
    //smart is false: play past action of opponent
    Smart=smart;
    if(smart==false)return;
    Pareto_opt::Par_opt_init(params,false);//false cooperative
    //else: play coop par opt als coop, comp par opt als punishment
    int TFT_coop_act=Pareto_opt::Par_optimal_act;
    int TFT_cooprew=params->game->Matrix_i[Pareto_opt::TFT_pcoop_ind][params->plnum];//expected reward for cooperation.
    Pareto_opt::Par_opt_init(params,true);//true: competitive
    int TFT_comp_act=Pareto_opt::Par_optimal_act;
    if(TFT_comp_act == TFT_coop_act) TFT_comp_act=-1;//random.
    TFT_act={TFT_cooprew,TFT_coop_act,TFT_comp_act};
    //qDebug()<<"TFT"<<TFT_cooprew<<TFT_coop_act<<TFT_comp_act;
}


int TitforTat::action(IterParam *params)
{
    //play action of one of your opponents, only really works in 2 player games
    if(Smart==false)
    {
        if(params->iteration==0){return rand()%params->game->Actions;}
        QVector<int>opp_past_act=params->past_act;
        opp_past_act.remove(params->plnum);//remove own past action
        int act_ind=rand()%opp_past_act.count();
        return opp_past_act[act_ind];
    }
    //always cooperate on first round.
    if(params->iteration==0)return TFT_act[1];
    int reward_earned=params->past_rews[params->plnum];
    if(reward_earned>=TFT_act[0])return TFT_act[1];//earned the coop reward or mre, keep cooperating
    if(TFT_act[2]>=0)return TFT_act[2];//else if there is a punishment action: play it
    //if not:play random.
    QVector<int> pas_act;//((params->game->Actions-1),0);
    //qDebug()<<"past_act"<<params->past_act[params->plnum];
    for(int i=0;i<params->game->Actions;i++){if(i==params->past_act[params->plnum]){continue;} pas_act.append(i);}
    //qDebug()<<"pas_act"<<pas_act;
    int random = rand()%pas_act.count();
    //qDebug()<<"rand"<<random<<pas_act[random];
    return pas_act[random];
    /*do {  //we will mostly do 2 action games, so this methode is inefficient...
       random = rand()%params->game->Actions;
    } while (random == params->past_act[params->plnum]);//pick any action as long as its not the past action.
    return random;
    */
}

Markov::Markov(IterParam *params,int windowsize)
{
    Windowsize=windowsize;
    //windows size one
    Multipl_steps={1};//col
    Multipl_steps.append(params->game->Actions);//row
    for(int i=2;i<params->game->Players;i++)Multipl_steps.append(Multipl_steps[i-1]*params->game->Actions);//all other players

    //now repeat this for windows size>1
    for(int win=1;win<windowsize;win++)
    {
        int ofset=win*params->game->Players;
        for(int i=0;i<params->game->Players;i++)Multipl_steps.append(Multipl_steps[ofset+i-1]*params->game->Actions);//for all players
    }
    //States=QVector<int>(static_cast<int>((pow(params->game->Actions,(windowsize*params->game->Players)))),1);//number of actions ^ (number of players * windowsize)
    //qDebug()<<"Multipl_steps"<<Multipl_steps;
    //qDebug()<<"States"<<States.count();
    States=QHash<int, int>();
    //initial window: all zero's
    Curentwindow=QList<int>();
    for(int i=0;i<params->game->Players*Windowsize;i++)Curentwindow.append(0);
    //qDebug()<<"cur"<<Curentwindow;
}

int Markov::action(IterParam *params)
{
//optional: play random until first windo is filled
/*    //play random in the first round
    if(params->iteration==0){return rand()%params->game->Actions;}
    //first few rounds init, buffer needs to fill so we play random
    if(Curentwindow.count()!=params->game->Players*Windowsize)
    {
        for(int i=0;i<params->past_act.count();i++)Curentwindow.append(params->past_act[i]);//add past act to fill the first window
        return rand()%params->game->Actions;
    }
*/
    //create current window
    //qDebug()<<"icur window"<<Curentwindow;
    for (int i = 0; i < params->game->Players; ++i)Curentwindow.removeFirst();
    for(int i=0;i<params->past_act.count();i++)Curentwindow.append(params->past_act[i]);
    //qDebug()<<"past_act"<<params->past_act;
    //qDebug()<<"cur window"<<Curentwindow;

    //store current window in states unles the first round.
    if(params->iteration>0)
    {
        int state=0;
        for (int i=0;i<Curentwindow.count();i++) {state+=Curentwindow[i]*Multipl_steps[i];}
        //States[state]+=1;
        QHash<int, int>::const_iterator States_p = States.find(state);
        if(States_p != States.end() && States_p.key() == state) //found
        {
            //insert works but causes memory leaks
            //States.insert(state,States_p.value()+1);//insert overwrites the value associated to the key.
            States[state]=States_p.value()+1;
        }
        else{States.insert(state,2);}//else insert two, starts at one.

        //qDebug()<<"Curentwindow"<<Curentwindow;
        //qDebug()<<"state"<<state;
    }
/*    if(params->iteration==9999)
    {
        qDebug()<<"number of states in markov is:"<<States.count();
        //qDebug()<<States;
    }
*/
    //lets pick an action based on the new window.
    QList<int>curstate=Curentwindow;
    for (int i = 0; i < params->game->Players; ++i)curstate.removeFirst();//remove last &add for the new round
    int state_base=0;//precompute past of state thats based on history
    for(int i=0;i<curstate.length();i++)state_base+=curstate[i]*Multipl_steps[i];
    int st_ind_ofset=curstate.length();
    //qDebug()<<"curstate"<<curstate;
    //qDebug()<<"state_base"<<state_base;
    //qDebug()<<"st_ind_ofset"<<st_ind_ofset;
    //qDebug()<<"plnum"<<params->plnum;
    QVector<int>newstate((params->game->Players-1),0);//minum own action
    QVector<int>Steps_game=params->game->Stepsize;
    double max_rew=0.0;
    int best_act=0;

    for (int my_Act=0;my_Act<params->game->Actions;my_Act++)//for all my actions
    {
        //compute expected reward for this action using the expected actions from the states & the expected rewards from the game
        //we pick the action with the highest expected payoff.
        bool done=false;
        QVector<int>payoffs;
        QVector<int>occurances;
        int total_occ=0;
        newstate=QVector<int>((params->game->Players-1),0);
        while(done==false)//for all combinations of opponent actions
        {
            int cel=0,ind=0;
            int state_act=0;
            for(int i=0;i<newstate.count()+1;i++)
            {
              if(i==params->plnum){cel+=my_Act*Steps_game[i];state_act+=my_Act*Multipl_steps[st_ind_ofset+i];continue;}//own actions
              cel+=newstate[ind]*Steps_game[i];//all opponent actions
              state_act+=newstate[ind]*Multipl_steps[st_ind_ofset+i];
              ind++;
            }
            //qDebug()<<"act"<<my_Act<<newstate;
            //qDebug()<<"state"<<state_base+state_act;
            //qDebug()<<"cel"<<cel;
            //qDebug()<<"rew"<<params->game->Matrix_i[cel][params->plnum];
            payoffs.append(params->game->Matrix_i[cel][params->plnum]);//get payoff.

            int new_st=state_base+state_act;
            QHash<int, int>::const_iterator States_p = States.find(new_st);
            if(States_p != States.end() && States_p.key() == new_st) //found
            {
                occurances.append(States_p.value());
                total_occ+=States_p.value();
            }
            else //state has not yet been played.
            {
                occurances.append(1);
                total_occ+=1;
            }
            newstate=params->game->update_binpl(newstate,&done);//get next combination of opponents actions
            if(done==true)break;
        }
        //qDebug()<<"total_occ"<<total_occ;
        //qDebug()<<"payoffs"<<payoffs;
        //qDebug()<<"occurances"<<occurances;
        //compute expected payoff for this action
        double exp_rew=0.0;
        for(int i=0;i<payoffs.length();i++)
        {
            exp_rew+=static_cast<double>(payoffs[i])*(static_cast<double>(occurances[i])/total_occ);
        }
        //qDebug()<<"exp_rew"<<exp_rew;
        if(exp_rew>max_rew){max_rew=exp_rew;best_act=my_Act;}
    }
    //qDebug()<<"max_rew"<<max_rew;
    //qDebug()<<"best_Act"<<best_act;
    return best_act;
}






/*
 * This code is IGA-wolf.
 Zinkevich [11] looked at gradient ascent using the evaluation criterion of regret.
He ﬁrst extended IGA beyond two-player, two-action games.
His algorithm, GIGA (Generalized Inﬁnitesimal Gradient Ascent)
*/
/*
IGA::IGA(double alpha_i,double beta_i,double learn_rate)
{
    Learn_r=learn_rate;
    Alpha=alpha_i;
    Beta=beta_i;
}

//only for 2 player, 2 action games, works best if both use this strategy...
//theirefor not used nor tested.
int IGA::action(IterParam *params)
{
    //matrix:
    //R11,R12
    //R21,R22
    //u row= R11-R12-R21+R22 , col= C11-C12-C21+C22
    int u_coli=params->game->Matrix_i[1][0]-params->game->Matrix_i[3][0]-params->game->Matrix_i[2][0]+params->game->Matrix_i[4][0];
    double u_col= static_cast<double>(u_coli);
    int u_rowi=params->game->Matrix_i[1][1]-params->game->Matrix_i[3][1]-params->game->Matrix_i[2][1]+params->game->Matrix_i[4][1];
    double u_row= static_cast<double>(u_rowi);

    double Alpha_stab=static_cast<double>(params->game->Matrix_i[4][0]-params->game->Matrix_i[2][0])/u_col;
    double Beta_stab=static_cast<double>(params->game->Matrix_i[4][1]-params->game->Matrix_i[3][1])/u_row;

//iga wolf addition
    double v_row=Alpha*Beta*u_row+Alpha*static_cast<double>(params->game->Matrix_i[3][1]-params->game->Matrix_i[4][1])+Beta*static_cast<double>(params->game->Matrix_i[2][1]-params->game->Matrix_i[4][1])+static_cast<double>(params->game->Matrix_i[4][1]);
    double v_col=Alpha*Beta*u_col+Alpha*static_cast<double>(params->game->Matrix_i[3][0]-params->game->Matrix_i[4][0])+Beta*static_cast<double>(params->game->Matrix_i[2][0]-params->game->Matrix_i[4][0])+static_cast<double>(params->game->Matrix_i[4][0]);

    double v_row_equil=Alpha_stab*Beta*u_row+Alpha_stab*static_cast<double>(params->game->Matrix_i[3][1]-params->game->Matrix_i[4][1])+Beta*static_cast<double>(params->game->Matrix_i[2][1]-params->game->Matrix_i[4][1])+static_cast<double>(params->game->Matrix_i[4][1]);
    double v_col_equil=Alpha*Beta_stab*u_col+Alpha*static_cast<double>(params->game->Matrix_i[3][0]-params->game->Matrix_i[4][0])+Beta_stab*static_cast<double>(params->game->Matrix_i[2][0]-params->game->Matrix_i[4][0])+static_cast<double>(params->game->Matrix_i[4][0]);


    double wls_r;//wolf learn speed
    if(v_row>v_row_equil)wls_r=1;
    else wls_r=2;
    double wls_c;//wolf learn speed
    if(v_col>v_col_equil)wls_c=1;
    else wls_c=2;

    Alpha=Alpha+Learn_r*wls_r*u_row*Beta+static_cast<double>(params->game->Matrix_i[3][1]-params->game->Matrix_i[4][1]);
    Beta=Beta+Learn_r*wls_c*u_col*Alpha+static_cast<double>(params->game->Matrix_i[2][0]-params->game->Matrix_i[4][0]);

    double prob=static_cast<double>(rand()) / RAND_MAX;
    int retval=0;
    if(params->plnum==1 && prob>Alpha)retval=1; //row player
    else if(params->plnum==0 && prob>Beta)retval=1;//colplayer
    return retval;
}
*/
