#ifndef REPLICATOR_H
#define REPLICATOR_H

#include <widget.h>
#include <QWidget>
#include <QStandardItemModel>
#include <qcustomplot.h>

namespace Ui {
class Replicator;
}

class Replicator : public QWidget
{
    Q_OBJECT

public:
    explicit Replicator(Widget *widgetref,QVector<QVector <double>> round_rob_matrix, QStringList players,QWidget *parent = nullptr);
    ~Replicator();
    void update_tab_matrix();
private slots:
    void on_rb_actuator_toggled(bool checked);
    void on_rb_av_rew_toggled(bool checked);
    void update_siderl(int prop);//int row

    void on_pb_start_clicked();

    void on_slid_birthr_valueChanged(int value);

    void on_graph_scrlbar_valueChanged(int value);
    //void xAxisChanged(QCPRange range);

    void on_pb_cont_clicked();

//protected:
//   void resizeEvent(QResizeEvent *event) override; //override
private:
    Ui::Replicator *ui;
    void update_proportions();
    void repl_step();//slot so it does not block the ui
    Widget *widget;
    QStringList Players;//the players we show in the round robin table.
    QVector<QVector <double>> Round_rob_matrix;//the game matrix
    QVector<double> Proportions;//proportions of the population
    QVector<QVector<double>>Scoretable;
    double Birthrate;//the birthrate.
    int Tick_counter;//x-as: how manny steps of the replicator dynamic have been simulated
    bool Repl_running=false;//to keep track if the replicator is running or not for pause/continue
};

#endif // REPLICATOR_H
