#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QDebug>
#include <QSignalMapper>
#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <game.h>
#include <game_creator.h>



namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(Widget *widget, QWidget *parent = nullptr);
    ~Settings();
    QStandardItemModel *model;

signals:
    void update_algos();//update the algorithms class:new compdata_Read
public slots:
    void Game_Created();
    void reactivate_tab();
private slots:
    void on_pb_create_game_clicked();
    void Table_view_but_clicked(int row);
    void Table_rem_but_clicked(int row);
    void Table_add_row(int row);
    void on_pb_start_clicked();
    void on_btn_open_ses_clicked();
    void on_btn_save_ses_clicked();
    void open_session();
    void write_compdata();
    void read_compdata();
    void read_gamedata_file(QString filename);
//    QStringList Read_file_helper(QTextStream *stream);

private:
        Ui::Settings *ui;
    Widget *widget;

};



#endif
