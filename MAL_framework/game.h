#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QDebug>

class Game
{
public:

    int Players;
    int Actions;
    int Min_payoff;
    int Max_payoff;
//    int Conv_val;
    bool Normalize;
    //bool Int_payoffs;
    QString Generator;
//    QString Dir;
//    QString Filename;
    QStringList NE;
    QVector<QVector<QVector<double>>> NE_d;//double version of NE: action profile/player.
    QVector<double> Cel_col_alpha;//NE cel color for game_creator show table
    QStringList Matrix;//order: pl1,pl2,pl3, etc.
    QVector<QVector<int>> Matrix_i;//int version of the matrix: pl2,pl1,pl3, etc.
    QVector<QVector <int>> Pareto_opt;//list of pareto optimal actions
    QVector <int> Pareto_opt_ind;//list of indeces of the pareto optimal actions


//     QString filename() const { return Filename; }//getters & setters...

/*use for calculations*/
    QVector<int> Stepsize;//difference between the payofs >>> last player, ....  , pl 1 pl 2
    QVector<int>Conv_ind;//used to convert cel back to actions(pl1 should be first, highest value)
    QVector<int> Binplayer;//binary counter for game matrix: each player is one digit

/*functions*/
    Game();
    Game(int , int);
    Game(int, int, int ,int , bool ,QString ,QVector<QVector<QVector<double>>> ,QVector<QVector<int>> ,QVector<QVector <int>> ,QVector <int> );
    //Game(int pl, int act, int min_p,int max_p, bool N,QString gen,QVector<QVector<QVector<double>>> NEds,QVector<QVector<int>> Matr_i,QVector<QVector <int>> PO,QVector <int> POi);
    void Calculate_stepsize();
    void Init_binplayer();
    int Calculate_bincel(QVector<int> binpl,QVector<int> steps);
    int Calculate_binact(QVector<int> steps,int cel,int plnum);
    QVector<int> update_binpl(QVector<int> binpl,bool*done);
};

#endif // GAME_H
