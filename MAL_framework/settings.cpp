#include "settings.h"
#include "widget.h"
#include "ui_settings.h"
#include "QStandardItemModel"
#include "QFileDialog"

//constructor
Settings::Settings(Widget *widgetref,QWidget *parent) :
    QWidget(parent),ui(new Ui::Settings)
{
    ui->setupUi(this);
    widget=widgetref;
    connect(widget, SIGNAL(done_comparison()), this, SLOT(reactivate_tab()));
//create the game table
    int nr_row=widget->G_data.count(),nr_col=3;
    model = new QStandardItemModel(nr_row,nr_col,this);
    ui->tableView->setModel(model);
    for(int row = 0; row < nr_row; row++)//increase preloaded games are used.
    {
        Table_add_row(row);
    }
    ui->tableView->setColumnWidth(0,int(float(ui->tableView->width())*float (0.6))-6);
    ui->tableView->setColumnWidth(1,int(float(ui->tableView->width())*float (0.2))-5);//too make it fir nicely in the table
    ui->tableView->setColumnWidth(2,int(float(ui->tableView->width())*float (0.2))-5);
    open_session();//now for debugging: load test session on creation
}


//add a row to the game table
void Settings::Table_add_row(int row)
{
    QList<QStandardItem*> newRow;
    QStandardItem *item1 = new QStandardItem("game"+QString::number(row));//widget->g_data[row].Filename
    QStandardItem *item2 = new QStandardItem();
    QStandardItem *item3 = new QStandardItem(QString("col2"));
    newRow.append(item1);
    newRow.append(item2);
    newRow.append(item3);
    model->appendRow(newRow);

    //we set the buttons directly in the table.
    QString butviewtext = QString("View");
    QPushButton* butview = new QPushButton(butviewtext);
    butview->setFixedWidth(int(float(ui->tableView->width())*float (0.2))-5);
    connect(butview, &QPushButton::clicked,[this, row]() {Table_view_but_clicked(row);});//function we call

    QString butremtext = QString("Rem");
    QPushButton* butrem = new QPushButton(butremtext);
    butrem->setFixedWidth(int(float(ui->tableView->width())*float (0.2))-5);
    connect(butrem, &QPushButton::clicked,[this, row]() {Table_rem_but_clicked(row);});//function we call

    QModelIndex indexb= model->index(row,1,QModelIndex());
    QModelIndex indexc= model->index(row,2,QModelIndex());
    ui->tableView->setIndexWidget(indexb, butview);
    ui->tableView->setIndexWidget(indexc, butrem);
    //has to be done after each edit...
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    //ui->tableView->setColumnWidth(0,int(float(ui->tableView->width())*float (0.5))-6);
    ui->tableView->setColumnWidth(1,int(float(ui->tableView->width())*float (0.2))-5);//too make it fir nicely in the table
    ui->tableView->setColumnWidth(2,int(float(ui->tableView->width())*float (0.2))-5);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//remove a game
void Settings::Table_rem_but_clicked(int row)
{
     model->removeRows( row, 1 );
     widget->G_data.removeAt(row);
     for(int i=row;i<widget->G_data.count();i++)//repair indices
     {
        model->removeRows( i, 1 );//the row reference is used in the button so table has to be rebuild.
        Table_add_row(i);
     }
}

//view game: open the game in the game creator
void Settings::Table_view_but_clicked(int row)
{
//    qDebug() << "view, row:"<<row;
    Game_creator *gamecr;
    gamecr=new Game_creator(widget,row);//create game creation menu
    gamecr->show();
    connect(gamecr,SIGNAL(Game_createClosed()),this,SLOT(Game_Created()));
}

//default destructor
Settings::~Settings()
{
    delete ui;
}

//create new game
void Settings::on_pb_create_game_clicked()
{
    Game_creator *gamecr;
    gamecr=new Game_creator(widget,widget->G_data.count());//create game creation menu
    gamecr->show();
    connect(gamecr,SIGNAL(Game_createClosed()),this,SLOT(Game_Created()));
}


//called upon closing of game_creator
void Settings::Game_Created()
{
    model->clear();//remove old list of games
    //add all games that are now in widget.
    for(int i=0;i<widget->G_data.count();i++){qDebug()<<"Adding row!!";Table_add_row(i);}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//runn comparison code
//-----------------------------------------------------------------------------------------------------
void Settings::on_pb_start_clicked()
{
    //load the comp_data instance with the settings used in the comparison
    widget->comp_data.Rec_last=ui->cb_rec_last->isChecked();
    widget->comp_data.Rec_first=ui->cb_rec_first->isChecked();
    if(ui->cb_rec_last->isChecked())widget->comp_data.Last_x_rounds=ui->LE_rec_lrounds->text().toInt();
    else widget->comp_data.Last_x_rounds=0;
    if(ui->cb_rec_first->isChecked())widget->comp_data.First_x_rounds=ui->LE_rec_frounds->text().toInt();
    else widget->comp_data.First_x_rounds=0;
    widget->comp_data.Num_iteration=ui->LE_niter->text().toInt();
    widget->comp_data.Num_rounds=ui->LE_nrounds->text().toInt();
    widget->comp_data.Numthreads=ui->LE_threads->text().toInt();
    ui->pb_start->setEnabled(false);//diable ui
    ui->pb_create_game->setEnabled(false);
    ui->tableView->setEnabled(false);
    ui->btn_open_ses->setEnabled(false);
    ui->btn_save_ses->setEnabled(false);
    widget->comp_threadmanager();
}

void Settings::reactivate_tab()//re-activate ui after comparison
{
    ui->pb_start->setEnabled(true);
    ui->pb_create_game->setEnabled(true);
    ui->tableView->setEnabled(true);
    ui->btn_open_ses->setEnabled(true);
    ui->btn_save_ses->setEnabled(true);
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//open and save sessions
//-----------------------------------------------------------------------------------------------------
//open session:select directory
void Settings::on_btn_open_ses_clicked()
{
    widget->comp_data.Session=QFileDialog::getExistingDirectory(this, tr("Open Directory"),widget->comp_data.Session,QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    open_session();
}

//load the game files
void Settings::open_session()
{
    qDebug()<<"session"<<widget->comp_data.Session;
    QStringList session_parts=widget->comp_data.Session.split('/',QString::SkipEmptyParts);
    ui->lbl_sesion->setText(session_parts.last());
    bool checkgames=true;
    int i=1;
    widget->G_data.clear();//remove all game
    model->clear();
    read_compdata();
    emit update_algos();
    while(checkgames==true)//for all games
    {
        QString filename=widget->comp_data.Session+"/Game"+QString::number(i)+"_data.txt";
        QFileInfo check_file(filename);
        if(!(check_file.exists() && check_file.isFile())){checkgames=false;break;}
        read_gamedata_file(filename);
        i++;
    }
    Game_Created();//show table.
//    ui->tableView->setColumnWidth(0,int(float(ui->tableView->width())*float (0.6))-6);
//    ui->tableView->setColumnWidth(1,int(float(ui->tableView->width())*float (0.2))-5);//table format needs a reset after the model.clear
//    ui->tableView->setColumnWidth(2,int(float(ui->tableView->width())*float (0.2))-5);
}

//save all game data to hdd: game > game_data
void Settings::on_btn_save_ses_clicked()
{
    //remove old files from hdd
    bool checkgames=true;
    int i=1;
    while(checkgames==true)//for all games
    {
        QString filename=widget->comp_data.Session+"/Game"+QString::number(i)+"_data.txt";
        QFileInfo check_file(filename);
        if(!(check_file.exists() && check_file.isFile())){checkgames=false;break;}
        QFile::remove(filename);//remove it
        qDebug()<<"removing"<<filename;
        i++;
    }

    write_compdata();//save comp data
    for(int i=0;i<widget->G_data.count();i++)//for each game
    {
        Game *game=&widget->G_data[i];
        QString filename = widget->comp_data.Session+"/Game"+QString::number(i+1)+"_data.txt";
        qDebug()<<filename;
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) //endl causes the buffer to be flushed immediately
        {
            QTextStream stream(&file);//Qdatastream is unreadable on hdd
            stream <<"players, actions, min payoff, max payoff"<<endl;
            stream << game->Players<<","<<game->Actions<<","<<game->Min_payoff<<","<<game->Max_payoff<<endl;
            stream <<"normalize"<<endl;//, Int_payoffs"<<endl;
            stream << game->Normalize<<endl;//<<","<<game->Int_payoffs<<endl;
            stream <<"generator"<<endl;
            stream << game->Generator<<endl;
            stream <<"NE"<<endl;
            for(int j=0;j<game->NE.count();j++){stream << game->NE[j]<<",";}stream<<endl;
            stream <<"NE_d"<<endl;
            for(int j=0;j<game->NE_d.count();j++)//list of list of list
            {
                for(int k=0;k<game->NE_d[j].count();k++){for(int l=0;l<game->NE_d[j][k].count();l++){stream<<game->NE_d[j][k][l];stream<<",";}}
            }stream<<endl;
            stream <<"cel_col_alpha"<<endl;
            for(int j=0;j<game->Cel_col_alpha.count();j++){stream << game->Cel_col_alpha[j]<<",";}stream<<endl;
            stream <<"Matrix"<<endl;
            for(int j=0;j<game->Matrix.count();j++){stream << game->Matrix[j]<<",";}stream<<endl;
            stream <<"matrix_i"<<endl;
            for(int j=0;j<game->Matrix_i.count();j++)//list of list
            {
                for(int k=0;k<game->Matrix_i[j].count();k++){stream<<game->Matrix_i[j][k];stream<<",";}
            }stream<<endl;
            stream <<"pareto_opt"<<endl;
            for(int j=0;j<game->Pareto_opt.count();j++)//list of list
            {
                for(int k=0;k<game->Pareto_opt[j].count();k++){stream<<game->Pareto_opt[j][k];stream<<",";}
            }stream<<endl;
            stream <<"pareto_opt_ind"<<endl;
            for(int j=0;j<game->Pareto_opt_ind.count();j++){stream<<game->Pareto_opt_ind[j];stream<<",";}stream<<endl;
            stream <<"stepsize"<<endl;
            for(int j=0;j<game->Stepsize.count();j++){stream<<game->Stepsize[j];stream<<",";}stream<<endl;
            stream <<"conv_ind"<<endl;
            for(int j=0;j<game->Conv_ind.count();j++){stream<<game->Conv_ind[j];stream<<",";}stream<<endl;
            stream <<"binplayer"<<endl;
            for(int j=0;j<game->Binplayer.count();j++){stream<<game->Binplayer[j];stream<<",";}stream<<endl;
/*            qDebug()<<game->Players;
            qDebug()<<game->Actions;
            qDebug()<<game->Min_payoff;
            qDebug()<<game->Max_payoff;
            qDebug()<<game->Normalize;
            qDebug()<<game->Int_payoffs;
            qDebug()<<game->generator;
            qDebug()<<game->NE;
            qDebug()<<game->NE_d;
            qDebug()<<game->cel_col_alpha;
            qDebug()<<game->Matrix;
            qDebug()<<game->matrix_i;
            qDebug()<<game->pareto_opt;
            qDebug()<<game->pareto_opt_ind;
            qDebug()<<game->stepsize;
            qDebug()<<game->Conv_ind;
            qDebug()<<game->binplayer;
*/
            file.close();
        }
    }
}

//read all game_data files from the hdd to game class instances
void Settings::read_gamedata_file(QString filename)
{
    QFile file(filename);
    Game game;
    qDebug()<<"file:"<<file;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString line = stream.readLine();
        line = stream.readLine();
        QStringList info=line.split(',',QString::SkipEmptyParts);
        game.Players=info[0].toInt();game.Actions=info[1].toInt();game.Min_payoff=info[2].toInt();game.Max_payoff=info[3].toInt();
        int plact=game.Players*game.Actions;
        //qDebug()<<game.Players;qDebug()<<game.Actions;qDebug()<<game.Min_payoff;qDebug()<<game.Max_payoff;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        game.Normalize= (info[0] == "1" ? true : false);//game.Int_payoffs= (info[1] == "1" ? true : false);
        //qDebug()<<game.Normalize;qDebug()<<game.Int_payoffs;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        game.Generator=info[0];
        //qDebug()<<game.Generator;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<(info.count()/plact);i++)for(int j=0;j<plact;j++){if(j==0)game.NE.append(info[plact*i+j]);else game.NE[i].append(","+info[plact*i+j]);}
        //qDebug()<<game.NE;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<game.NE.count();i++)
        {
            QVector<QVector<double>>profile;
            for(int j=0;j<game.Players;j++)
            {
                QVector<double>player;
                for(int k=0;k<game.Actions;k++)player.append(info[i*plact+j*game.Actions+k].toDouble());
                profile.append(player);
            }
            game.NE_d.append(profile);
        }//qDebug()<<game.NE_d;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Cel_col_alpha.append(info[i].toDouble());}
        //qDebug()<<game.Cel_col_alpha;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Matrix.append(info[i]);}
        //qDebug()<<"mat"<<game.Matrix;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<(info.count()/game.Players);i++)
        {
            QVector<int>pl;
            for(int j=0;j<game.Players;j++)pl.append(info[i*game.Players+j].toInt());
            game.Matrix_i.append(pl);
        }
        //qDebug()<<"mati"<<game.Matrix_i;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<(info.count()/game.Players);i++)
        {
            QVector<int>pl;
            for(int j=0;j<game.Players;j++)pl.append(info[i*game.Players+j].toInt());
            game.Pareto_opt.append(pl);
        }
        //qDebug()<<"par"<<game.Pareto_opt;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Pareto_opt_ind.append(info[i].toInt());}
        //qDebug()<<"par_i"<<game.Pareto_opt_ind;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Stepsize.append(info[i].toInt());}        
//        qDebug()<<"steps"<<game.Stepsize;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Conv_ind.append(info[i].toInt());}
//        qDebug()<<"steps"<<game.Conv_ind;

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);//qDebug()<<info;
        for(int i=0;i<info.count();i++){ game.Binplayer.append(info[i].toInt());}
        //qDebug()<<"binpl"<<game.Binplayer;
    }
    file.close();
    widget->G_data.append(game);
//    qDebug()<<"finishing load";
}

//write compdata to a file
void Settings::write_compdata()
{
    QString filename = widget->comp_data.Session+"/Comp_data.txt";
    qDebug()<<filename;
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) //endl causes the buffer to be flushed immediately
    {
        QTextStream stream(&file);//Qdatastream is unreadable on hdd
        stream <<"Comparing_Algos"<<endl;
        for(int j=0;j<widget->comp_data.Comparing_Algos.count();j++){stream << widget->comp_data.Comparing_Algos[j]<<",";}stream<<endl;
        stream <<"N_thr, N_r, N_iter, F_x_r, L_x_r;"<<endl;
        stream<<ui->LE_threads->text()<<","<<ui->LE_nrounds->text()<<","<<ui->LE_niter->text()<<","<<ui->LE_rec_frounds->text()<<","<<ui->LE_rec_lrounds->text()<<endl;
        stream <<"Rec_first, last"<<endl;//, Int_payoffs"<<endl;
        stream << ui->cb_rec_first->isChecked()<<","<<ui->cb_rec_last->isChecked()<<endl;
        stream <<"Running_comp"<<endl;
        stream <<widget->comp_data.Running_comp<<endl;

        stream <<"Egr_expl"<<endl;
        stream <<widget->comp_data.Egr_expl<<endl;
        stream <<"Mark_win"<<endl;
        stream <<widget->comp_data.Mark_win<<endl;
        stream <<"Par_comp"<<endl;
        stream <<widget->comp_data.Par_comp<<endl;
        stream <<"Ql_lear, disc, expl, dec_l, dec_expl"<<endl;
        stream <<widget->comp_data.Ql_lear<<","<<widget->comp_data.Ql_disc<<","<<widget->comp_data.Ql_expl<<","<<widget->comp_data.Ql_dec_l<<","<<widget->comp_data.Ql_dec_expl<<endl;
        stream <<"Sat_aspir, pers_rate"<<endl;
        stream <<widget->comp_data.Sat_aspir<<","<<widget->comp_data.Sat_pers_rate<<endl;
        stream <<"TFT_smart"<<endl;
        stream <<widget->comp_data.TFT_smart<<endl;
    }
    file.close();
}

