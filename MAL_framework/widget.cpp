#include "widget.h"
#include "ui_widget.h"
#include "settings.h"
#include "algorithms.h"
#include "comparison.h"
#include "roundrobin_tab.h"
#include "replicator.h"
#include <QResizeEvent>


Widget::Widget(QWidget *parent) ://constructor
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    Widget *Widget = this;

    ui->tabwid1->setStyleSheet("QTabBar::tab { height: 30px; width: 100px; }");
    ui->tabwid1->addTab(new Settings(Widget), QString("Settings"));//.arg(ui->tabwid1->count()+1));
    ui->tabwid1->addTab(new Algorithms(Widget), QString("Algorithms"));//.arg(ui->tabwid1->count()+1)
    ui->tabwid1->addTab(new Roundrobin_tab(Widget), QString("Round rob"));
//    ui->tabwid1->addTab(new Replicator(Widget), QString("replicator"));

    //connect sesion opend signal between settings and algorithms
    QWidget* pWid_set= ui->tabwid1->widget(0); // for the first tab:settings
    QWidget* pWid_algo= ui->tabwid1->widget(1); // for the second tab:algorithms
    connect(pWid_set,SIGNAL(update_algos()),pWid_algo,SLOT(read_compdat()));//after reading new comparison data we update the UI.
    //Session < set by settings
    qDebug()<<"all"<<comp_data.Comparing_Algos;
}

Widget::~Widget()//default destructor
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//runn comparison code
//-----------------------------------------------------------------------------------------------------

//start threads that will runn the comparisons, called from settings
void Widget::comp_threadmanager()
{
    //based on: https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
   if(Threads_running>0){qDebug()<<"already running comparisons";return;}
   emit starting_comparison();//current comp_data can be deleted.
   comp_data.Running_comp=0;
   setup_comp_db();//setup the db

   //comp_data.Running_comp game index.
   //Threads_running: thread index
   while(Threads_running<comp_data.Numthreads)
   {
        if(comp_data.Running_comp==G_data.count()){comp_data.Numthreads=comp_data.Running_comp;break;}//check if there are enough games
        if(comp_data.Comparing_Algos.count()<G_data[comp_data.Running_comp].Players){qDebug()<<"not enough algorithms to runn this game";comp_data.Running_comp++;continue;}        
        QThread* thread = new QThread;
        comparison* worker = new comparison();
        worker ->moveToThread(thread);
        worker->Running_game=comp_data.Running_comp;
        worker->Thread_id=Threads_running;
        worker->game=&G_data[comp_data.Running_comp];
        worker->comp_data=&comp_data;
        connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(start()), worker, SLOT(process()));//so thread can restart itself.
        connect(worker, SIGNAL(done_game(int, comparison*)),this,SLOT(Next_game(int,comparison*)));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
        write_comp_line(comp_data.Running_comp);//update db
        Threads_running++;
        comp_data.Running_comp++;
        ui->tabwid1->tabBar()->hide();//disable tabwidget during comparison
    }
    //no games where started...
    if(Threads_running==0){emit done_comparison();ui->tabwid1->tabBar()->show(); QSqlDatabase::removeDatabase("qt_sql_default_connection");qDebug()<<"no game could be run";}

}

//thread error receiver, so we know somethings up
void Widget::errorString(QString error)
{
    qDebug()<<"Thread error:"<<error;
}

//remove old database if it excists & prep new one
void Widget::setup_comp_db()
{
    QString filename = comp_data.Session+comp_data.db_loc;
    //QFileInfo check_file(filename);
    QSqlDatabase  db = QSqlDatabase::addDatabase ("QSQLITE");//create new dataase for main comparason data
    db.setDatabaseName(filename);
    db.open();//create new one
    if (!db.open()){qDebug("Error occurred opening the database.");qDebug("%s.", qPrintable(db.lastError().text()));}
    QStringList tabels =db.tables();
    QSqlQuery q(db);
//    qDebug()<<"tabels"<<tabels;
//    qDebug()<<"DROP TABLE "+tabels[0]+";";
    for(int i=0;i<tabels.count();i++)q.exec("DROP TABLE "+tabels[i]+";");//remove all excisting tabels in database

    QString query="create table compdata (players VARCHAR(255),gamenum integer,rounds integer, iterations integer,record_f integer, record_l integer, game_pl integer, game_a integer, generator VARCHAR(20), matrix VARCHAR(4096),celcol VARCHAR(400),par_ind VARCHAR(200));";//record_l recording last x rounds yes or no.
    q.exec(query);
    db.close();
}

