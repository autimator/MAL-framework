#include "comparison.h"

/*A thread used to  run the comparison itself
All global variables are read threadsafe
*/

comparison::comparison()//constructor, not used
{
    //seed random
    srand (static_cast<uint>(time(nullptr)));
}

comparison::~comparison()//default destructor
{
    // free resources
}

//main function used as thread!
void comparison::process()
{
    qDebug()<<"thread running:"<<comp_data->Last_x_rounds;
    if(I_locked_mutex==false)Mutex.lock();
    iter_params.game=game;//init iter_params for this game

    //for all possible combinations of players
    QVector<QString> players = comp_data->Comparing_Algos;
    int num_pl = game->Players;
    //int n = players.count();
    qDebug()<<"players"<<players<<",num_pl"<<num_pl;

    //if(n<num_pl){qDebug()<<"not enough algorithms to runn this game";emit finished();return;}
    if(true)
    {
        QSqlDatabase  db = QSqlDatabase::addDatabase ("QSQLITE","thread"+QString::number(Thread_id));//create new dataase for this comparison-----------------------------------------------
        db.setDatabaseName(comp_data->Session+comp_data->db_loc);
        db.open();//create new one
        if (!db.open()){qDebug("Error occurred opening the database.");qDebug("%s.", qPrintable(db.lastError().text()));}
        QSqlQuery q(db);

        QString query="create table compdata"+QString::number(Running_game)+" (round integer,rec_FL VARCHAR(1),";
        for(int i=0;i<game->Players;i++)
        {
            query+="pl_id"+QString::number(i)+" VARCHAR(15),";
            //query+="rewf_pl"+QString::number(i)+" integer,";//rewl_pl
            query+="act_pl"+QString::number(i)+" integer,";
            query+="rew_pl"+QString::number(i)+" integer,";
        }
        query.replace(query.count()-1,1,")");
        q.exec(query);
//        overwrite_comp_times();
//        qDebug()<<"querry successfull.";
        all_combination(players,num_pl,&q);
        //qDebug()<<Pl;
        //qDebug()<<Rew_f<<Rew_l;
//        write_comp_times();//must be done before write file...
        writefile(&q);
        db.close();//close database connection------------------------------------------------------------------------------------------------------
    }
    QSqlDatabase::removeDatabase("thread"+QString::number(Thread_id));//database usage must be out iof scope: behind a }....


/*load up next game*/
    Mutex.unlock();//give widget access to the thread
    emit done_game(Running_game,this);
/*starting next comparison*/
    Running_game=-1;//safety procaution. If widget does not grab the mutex in time, thread dies
    thread()->sleep(1);//sleep 1 second, allow widget to grab the mutex
    I_locked_mutex=Mutex.try_lock_for(std::chrono::seconds(6));//try to unlock mutex for 10 seconds
    if(Running_game>0)emit start();//restart running
    else{qDebug()<<"closing thread";Mutex.unlock(); emit finished();}//close thread. done
}


//recursive function to compute all possible num_pl combinations of end items
//Input Array,buffer, start, stop index,current index in data, num of players in one game
void comparison::possible_combinations(QVector<QString> arr, QVector<QString> data, int start, int end,int index, int num_pl)
{
    if (index == num_pl)// combination is ready
    {
        for(int i=0;i<num_pl;i++)
        {
            qDebug()<<data;
            data.append(data.takeFirst());
        }
//        qDebug()<<"----";
        return;
    }
    for (int i=start; i<=end && end-i+1 >= num_pl-index; i++)
    {
        data[index] = arr[i];
        possible_combinations(arr, data, i+1, end, index+1, num_pl);
    }
}

//check if the game is symmetric
void comparison::checksymmetric()
{
    if(comp_data->symmetric_games.contains(game->Generator))Symmertric=true;
    else Symmertric=false;
}

