#ifndef COMP_DATA_H
#define COMP_DATA_H

#include <QtCore>

class Comp_data : public QObject
{
    Q_OBJECT
public:
    const QStringList symmetric_games ={"ArmsRace","BattleOfTheSexes","BertrandOligopoly","BidirectionalLEG","Chicken","DispersionGame","GrabTheDollar","HawkAndDove","MinimumEffortGame","NPlayerChicken","NPlayerPrisonersDilemma","PrisonersDilemma","RandomCompoundGame","RandomLEG","RockPaperScissors","ShapleysGame","TravelersDilemma","UniformLEG"};
    //not sym: "CollaborationGame" "CoordinationGame" "CournotDuopoly" "CovariantGame","LocationGame","MajorityVoting","MatchingPennies","PolymatrixGame","RandomGame","RandomGraphicalGame","RandomZeroSum","TwoByTwoGame","WarOfAttrition"


    explicit Comp_data(QObject *parent = nullptr);
    const QString db_loc = "/test.db";
//    QString Session="D:Qt/example/test";//default
    bool Signal_algorithms=false;//signal algorithms upon init too read data from this file.

    /*comparison parameters*/
    //settings
    bool Rec_first,Rec_last=true;
    int Num_iteration,Num_rounds,First_x_rounds,Last_x_rounds;
    int Numthreads;
    //algorithms
    QVector<QString> Comparing_Algos; //algorithms that will be compared
    bool Par_comp=true; //pareto optimal
    int Egr_expl; //eGreedy explor level: 0-100
    double Sat_aspir; //Satisficing play  initial aspiration level
    double Sat_pers_rate; //Satisficing play persistence rate
    double Ql_lear; //QL
    double Ql_disc; //QL
    double Ql_expl; //QL
    double Ql_dec_l; //QL
    double Ql_dec_expl;//QL
    bool TFT_smart; //TFT smart mode or not
    int Mark_win;//window size of markov
    int Running_comp;//counter to keep track of what comparison is ran


signals:

public slots:
};

#endif // COMP_DATA_H