//write a line to the database: game x was runn with the setting s that where used
void Widget::write_comp_line(int gamenum)
{
    QString player;
    for(int i=0;i<comp_data.Comparing_Algos.count();i++)player+=(comp_data.Comparing_Algos[i]+",");

    QSqlDatabase  db = QSqlDatabase::database();//after first call use a reference for the db connection
    db.setDatabaseName(comp_data.Session+comp_data.db_loc);
    db.open();
    if (!db.open()){qDebug("Error occurred opening the database.");qDebug("%s.", qPrintable(db.lastError().text()));}
    QSqlQuery query(db);
    query.prepare("INSERT INTO compdata (players,gamenum,rounds,iterations, record_f, record_l, game_pl, game_a, generator, matrix, celcol, par_ind) "
                  "VALUES (:pl,:num,:round,:iter, :recf, :recl, :play, :act,:gen, :mat, :col, :par)");
    query.bindValue(":pl", player);
    query.bindValue(":num",gamenum);
    query.bindValue(":round",comp_data.Num_rounds);
    query.bindValue(":iter",comp_data.Num_iteration);
    query.bindValue(":recf", comp_data.First_x_rounds);
    query.bindValue(":recl", comp_data.Last_x_rounds);
    query.bindValue(":play", G_data[gamenum].Players);
    query.bindValue(":act", G_data[gamenum].Actions);
    query.bindValue(":gen", G_data[gamenum].Generator);
    QString matr;
    for(QString cel: G_data[gamenum].Matrix){matr+=(cel)+",";}
    query.bindValue(":mat", matr);
    QString col;
    for(double cel: G_data[gamenum].Cel_col_alpha){col+=(QString::number(round(cel*10)/10))+",";} //rounded at one decimal.
    query.bindValue(":col", col);
    QString parind;
    for(int ind: G_data[gamenum].Pareto_opt_ind){parind+=(QString::number(ind))+",";}
    query.bindValue(":par", parind);
    query.exec();
    db.close();
}

