#ifndef GAME_CREATOR_H
#define GAME_CREATOR_H

#include <QtCore>
#include <QWidget>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QStandardItemModel>
#include <game.h>
#include <widget.h>



namespace Ui {
class Game_creator;
}

class Game_creator : public QWidget
{
    Q_OBJECT


signals:
    void Game_createClosed();
protected:
    void closeEvent ( QCloseEvent * event );

public:    
    explicit Game_creator(Widget*widgetref,int next_game_number, QWidget *parent= nullptr);
    ~Game_creator();

private slots:
    void on_Btn_close_clicked();
    void on_Btn_create_clicked();
    void on_cb_norm_stateChanged();
    void on_Generator_CMB_currentTextChanged(const QString &arg1);



private:
    Ui::Game_creator *ui;
    Widget *widget;
    //bool Creating_gamut=false;   //bool used to check if gamut has finished making the game
    Game *Creating_Game;        //game that is shown in the ui/being created
    QString Dir;                //file directory
    QString Filename;           //filename of game that being created
    QStandardItemModel *Tabmodel; //model used to create the game table
    void Gamut();
    void fill_gametable(Game *curgame);
    void compute_NE_col(Game *curgame);
    bool fileExists(QString path);
    void readTxt(QString path);
    void convert_gamematrix(QStringList curmat);
    //void conv_NE(QStringList NE);
};

#endif // GAME_CREATOR_H