//read compdata to a file
//and signal algorithms to pick up the information
void Settings::read_compdata()
{
    QString filename = widget->comp_data.Session+"/Comp_data.txt";
    widget->comp_data.Signal_algorithms=true;//signal to algorithms on start (the signal is not yet connected because the class is not created.)
    QFile file(filename);
    qDebug()<<"file:"<<file;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString line = stream.readLine();
        line = stream.readLine();
        QStringList info=line.split(',',QString::SkipEmptyParts);
        QVector<QString> variable;
        for(int i=0;i<info.count();i++){ variable.append(info[i]);}
        widget->comp_data.Comparing_Algos=variable;
        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);
        widget->comp_data.Numthreads=info[0].toInt();
        ui->LE_threads->setText(info[0]);
        widget->comp_data.Num_rounds=info[1].toInt();
        ui->LE_nrounds->setText(info[1]);
        widget->comp_data.Num_iteration=info[2].toInt();
        ui->LE_niter->setText(info[2]);
        widget->comp_data.First_x_rounds=info[3].toInt();
        ui->LE_rec_frounds->setText(info[3]);
        widget->comp_data.Last_x_rounds=info[4].toInt();
        ui->LE_rec_lrounds->setText(info[4]);

        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);
        widget->comp_data.Rec_first= (info[0] == "1" ? true : false);
        ui->cb_rec_first->setChecked(info[0] == "1" ? true : false);
        widget->comp_data.Rec_last= (info[1] == "1" ? true : false);
        ui->cb_rec_last->setChecked(info[1] == "1" ? true : false);

        line = stream.readLine();line = stream.readLine();
        widget->comp_data.Running_comp=line.toInt();
        line = stream.readLine();line = stream.readLine();
        widget->comp_data.Egr_expl=line.toInt();
        line = stream.readLine();line = stream.readLine();
        widget->comp_data.Mark_win=line.toInt();
        line = stream.readLine();line = stream.readLine();
        widget->comp_data.Par_comp= (line == "1" ? true : false);
        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);
        widget->comp_data.Ql_lear=info[0].toDouble();
        widget->comp_data.Ql_disc=info[1].toDouble();
        widget->comp_data.Ql_expl=info[2].toDouble();
        widget->comp_data.Ql_dec_l=info[3].toDouble();
        widget->comp_data.Ql_dec_expl=info[4].toDouble();
        line = stream.readLine();line = stream.readLine();
        info=line.split(',',QString::SkipEmptyParts);
        widget->comp_data.Sat_aspir=info[0].toDouble();
        widget->comp_data.Sat_pers_rate=info[1].toDouble();
        line = stream.readLine();line = stream.readLine();
        widget->comp_data.TFT_smart= (line == "1" ? true : false);
    }
    file.close();

    //qDebug()<<"read_compdat"<<widget->comp_data.Comparing_Algos;
}