//if there are still games left hand the next one to the thread thats done, otherways close thread down.
void Widget::Next_game(int ran_game,comparison *thread)
{
    qDebug()<<"thread:"<<thread->Thread_id<<",ran_game:"<<ran_game;
    bool lock=thread->Mutex.try_lock_for(std::chrono::seconds(1));
    qDebug()<<comp_data.Running_comp<<","<<(G_data.count()-1);

    while(comp_data.Running_comp<=(G_data.count()-1))//check if game can be played
    {if(comp_data.Comparing_Algos.count()<G_data[comp_data.Running_comp].Players){qDebug()<<"not enough algorithms to runn this game";comp_data.Running_comp++;}
    else break;
    }
    if(comp_data.Running_comp>(G_data.count()-1)||lock==false)//dont start new game
    {
        Threads_running--;
        thread->Running_game=-1;//set negative: thread will shut down
        thread->Mutex.unlock();
        if(Threads_running==0){emit done_comparison();ui->tabwid1->tabBar()->show(); QSqlDatabase::removeDatabase("qt_sql_default_connection");qDebug()<<"done last comparison";}

        return;
    }
    else//runn next game on thread
    {
        qDebug()<<"setting up:"<<comp_data.Running_comp;
        thread->Running_game =comp_data.Running_comp;
        thread->game=&G_data[comp_data.Running_comp];
        write_comp_line(comp_data.Running_comp);//update db
        comp_data.Running_comp++;
        thread->Mutex.unlock();
    }
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//support functions used by both game creator and round robin table: gambit & pareto optimal cells
//-----------------------------------------------------------------------------------------------------


//Gambit call, used by both game_creator and round robin table
//Only problem is that the data comes in through a signal: process ready so it has to be catched in a global variable.
QStringList Widget::CallGambit(QString path, int players, int actions)
{
/*
https://gambitproject.readthedocs.io/en/v15.1.1/tools.html#gambit-enumpure-enumerate-pure-strategy-equilibria-of-a-game
gambit-enumpure: Enumerate pure-strategy equilibria of a game
*/
    Gambit_NE_buff= QStringList{};
    QProcess *Gambit_pros= new QProcess(this);
    connect(Gambit_pros, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
    connect(Gambit_pros, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    Gambit_pros->setWorkingDirectory(Gambit_loc);
    Gambit_pros->setProgram("cmd");
    QStringList arg;
    //arg<<"/C "<<"gambit-enumpure"<<"-S"<< path;

    if(players>2)arg<<"/C "<<"gambit-enumpoly"<<"-S"<< path;//|| actions>2
    else arg<<"/C "<<"gambit-enummixed"<<"-S"<< path;//2 player
    Gambit_pros->setArguments(arg);
    Gambit_pross_done=false;
    Gambit_pros->start(); //startDetached
    while(Gambit_pross_done==false)//wait for gambit to finish
    {
         QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    return(Gambit_NE_buff);//done signal
}

//receive Gambit output, some time one call/NE, others times multiple in one message
void Widget::processReadyRead()
{
    if (QProcess * proc = qobject_cast<QProcess*>(sender()))
    {
        QString result = proc->readAllStandardOutput();
        QStringList NE = result.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
        for(int i=0;i<NE.count();i++){NE[i].remove(0,3);}//remove NE,
        qDebug() << "process output:b " << NE;
        Gambit_NE_buff.append(NE);
    }
}

//Gambit process finished: results are in
void Widget::processFinished(int exitcode, QProcess::ExitStatus stat)
{
    qDebug()<<exitcode<<stat;
    Gambit_pross_done=true;
}

//get a list of NE profiles as double, also converts the mixed NE's
QVector<QVector<QVector<double>>> Widget::conv_NE(QStringList NE,int players, int actions)
{
    QVector<QVector<QVector<double>>>conv_NES;
    for(int i=0;i<NE.count();i++)//for each NE
    {
        QStringList NE_one = NE[i].split(QRegExp(","),QString::SkipEmptyParts);
        QVector<QVector <double>>NE_profile;
        QVector<int>NE_ind;       //{1,0}& forloop from 2 to switch player 1 and 0
        for(int i=0;i<players;i++)NE_ind.append(i);

        for(int j:NE_ind)//for each player
        {
            QVector<double>NE_player;
            for(int k=0;k<actions;k++)//for each action
            {
                int index_split=NE_one[(j*actions)+k].indexOf("/");//mixed NE is x/y: index >0
                if(index_split>0)//convert the division
                {
                    QString convert=NE_one[(j*actions)+k];
                    double one=convert.left(index_split).toDouble();
                    double two=convert.mid(index_split+1).toDouble();
                    NE_player.append(one/two);
                }
                else NE_player.append(NE_one[(j*actions)+k].toDouble());
            }
            NE_profile.append(NE_player);
        }
        qDebug()<<"NE_profile"<<NE_profile;
        conv_NES.append(NE_profile);
    }
    return conv_NES;
}




//Calculate pareto optimal cells: also used by game creator & round robin table
//find the pareto optimal actions of the game
void Widget::Find_pareto_opt_actions(QVector<QVector <int>> matrix,int players,QVector<QVector <int>> &pareto_opt,QVector <int> &pareto_opt_ind )
{
    //based on: https://www.youtube.com/watch?v=efSqXqCyuvg
    QVector<QVector <int>> pareto_opt_cells;
    QVector<int> payoffcel;
    QVector<int>Pareto_indexes;

    for (int i = 0; i< matrix.count(); i++)//for all cells
    {
        QVector<int>tocheck=matrix[i];
        //1.An outcome is Pareto efficient if there is no other outcome that increases at least one player’s payoff without decreasing anyone else’s.
        //2.Likewise, an outcome is Pareto inefficient if another outcome increases at least one player’s payoff without decreasing anyone else’s.
        //3.Note that Pareto efficiency permits indifferences. For example, an outcome that pays <12, 2> Pareto dominates an outcome that pays <9, 2>.
        bool pareto_opt=true;
        for(int j=0;j<pareto_opt_cells.count();j++)//check the already pareto opt declared cells
        {
            pareto_opt=pareto_check_Cel(tocheck,pareto_opt_cells[j],players);
            if(pareto_opt==false)break;
        }
        if(pareto_opt==true)//check all not yet checked cells (an already checked cells that is not pareto opt is dominated by an other one we still have to add)
        {
            for(int j=i+1;j<matrix.count();j++)
            {
                pareto_opt=pareto_check_Cel(tocheck,matrix[j],players);
                if(pareto_opt==false)break;
            }
        }
        if( pareto_opt==true)
        {
//            qDebug()<<"pareto opt:"<<tocheck;
            pareto_opt_cells.append(tocheck);
            Pareto_indexes.append(i);
        }
    }
    pareto_opt=pareto_opt_cells;
    pareto_opt_ind=Pareto_indexes;
}

bool Widget::pareto_check_Cel(QVector<int>tocheck, QVector<int>payoffcel,int players)//compare two cells pareto wise
{
    bool one_decreases=false,one_improves=false;
    for(int k=0;k<players;k++)
    {
        if(tocheck[k]>payoffcel[k])one_improves=true; //strictly higher reward
        else if(tocheck[k]<payoffcel[k])one_decreases=true;//one player gets a lower reward
    }
    if(one_improves==false && one_decreases==true){return(false);}//stop check
    return true;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//general support functions
//-----------------------------------------------------------------------------------------------------
//add replicator dynamic tab with the current round rob matrix for comparisons.
void Widget::add_tab_repl_dyn(QVector<QVector <double>> round_rob_matrix, QStringList players)
{
   if(Repl_dyn_tab>0)
   {
       QWidget* removedtab = ui->tabwid1->widget(Repl_dyn_tab);
       ui->tabwid1->removeTab(Repl_dyn_tab); //removeTab() doesn't delete the widget
       removedtab->deleteLater(); //so you have to delete it yourself
    }
    Replicator *Repl = new Replicator(this,round_rob_matrix,players);

   QLayout *widgetlayout= this->layout();
   widgetlayout->addWidget(Repl);
   setLayout(widgetlayout);
   Repl_dyn_tab=ui->tabwid1->addTab(Repl, QString("replicator"));
}

