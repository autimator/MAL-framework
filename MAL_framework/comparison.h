#ifndef COMPARISON_H
#define COMPARISON_H
#include <QtCore>
#include <QDebug>
#include <game.h>
#include <comp_data.h>
#include <comp_algo.h>
//#include <widget.h>


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class comparison : public QObject {

Q_OBJECT

public:

    comparison();//called upon creating thread
    ~comparison();
    int Running_game;//game number thats being calculated
    Game *game;//game object the calculations are dne for
    Comp_data *comp_data;//compare data: settings for the comparison
    int Thread_id;
    QMutex Mutex;

private:
    void possible_combinations(QVector<QString> arr, QVector<QString> data, int start, int end,int index, int num_pl);
    void all_combination(QVector<QString> players, int num_pl,QSqlQuery *q);
    void run_comparison(QVector<QString> players);
    void writefile(QSqlQuery *q);
    void db_writerows(QString message, QSqlQuery *q);
//    void overwrite_comp_times();
//    void write_comp_times();
    void checksymmetric();
    bool I_locked_mutex=false;//if the thread itself locked the mutex.
    bool Symmertric; //wheather we are playing a symmetric game or not
/*comparison output*/
    //QVector<QVector<QString>> Pl; //store list of the players that played the round
    QList<QVector<int>> Rew,Act;//the rewards earned
    QList<char>FL;//if we are recording first or last round now
    //QVector<short> Rounds;//for each round that was played
    QList<QVector<QString>> Single_pl;
    QList<QVector<qint64>> Init_times;//get the init time for each algorithm
    QList<QVector<qint64>> Comp_times;
    //Comp_algo *Algorithms=new Comp_algo();//only has to be created once/thread...
    IterParam iter_params;//only has to be created once/thread...
public slots:
    void process();
//    void no_new_game();
//    void new_game(int game_num ,Game* new_game);
//    void init();
signals:
//    void prepfinish();
    void finished();
    void error(QString err);
    void done_game(int,comparison*);
    void start();

};

#endif // COMPARISON_H