//makes all possible combinations: binary counter
void comparison::all_combination(QVector<QString> players, int num_pl,QSqlQuery *q)
{
    int num_algos=players.count();
    //bool symmertric=checksymmetric();//symmetric game??
    checksymmetric();
    QVector<int> binpl;
    QVector<QString> data;
//    num_algos=num_algos-1;//we count from zero
    for(int i=0;i<num_pl;i++){binpl.append(0);data.append(players[0]);}
    bool done=false;

    while(done==false)
    {
        data[0]=players[binpl[0]];
//        qDebug()<<binpl;
        qDebug()<<data;
        //pl.append(data);
        run_comparison(data);

        if((FL.count()>100000)){writefile(q);}//limit memory usage
        binpl[0]++;
        if(binpl[0]>=num_algos)//update nextnum
        {
            for(int i=0;i<num_pl;i++)
            {
                if(binpl[i]>=num_algos)
                {
//                    if(binpl[i+1]<=num_algos)data[i+1]=players[binpl[i+1]];
//                    data[i]=players[0];
                    if((i+1)<num_pl)
                    {
                        binpl[i+1]++;
                        if(Symmertric==false) //asymmetric games
                        {
                            binpl[i]=0;
                            if(binpl[i+1]<num_algos)data[i+1]=players[binpl[i+1]];
                            data[i]=players[0];
                        }
                        else //symmetric games
                        {
                            for(int j=i;j>=0;j--)binpl[j]=binpl[j+1];
                            if(binpl[i+1]<num_algos){data[i+1]=players[binpl[i+1]];data[i]=players[binpl[i]];}
                        }
                    }
                    else{ done=true;}
                }
            }
        }
    }
}

//lets play the game
void comparison::run_comparison(QVector<QString> players)
{
    Single_pl.append(players);
    for(int round=0;round<comp_data->Num_rounds;round++)//one round
    {
        //Algorithms->init_round(game);//reset the parameters used by multiple players of the same type
        QVector<Comp_algo*> algos;
        //enemies.push_back(new Enemy1());
        //enemies.push_back(new Enemy2());

        QVector<int> actions,rewards;
        //QVector<QString> player_copy=players;
        QVector<int> rewards_first_x,rewards_last_x;
        int last_rounds= comp_data->Num_iteration-comp_data->Last_x_rounds;
        //qDebug()<<"run_comparison:players"<<players;
        //QElapsedTimer timer;
        //qDebug()<<"clock type:"<<timer.clockType();//clock type=4, QElapsedTimer::PerformanceCounter
        //qDebug()<<"clock is monotonic?"<<QElapsedTimer::isMonotonic(); >true
        //QElapsedTimer::PerformanceCounter timer;
//        timer.start();
//        QVector<qint64>rec_times(players.count(),0);
        for(int i=0;i<players.count();i++)//init algorithms
        {
            iter_params.plnum=i;
            actions.append(0);
            rewards_first_x.append(0);
            rewards_last_x.append(0);
            if (players[i]=="best_resp"){algos.append(new Best_resp()); }
            else if (players[i]=="bully"){algos.append(new Bully_own()); }// Bully(&iter_params)
            else if(players[i]=="determined"){algos.append(new Determ(&iter_params));}
            else if (players[i]=="egreedy"){algos.append(new Egreedy(&iter_params,comp_data->Egr_expl)); }
            else if(players[i]=="fict_play"){algos.append(new Fict_pl(&iter_params)); }
            else if(players[i]=="godfather"){algos.append(new Godfather(&iter_params)); }
            else if(players[i]=="markov"){algos.append(new Markov(&iter_params,comp_data->Mark_win)); }
            else if(players[i]=="maxmin"){algos.append(new Maxmin(&iter_params)); }
            else if(players[i]=="nash"){algos.append(new Nash(&iter_params)); }
            else if(players[i]=="ngreedy"){algos.append(new Ngreedy(&iter_params)); }
            else if(players[i]=="noreg"){algos.append(new Noreg(&iter_params)); }
            else if(players[i]=="pareto_opt"){algos.append(new Pareto_opt(&iter_params,comp_data->Par_comp));}
            else if(players[i]=="ql"){algos.append(new QL(&iter_params,comp_data->Ql_lear,comp_data->Ql_disc, comp_data->Ql_expl, comp_data->Ql_dec_l, comp_data->Ql_dec_expl)); }
            else if(players[i]=="random")algos.append(new Random());
            else if(players[i]=="satisF"){algos.append(new SatisF(comp_data->Sat_aspir,comp_data->Sat_pers_rate)); }
            else if(players[i]=="TFT"){algos.append(new TitforTat(&iter_params,comp_data->TFT_smart)); }
            //qDebug()<<"initialising: "<<players[i]<<"took:"<<timer.nsecsElapsed(); timer.start();
//            rec_times[i]=timer.elapsed();//nsecsElapsed();
//            timer.start();
        }
//        Init_times.append(rec_times);
//        rec_times=QVector<qint64>(players.count(),0);

        for(int iter=0;iter<comp_data->Num_iteration;iter++)//one iteration
        {
            iter_params.past_act=actions;
            iter_params.iteration=iter;

            for(int i=0;i<players.count();i++)//get actions
            {
                 iter_params.plnum=i;
//                 timer.start();
                 actions[i]=algos[i]->action(&iter_params);
//                 rec_times[i]=timer.nsecsElapsed();//elapsed();//nsecsElapsed();
            }
//            qDebug()<<iter<<","<<actions;
//            qDebug()<<rewards_first_x<<","<<rewards_last_x;
            //pl 0 = col, then row 1, row 3 etc.
            int cel=game->Calculate_bincel(actions,iter_params.game->Stepsize);
            iter_params.past_rews=game->Matrix_i[cel];
            //FL={};Rew={};Act={};
            if(comp_data->Rec_first==true && iter<comp_data->First_x_rounds)
            {
                FL.append('F');
                Act.append(actions);
                Rew.append(game->Matrix_i[cel]);
                //Pl.append(players);
                //for(int i=0;i<players.count();i++){rewards_first_x[i]+=game->Matrix_i[cel][i];}
            }
            if(comp_data->Rec_last==true && iter>=last_rounds)
            {
                FL.append('L');
                Act.append(actions);
                Rew.append(game->Matrix_i[cel]);
                //Pl.append(players);
                //for(int i=0;i<players.count();i++){rewards_last_x[i]+=game->Matrix_i[cel][i];}
            }          
        }
//        Comp_times.append(rec_times);
    }

}


