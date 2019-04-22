#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QWidget>
#include <QDateTime>
#include <QDebug>
#include <game.h>
#include <widget.h>
#include <dlib/global_optimization.h>

//#include <QSqlDatabase>
//#include <QSqlQuery>
//#include <QSqlError>

namespace Ui {
class Algorithms;
}

class Algorithms : public QWidget
{
    Q_OBJECT

public:
    explicit Algorithms(Widget *widget, QWidget *parent = nullptr);
    ~Algorithms();
private slots:
    void read_compdat();

    void on_pushButton_clicked();

    void combinationUtil(QList<QString> arr, QList<QString> data, int start, int end,int index, int r);
    void all_combination(QList<QString> players, int num_pl,bool symmertric);


    void on_rb_par_comp_toggled(bool checked);
    void on_rb_par_coop_toggled(bool checked);
    void on_cb_al_rand_toggled(bool checked);
    void on_cb_al_fict_toggled(bool checked);
    void on_cb_al_det_toggled(bool checked);
    void on_cb_al_par_opt_toggled(bool checked);
    void on_CB_Bully_toggled(bool checked);
    void on_cb_best_resp_toggled(bool checked);
    void on_cb_Egreedy_toggled(bool checked);
    void on_CB_GodFather_toggled(bool checked);
    void on_CB_Maxmin_toggled(bool checked);
    void on_cb_Nash_toggled(bool checked);
    void on_cb_Ngreedy_toggled(bool checked);
    void on_cb_ql_toggled(bool checked);
    void on_cb_satisfising_toggled(bool checked);
    void on_cb_Noregret_toggled(bool checked);
    void on_cb_TFT_toggled(bool checked);
    void on_cb_smart_toggled(bool checked);
    void on_slid_Sat_pers_valueChanged(int value);
    void on_slid_ql_expl_valueChanged(int value);
    void on_slid_QL_learn_Dec_valueChanged(int value);
    void on_slid_ql_learn_valueChanged(int value);
    void on_slid_QL_disc_valueChanged(int value);
    void on_slid_expl_dec_valueChanged(int value);
    void on_slid_Egr_expl_valueChanged(int value);

    void on_le_sat_aspir_editingFinished();
    void on_cb_Markov_toggled(bool checked);
    void on_slid_Mark_w_valueChanged(int value);

    void on_Pb_tune_clicked();

private:
    Ui::Algorithms *ui;
    Widget *widget;

    double EGr_tune(double a, double b, double c, double d, double e);
    double run_mini_comparison(QVector<QString> players,QString test_alg,IterParam pars,double Ql_lear,double Ql_disc, double Ql_expl, double Ql_dec_l, double Ql_dec_expl);
    //    Game test1(int pl=2,int act=3);//,2,0,100,TRUE,"PrisonersDillema",{{{0,1},{1,0}}},{{0,1},{2,3},{3,4},{5,6}},{{2,3}},{1});
    //Game testa= Game(int pl=2, int act=2, int min_p=0,int max_p=100, bool N=TRUE,QString gen="PrisonersDillema",QVector<QVector<QVector<double>>> NEds={{{0,1},{1,0}}},QVector<QVector<int>> Matr_i={{0,1},{2,3},{3,4},{5,6}},QVector<QVector <int>> PO={{2,3}},QVector <int> POi={1});
    //Game testa= Game(2,3,0,100,TRUE,"PrisonersDillema",{{{0,1},{1,0}}},{{0,1},{2,3},{3,4},{5,6}},{{2,3}},{1});

};

#endif // ALGORITHMS_H
