#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QDebug>
#include <comparison.h>
#include <game.h>
#include <comp_data.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT



public:
//    const QString Gamut_loc=  "D:/Onedrive/master/thesis/Qt/text_finder_example/ext_tools/";
//    const QString Gambit_loc= "C:/Program Files (x86)/Gambit/";
//    const QString Java_loc ="C:/Program Files (x86)/Common Files/Oracle/Java/javapath/java.exe";//java location, needed for Gamut

    //session stored in comp_Data
    //just as the name of the database: db_loc

    QList<Game> G_data{};
    Comp_data comp_data;
    int Num_threads_running;

    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
    void comp_threadmanager();
    QStringList CallGambit(QString path, int players, int actions);//gambit call
    void Find_pareto_opt_actions(QVector<QVector <int>> matrix,int players,QVector<QVector <int>> &pareto_opt,QVector <int> &pareto_opt_ind );
    QVector<QVector<QVector<double>>> conv_NE(QStringList NE,int players, int actions);
    void add_tab_repl_dyn(QVector<QVector <double>> round_rob_matrix, QStringList players);

public slots:
    void errorString(QString);
    void Next_game(int,comparison *thread);


signals:
    void starting_comparison();
    void done_comparison();


private slots:
//    void on_Btn_split_clicked();
    void processReadyRead();
    void processFinished(int exitcode, QProcess::ExitStatus stat);


private:
    Ui::Widget *ui;
//    void set_tabsize(QTabWidget *Temp);
//    int Nr_screens =2;//on start split screen
    int Threads_running=0;
    int Repl_dyn_tab=-1;//for creating&updating of the replicator tab.

    bool Gambit_pross_done; //for gambit process
    QStringList Gambit_NE_buff;//for gambit process
    bool pareto_check_Cel(QVector<int>tocheck, QVector<int>payoffcel,int players);
    void setup_comp_db();
    void write_comp_line(int gamenum);
};

#endif // WIDGET_H