//write to database
void comparison::writefile(QSqlQuery *q)
{
    QVector<int>pl_ind;
    QString query="INSERT INTO compdata"+QString::number(Running_game)+" (round,rec_FL";
    for(int i=0;i<game->Players;i++){
                  query+=",pl_id"+QString::number(i)+",act_pl"+QString::number(i)+",rew_pl"+QString::number(i);
                  pl_ind.append(i);
    }
    query+=") VALUES ";
    //pl_ind.insert(1,pl_ind.takeFirst());
    QString values;
    int round=-1;//first round also gives 0: 0/value=0
    for(int i=0;i<Rew.count();i++)//for all recordediterations Rounds.count()
    {
        if(i%(comp_data->First_x_rounds+comp_data->Last_x_rounds)==0)round++;
        if(round==comp_data->Num_rounds)round=0;
        values+="("+QString::number(round)+",";//round
        values+="'";values+=FL[i];values+="',";

        int Single_pl_ind;
        if(i>0)Single_pl_ind=i/(comp_data->Num_rounds*(comp_data->First_x_rounds+comp_data->Last_x_rounds));//only store each algorithm combination once.
        else Single_pl_ind=0;
        //qDebug()<<"Single_pl_ind"<<Single_pl_ind;
        for(int j=0;j<Single_pl[Single_pl_ind].count();j++)//for all players
        {
            values+="'"+Single_pl[Single_pl_ind][pl_ind[j]]+"',";
            values+=QString::number(Act[i][pl_ind[j]])+",";
            values+=QString::number(Rew[i][pl_ind[j]])+",";
        }
        values.replace(values.count()-1,1,")");//finish row
        values+=",";
        /*limit the memory usage*/
        //if(i==100){db_writerows(query+values,q);values="";}//buffer has limited size
        //if((i%100000)==0 && i!=0){db_writerows(query+values,q);values=""; }//buffer has limited size
    }
    db_writerows(query+values,q);//
    Single_pl={};FL={};Rew={};Act={};//Rounds={};
}

//write one row of data to the database
void comparison::db_writerows(QString message, QSqlQuery *q)
{
    message.replace(message.count()-1,1,";");//replace last comma with a semicolumn
    q->exec(message);
}

/*
//just open file in write mode so a new file is created & we can append to it in write_comp_times
void comparison::overwrite_comp_times()
{
    QString filename = comp_data->Session+"/times"+"_data"+QString::number(Running_game)+".txt";
    qDebug()<<filename;
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) //endl causes the buffer to be flushed immediately
    {}
    file.close();
}


//write the computation times to a text file, not so much data
void comparison::write_comp_times()
{
    QString filename = comp_data->Session+"/times"+"_data"+QString::number(Running_game)+".txt";
    qDebug()<<filename;
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) //endl causes the buffer to be flushed immediately
    {
        QTextStream stream(&file);//Qdatastream is unreadable on hdd
        stream <<"players, init time (ms), comp time (ms)"<<endl;
        for(int i=0;i<Single_pl.count();i++)
        {
            for(int j=0;j<Single_pl[i].count();j++)//for all players
            {
                stream <<Single_pl[i][j]<<","<<Init_times[i][j]<<","<<Comp_times[i][j]/1000<<endl;
            }
        }
    }
    file.close();
}
*/
