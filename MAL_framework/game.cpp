#include "game.h"


Game::Game()
{

}


Game::Game(int pl, int act)
{
    Players=pl;
    Actions=act;
}

Game::Game(int pl, int act, int min_p,int max_p, bool N,QString gen,QVector<QVector<QVector<double>>> NEds,QVector<QVector<int>> Matr_i,QVector<QVector <int>> PO,QVector <int> POi)
{
    Players=pl;
    Actions=act;
    Min_payoff=min_p;
    Max_payoff=max_p;
    Normalize=N;
    Generator=gen;
    //NE=NEs;
    NE_d=NEds;
    //QVector<double> Cel_col_alpha;
    //Matrix;
    Matrix_i=Matr_i;
    Pareto_opt=PO;
    Pareto_opt_ind=POi;
/*use for calculations*/
    Calculate_stepsize();
    Init_binplayer();
}



//initialise the stepsize vector
void Game::Calculate_stepsize()
{
    Stepsize={Actions,1};//empty list
    //Stepsize.append(Actions);
    //for(int i=2;i<Players;i++)Stepsize.append(Stepsize[i-1]*Actions);

    for(int i=2;i<Players;i++)Stepsize.insert(0,Stepsize[0]*Actions);
    std::swap(Stepsize[0],Stepsize[1]);//switch first and last

    //index used to convert cel back to actions.
    Conv_ind=QVector<int>(Stepsize.count(),0);
    for(int i=0;i<Stepsize.count();i++){Conv_ind[i]=i;}
    Conv_ind.insert(1,Conv_ind.takeFirst());//switch first and last

//    qDebug() <<"stepsize:"<<stepsize;
}

//initialise the binary player instance with zero's
void Game::Init_binplayer()
{
    Binplayer= QVector<int>(Players,0);
    //for(int i=0;i<Players;i++)binplayer.append(0);
}

//cel in the matrix=stepsize * binplayer
int Game::Calculate_bincel(QVector<int> binpl,QVector<int> steps)
{
    int cel=0;
    for(int i=0;i<binpl.count();i++)
    {
      cel+=binpl[i]*steps[i];
    }
    return cel;
}

//from matrix cel bck to one players action
int Game::Calculate_binact(QVector<int> steps,int cel,int plnum)
{
    for(int i=0;i<steps.count();i++)
    {
        if(Conv_ind[i]==plnum)return (cel/steps[Conv_ind[i]]);
        else cel=cel%steps[Conv_ind[i]];
    }
}

//binplayer is a binary counter, we increase it here
QVector<int> Game::update_binpl(QVector<int> binpl, bool*done)
{
    binpl[0]++;
    if(binpl[0]>=Actions)//update nextnum
    {
        for(int i=0;i<binpl.count();i++)
        {
            if(binpl[i]==Actions)
            {
                if((i+1)<binpl.count()){binpl[i+1]++;binpl[i]=0;}
                else{*done=true;}//qDebug()<<"done";
            }
        }
    }
    return binpl;
}
/*
QVector<int> Game::update_binpl(QVector<int> binpl, bool*done)
{
    binpl[0]++;
    if(binpl[0]>=Actions)//update nextnum
    {
        for(int i=0;i<Players;i++)
        {
            if(binpl[i]==Actions)
            {
                if((i+1)<Players){binpl[i+1]++;binpl[i]=0;}
                else{ qDebug()<<"done";*done=true;}
            }
        }
    }
    return binpl;
}
*/
