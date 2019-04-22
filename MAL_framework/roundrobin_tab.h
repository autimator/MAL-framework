#ifndef ROUNDROBIN_TAB_H
#define ROUNDROBIN_TAB_H

#include <QWidget>
#include <QStandardItemModel>
#include <widget.h>
#include <game.h>
#include <comp_data.h>

namespace Ui {
class Roundrobin_tab;
}

class Roundrobin_tab : public QWidget
{
    Q_OBJECT

public:
    explicit Roundrobin_tab(Widget *widgetref, QWidget *parent = nullptr);
    ~Roundrobin_tab();
    void List_files();
    void addParentItem(const QString& text );
    void addChildItem(const QString& text);
    void show_game_table(int pl, int act, QStringList gamematrix,QVector<double> cel_col_alpha,QVector<int>pareto_opt_ind);
    void show_tab_matrix(int gamenum,int mode);
    void averaged_grandtable();
    void compute_NE_col(QVector<QVector<QVector<double>>>NE_for_celcol);
    QFont create_spfont();
    Widget *widget;
public slots:
    void update_files();
    void remove_files();

private slots:
    void on_cmb_games_currentIndexChanged(int index);
    void on_comp_NE_clicked();
    void on_comp_paropt_clicked();
    void on_btn_repl_dyn_clicked();

private:
    Ui::Roundrobin_tab *ui;
    QStandardItemModel *cmb_game_mod;
    int num_game_data; //number of combo box opties for each game
    QStringList modes;
    QStringList Players;//the players we show in the round robin table.
    QVector<QVector <int>> conv_roundr_table();
    QVector<QVector <double>>conv_roundr_table_d();

};

#endif // ROUNDROBIN_TAB_H
