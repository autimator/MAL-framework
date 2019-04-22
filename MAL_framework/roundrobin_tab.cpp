#include "roundrobin_tab.h"
#include "ui_roundrobin_tab.h"

Roundrobin_tab::Roundrobin_tab(Widget *widgetref,QWidget *parent) ://constructor
    QWidget(parent),ui(new Ui::Roundrobin_tab)
{
    ui->setupUi(this);
    widget=widgetref;
    num_game_data=1;modes.append("average_game");

    connect(widget, SIGNAL(starting_comparison()), this, SLOT(remove_files()));
    connect(widget, SIGNAL(done_comparison()), this, SLOT(update_files()));
//    if(widget->comp_data.rec_first==true){num_game_data++;modes.append("first_rounds");}
//    if(widget->comp_data.rec_last==true){num_game_data++;modes.append("last_rounds");}//number of opties for each game
    List_files();    
}

Roundrobin_tab::~Roundrobin_tab()//default destructor
{
    delete ui;
}

//slot: called after running a new comparisonto remove the game_comp files when starting a new comparison
void Roundrobin_tab::remove_files()
{
    int gamenum=0;
    ui->cmb_games->clear();
    while(true)
    {
        QString filename=widget->comp_data.Session+"/comp_game"+QString::number(gamenum+1)+".txt";
//        qDebug()<<"mat_tab"<<filename;
        QFileInfo check_file(filename);
        if(!(check_file.exists() && check_file.isFile())){return;}//no file
        QFile file(filename);
        file.remove();
        gamenum++;
    }
}

//slot: called after running a new comparison
void Roundrobin_tab::update_files()
{
    num_game_data=1;modes=QStringList{"average_game"};
    List_files();
}

//load the list of games that was played, including if the first/last rounds were recorded
void Roundrobin_tab::List_files()
{
    bool first_Game=true;
    qDebug()<<"session"<<widget->comp_data.Session;
    //bool checkgames=true;
    int i=0;
    cmb_game_mod=new QStandardItemModel();
    //top item:average grand table
    QStandardItem* item = new QStandardItem("average_all");
    QFont font = item->font();
    font.setBold(true);//font.setItalic( true );
    item->setFont(font);
    cmb_game_mod->appendRow(item);

    //int compdata_rows;
    qDebug()<<"openning database";
    if(true)//get number of rows in compdata
    {
        QSqlDatabase  db;
        if( !QSqlDatabase::contains("roundrobin_table" )){db= QSqlDatabase::addDatabase ("QSQLITE","roundrobin_table");}//connecto to the database
        else db = QSqlDatabase::database("roundrobin_table");//connecto to the database
        db.setDatabaseName(widget->comp_data.Session+widget->comp_data.db_loc);
        db.open();
        if (!db.open()){qDebug("Error occurred opening the database. Round robin");qDebug("%s.", qPrintable(db.lastError().text()));}
        QSqlQuery q(db);
        q.exec("SELECT gamenum,generator,record_f,record_l FROM compdata ORDER BY gamenum ;");//DESC LIMIT 1

        //checkgames==true &&
        while(q.next())//for all games
        {
            qDebug()<<"rr"<<q.value(0).toInt();
    //        qDebug()<<"showing games"<<i;
            addParentItem("Game"+QString::number(q.value(0).toInt())+": "+q.value(1).toString());
            if(first_Game==true)
            {
                if(q.value(2).toInt()>0){num_game_data++;modes.append("first_rounds");}
                if(q.value(3).toInt()>0){num_game_data++;modes.append("last_rounds");}
                first_Game=false;
            }
            if(modes.contains("first_rounds"))addChildItem("Game"+QString::number(i)+": first");
            if(modes.contains("last_rounds")){addChildItem("Game"+QString::number(i)+": last");}
            i++;
        }
    }
    qDebug()<<"closing database list items";
    QSqlDatabase::removeDatabase("roundrobin_table");//database usage must be out iof scope: behind a }....
    ui->cmb_games->setModel(cmb_game_mod);
}

//helper for List_files, add a parent item:game
void Roundrobin_tab::addParentItem( const QString& text )
{
    QStandardItem* item = new QStandardItem( text );
    QFont font = item->font();
    font.setBold( true );//font.setItalic( true );
    item->setFont( font );
    cmb_game_mod->appendRow(item);
}

//helper for List_files, add a child item, first/last rounds
void Roundrobin_tab::addChildItem(const QString& text )
{
    QStandardItem* item = new QStandardItem( QString( 4, QChar( ' ' ) )+text  );
    cmb_game_mod->appendRow(item);
}

//different game was selected: show it and the results.
void Roundrobin_tab::on_cmb_games_currentIndexChanged(int index)
{
    qDebug()<<"indexchanged"<<index;
    ui->listW_NE->clear();
    ui->listW_NEs->clear();
    ui->listW_paropt->clear();
    if(index<0)return;
    else if(index==0){averaged_grandtable();return;}

    index--;//remove one for grandtable average
    QStandardItem * item =cmb_game_mod->item(index,0);
    qDebug() << item->text();
    int game_ind=index/num_game_data;
    int mode=index%num_game_data;
    qDebug()<<game_ind;
    //show_game_table(&widget->G_data[game_ind]);
    qDebug()<<mode;
    show_tab_matrix(game_ind,mode);

}

//create the game table
void Roundrobin_tab::show_game_table(int pl, int act, QStringList gamematrix,QVector<double> cel_col_alpha,QVector<int>pareto_opt_ind)
{
//    int pl=curgame->Players;
//    int act =curgame->Actions;
    //int n_row=curgame->Stepsize.last();
    int n_row=static_cast<int>(pow(act, pl-1));
    int n_col=act;

    QStandardItemModel *game_tab_mod=new QStandardItemModel(n_row,n_col,this);
    ui->tab_game->setModel(game_tab_mod);
    QBrush my_brush;
    my_brush.setStyle(Qt::SolidPattern);

    for(int col = 0; col < n_col; col++)//create the game table
    {
        //if(n_row<=4)ui->tab_game->setColumnWidth(col,(ui->tab_game->width()-20)/n_col);
        for(int row = 0; row < n_row; row++)
        {
            //if(n_col<=4)ui->tab_game->setRowHeight(row,(ui->tab_game->height()-24)/n_row);
            QStandardItem *item = new QStandardItem(QString("-,-"));
            game_tab_mod->setItem(row, col, item);
            QString tabtext=(gamematrix[(col*n_row+row)*pl]);//payoff based matrix, cel based * pl.
            for(int i=1;i<pl;i++)   //first one seperate: no ','
            {
                tabtext+=","+gamematrix[(col*n_row+row)*pl+i];
            }
            game_tab_mod->item(row,col)->setText(tabtext);
            game_tab_mod->item(row,col)->setTextAlignment(Qt::AlignCenter);
            //qDebug()<<"curgame->cel_col_alpha"<<curgame->Cel_col_alpha;
            if(cel_col_alpha.count()>0)
            {
                my_brush.setColor(QColor(249,166,2,int(cel_col_alpha[col*n_row+row]*255)));//cel based matrix
                game_tab_mod->item(row,col)->setBackground(my_brush);//ne color
            }
            QFont textfont; //pareto optimal
            if(pareto_opt_ind.contains(col*n_row+row))textfont.setBold(true);//cel based matrix
            else textfont.setBold(false);
            game_tab_mod->item(row,col)->setFont(textfont);
        }
    }
    if((pl>2 && act>2) || act>4)ui->tab_game->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    else ui->tab_game->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tab_game->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//show the rewards in a table
void Roundrobin_tab::show_tab_matrix(int gamenum,int mode)
{
    qDebug()<<"openning database tab matrix";
    if(true)
    {
        //QString DBname="compdata"+QString::number(gamenum);
        //qDebug()<<"mat_tab"<<DBname;
        QSqlDatabase  db;
        if( !QSqlDatabase::contains("roundrobin_table" )){db= QSqlDatabase::addDatabase ("QSQLITE","roundrobin_table");}//connecto to the database
        else db = QSqlDatabase::database("roundrobin_table");
        db.setDatabaseName(widget->comp_data.Session+widget->comp_data.db_loc);
        db.open();
        if (!db.open()){qDebug("Error occurred opening the database.");qDebug("%s.", qPrintable(db.lastError().text()));}
        QSqlQuery q(db);
//get the general information of this game
        q.prepare( "SELECT * FROM compdata WHERE rowid = "+QString::number(gamenum+1)+"; ");
        q.exec();q.first();

//create game matrix
        QString playorder=q.value(0).toString();
        Players=playorder.split(',',QString::SkipEmptyParts);
        int num_pl_g=q.value(6).toInt(),num_act_g=q.value(7).toInt();
        QString cel_info = q.value(9).toString();
        QStringList gamematrix=cel_info.split(',',QString::SkipEmptyParts);
        cel_info = q.value(10).toString();
        QStringList temp =cel_info.split(',',QString::SkipEmptyParts);
        QVector<double> cel_col_alpha;
        foreach(QString num, temp){cel_col_alpha.append(num.toDouble());}
        cel_info = q.value(11).toString();
        temp =cel_info.split(',',QString::SkipEmptyParts);
        QVector<int>pareto_opt_ind;
        foreach(QString num, temp){pareto_opt_ind.append(num.toInt());}
        show_game_table(num_pl_g, num_act_g,gamematrix,cel_col_alpha,pareto_opt_ind);

        if(num_pl_g>2){qDebug()<<"round robin table to big too show!!";}//only show 2 player games, but cant return as were accessing the DB
        else //easiest alternative: put the rest behind this iff.
        {
            QVector<QString>play0_order,play1_order;
            QVector<double> rew_pl0,rew_pl1;
            QString querry;

            if(modes[mode]=="first_rounds")querry="SELECT pl_id0,avg(rew_pl0),pl_id1,avg(rew_pl1) FROM compdata"+QString::number(gamenum)+" where rec_FL = 'F' group by pl_id0, pl_id1;";
            else if(modes[mode]=="last_rounds")querry="SELECT pl_id0,avg(rew_pl0),pl_id1,avg(rew_pl1) FROM compdata"+QString::number(gamenum)+" where rec_FL = 'L' group by pl_id0, pl_id1;";
            else querry="SELECT pl_id0,avg(rew_pl0),pl_id1,avg(rew_pl1) FROM compdata"+QString::number(gamenum)+" group by pl_id0, pl_id1;";
            //If the last mode("average_game") is chosen it also automatically averages over the first and last recorded iterations.

            q.exec(querry);
            while(q.next())
            {
                //qDebug()<<q.value(0)<<q.value(1)<<q.value(2)<<q.value(3);
                play0_order.append(q.value(0).toString());
                play1_order.append(q.value(2).toString());
                rew_pl0.append(q.value(1).toDouble());
                rew_pl1.append(q.value(3).toDouble());
            }

            //symmetric games
            if(pow(Players.count(),2)!=play0_order.count())
            {
                int mirror=play0_order.count();
                for(int i=0;i<mirror;i++)
                {
                    if(play0_order[i]!=play1_order[i])//not on the main diagonal
                    {
                        play0_order.append(play1_order[i]);
                        play1_order.append(play0_order[i]);
                        rew_pl0.append(rew_pl1[i]);
                        rew_pl1.append(rew_pl0[i]);
                    }
                }
            }

            //show the results
            QStringList players_short=Players;
            for(int i=0;i<Players.length();i++)players_short[i]=Players[i].left(4);//get only first 4 caracters
            QStandardItemModel *matr_tab_mod=new QStandardItemModel(Players.count(),Players.count(),this);
            ui->tab_matrix->setModel(matr_tab_mod);
            matr_tab_mod->setHorizontalHeaderLabels(players_short);
            matr_tab_mod->setVerticalHeaderLabels(players_short);

            for(int i=0;i<rew_pl0.count();i++)
            {
                QString tabtext=QString::number(rew_pl0[i],'f', 2)+","+QString::number(rew_pl1[i],'f', 2);
                QStandardItem *item = new QStandardItem(tabtext);
                int ind_row=Players.indexOf(play0_order[i]);
                int ind_col=Players.indexOf(play1_order[i]);
                matr_tab_mod->setItem(ind_row,ind_col, item);//normal
                matr_tab_mod->item(ind_row,ind_col)->setTextAlignment(Qt::AlignCenter);
            }
            if(Players.count()>5) ui->tab_matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            else ui->tab_matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            ui->tab_matrix->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
    }
    QSqlDatabase::removeDatabase("roundrobin_table");//database usage must be out iof scope: behind a }....
}


//Created the grand table averaged over all games
void Roundrobin_tab::averaged_grandtable()
{
    qDebug()<<"called average grand table";
    //connect to db
    if(true)
    {
        QSqlDatabase  db;
        if( !QSqlDatabase::contains("roundrobin_table" )){db= QSqlDatabase::addDatabase ("QSQLITE","roundrobin_table");}//connecto to the database
        else db = QSqlDatabase::database("roundrobin_table");
        db.setDatabaseName(widget->comp_data.Session+widget->comp_data.db_loc);
        db.open();
        if (!db.open()){qDebug("Error occurred opening the database.");qDebug("%s.", qPrintable(db.lastError().text()));}
        QSqlQuery qgame(db);
        QSqlQuery q(db);

        //get game data and compute max rewards
        qgame.exec("SELECT players,gamenum,matrix FROM compdata ORDER BY gamenum ;");
        QVector<QString> order_rews;
        QVector<double> avr_rew_pl0,avr_rew_pl1;

        int num_games=0;
        bool first_Game=true;
        //q.exec();q.first();
        while(qgame.next())
        {
            QString cel_info = qgame.value(0).toString();
            Players=cel_info.split(',',QString::SkipEmptyParts);
            QString gamenum= qgame.value(1).toString();
            cel_info = qgame.value(2).toString();
            QStringList gamematrix=cel_info.split(',',QString::SkipEmptyParts);
            int pl0_max=INT_MIN,pl1_max=INT_MIN;
            int pl0_min=INT_MAX,pl1_min=INT_MAX;

            for(int i= 0; i< gamematrix.count(); i++)
            {
                int buffer=gamematrix[i].toInt();
                if(i%2==0)//pl0
                {
                    if(buffer>pl0_max)pl0_max=buffer; //max
                    if(buffer<pl0_min)pl0_min=buffer; //min
                }
                else
                {
                    if(buffer>pl1_max)pl1_max=buffer; //pl1
                    if(buffer<pl1_min)pl1_min=buffer; //pl1
                }
            }
            qDebug()<<"pl0 max rew="<<pl0_max<<", pl1 max rew="<<pl1_max;
            qDebug()<<"pl0 min rew="<<pl0_min<<", pl1 min rew="<<pl1_min;
            QString querry="SELECT pl_id0,avg(rew_pl0),pl_id1,avg(rew_pl1) FROM compdata"+gamenum+" group by pl_id0, pl_id1;";
            //qDebug()<<querry;
            q.exec(querry);
            QVector<double> rew_pl0,rew_pl1;
            QStringList list_pl0,list_pl1;
            while(q.next())
            {
                //qDebug()<<q.value(0)<<q.value(1)<<q.value(2)<<q.value(3);
                list_pl0.append(q.value(0).toString());
                list_pl1.append(q.value(2).toString());
                rew_pl0.append((q.value(1).toDouble()-pl0_min)/(pl0_max-pl0_min));//normalized.
                rew_pl1.append((q.value(3).toDouble()-pl1_min)/(pl1_max-pl1_min));
            }

            //unpack symmetric games
            if(pow(Players.count(),2)!=list_pl0.count())//symmetric
            {
                int mirror=list_pl0.count();
                for(int i=0;i<mirror;i++)
                {
                    if(list_pl0[i]!=list_pl1[i])//not on the main diagonal
                    {
                        list_pl0.append(list_pl1[i]);
                        list_pl1.append(list_pl0[i]);
                        rew_pl0.append(rew_pl1[i]);
                        rew_pl1.append(rew_pl0[i]);
                    }
                }
            }
            //add rewards to the average
            if(first_Game==true)
            {
                for(int i=0;i<list_pl0.count();i++)
                {
                    order_rews.append(list_pl0[i]+","+list_pl1[i]);
                    avr_rew_pl0.append(rew_pl0[i]);
                    avr_rew_pl1.append(rew_pl1[i]);
                    //qDebug()<<i<<list_pl0[i]+","+list_pl1[i]<<rew_pl0[i]<<rew_pl0[i];
                }
                first_Game=false;
            }
            else
            {
                for(int i=0;i<list_pl0.count();i++)
                {
                    int ind=order_rews.indexOf(list_pl0[i]+","+list_pl1[i]);
                    //qDebug()<<i<<list_pl0[i]+","+list_pl1[i]<<ind<<rew_pl0[ind]<<rew_pl0[ind];
                    //qDebug()<<ind;
                    avr_rew_pl0[ind]+=rew_pl0[ind];
                    avr_rew_pl1[ind]+=rew_pl1[ind];
                }
            }
            num_games++;
        }
        //show the results
        QStandardItemModel *grand_tab_mod=new QStandardItemModel(Players.count(),Players.count(),this);
        ui->tab_matrix->setModel(grand_tab_mod);
        QStringList players_short;
        for(int i=0;i<Players.length();i++)players_short.append(Players[i].left(4));//get only first 4 caracters
        grand_tab_mod->setHorizontalHeaderLabels(players_short);
        grand_tab_mod->setVerticalHeaderLabels(players_short);
        //fill table
        for(int i=0;i<avr_rew_pl0.count();i++)
        {
            QString tabtext=QString::number(avr_rew_pl0[i]/num_games,'f', 2)+","+QString::number(avr_rew_pl1[i]/num_games,'f', 2);
            QStringList plind=order_rews[i].split(',',QString::SkipEmptyParts);
            QStandardItem *item = new QStandardItem(tabtext);
            int ind_row=Players.indexOf(plind[0]);
            int ind_col=Players.indexOf(plind[1]);
            grand_tab_mod->setItem(ind_row,ind_col, item);//normal
            grand_tab_mod->item(ind_row,ind_col)->setTextAlignment(Qt::AlignCenter);
        }



        //show empty game matrix: dashes
        QStandardItemModel *game_tab_mod=new QStandardItemModel(2,2,this);
        ui->tab_game->setModel(game_tab_mod);

        for(int row = 0; row < 2; row++)/*create the game table*/
        {
           //ui->tab_game->setRowHeight(row,(ui->tab_game->height()/2)-12);
           for(int col = 0; col < 2; col++)/*create the game table*/
           {
               QStandardItem *item = new QStandardItem(QString("-,-"));
               game_tab_mod->setItem(row, col, item);
               ui->tab_game->setColumnWidth(col,(ui->tab_game->width()/2)-10);
           }
        }
        ui->tab_game->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tab_game->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    QSqlDatabase::removeDatabase("roundrobin_table");//database usage must be out iof scope: behind a }....
    if(Players.count()>5) ui->tab_matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    else ui->tab_matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tab_matrix->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}



//create a fake 2 players game, number of actions = number of algorithms that participated in the comparison
//and call gambit with this matrix
void Roundrobin_tab::on_comp_NE_clicked()
{
//    qDebug()<<"computing NE";
    QString num_Act=QString::number(Players.count());//number of players in the round robin table.
    QString filename= widget->comp_data.Session+"/comp_NE_game"+QString::number(1);
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream <<"NFG 1 R \"Generated by GAMUT v1.0.1";
        stream <<"A Game With Uniformly Random Payoffs";
        stream <<"Game Parameter Values:";
        stream <<"Random seed:\t1547317343101";
        stream <<"Cmd Line:\t-g RandomGame -f D:/Onedrive/master/thesis/Qt/text_finder_example/test/Game4 -int_mult 1 -random_params -output GambitOutput -players 2 -actions "+num_Act+" -normalize -min_payoff 0 -max_payoff 100 -int_payoffs";
        stream <<"Players:\t2";
        stream <<"Actions:\t"+num_Act+" "+num_Act;
        stream <<"players:\t2";
        stream <<"actions:\t["+num_Act+"]\" { \"Player1\" \"Player2\" } { "+num_Act+" "+num_Act+" } ";
        stream <<"";
        QString rewards;
        QAbstractItemModel *matrix_tab_mod=ui->tab_matrix->model();
        for(int i=0;i<matrix_tab_mod->columnCount();i++)
        {
            for(int j=0;j<matrix_tab_mod->rowCount();j++)
            {
                rewards+=(matrix_tab_mod->index(j,i).data().toString()+",");
            }
        }
        rewards.replace(QString(","), QString(" "));
//            qDebug()<<"num_Act"<<num_Act;
//            qDebug()<<rewards;
        stream <<rewards;//"69 0 76 38 60 100 38 88 ";//for 2 actions
        file.close();
    }
    QStringList NEs=widget->CallGambit(filename,2,Players.count());//compute the NE.
    //QString nes;
    ui->listW_NEs->clear();
    for(QString one_NE : NEs)//for all NE
    {
        QString result;
        QString NE_short;
        QStringList NE_profile=one_NE.split(',',QString::SkipEmptyParts);
        result="pl1:";
        for(int i=0;i<NE_profile.count();i++)//for each step
        {
            if(i%Players.count()==0 && i>0)result+="pl2:";
            if(NE_profile[i] != "0"){result+=(Players[(i%Players.count())]+",").leftJustified(13,' ',true);}
            NE_short+=NE_profile[i].left(4)+",";
        }
        QListWidgetItem *NE = new QListWidgetItem;
        QListWidgetItem *NE_ind = new QListWidgetItem;
        NE->setSizeHint(QSize (ui->listW_NE->width()-18,14));
        //NE_ind->setSizeHint(QSize (ui->listW_NEs->width()-18,14));
        NE->setText(result);
        ui->listW_NE->addItem(NE);
        one_NE.replace((Players.count()*2-1),1," "); //only 2 players now
        NE_ind->setText(NE_short);
        ui->listW_NEs->addItem(NE_ind);
    }
    ui->listW_NE->setFont(create_spfont());
    ui->listW_NEs->setFont(create_spfont());
    //update the matrix with cel colors
    QVector<QVector<QVector<double>>>NE_for_celcol=widget->conv_NE(NEs,2,Players.count());//get the double NEs needed to color the cells..
    compute_NE_col(NE_for_celcol);
}

//compute the NE colors, this is done for all cells
//if their are multiple NE or a mixed NE the color intensity varies based on how many NE contain the cel.
void Roundrobin_tab::compute_NE_col(QVector<QVector<QVector<double>>>NE_for_celcol)
{

    qDebug()<<"NE_for_celcol"<<NE_for_celcol;
    int pl=2;
    Game tempgame; //need its functions
    tempgame.Actions=Players.count();
    QVector <double> cel_col_alpha;
    QVector<int> binplayer(2,0);
    QVector<int> stepsize={1,Players.count()};
    bool done=false;
    QVector<int>NE_ind;//={0,1};
    for(int i=0;i<pl;i++)NE_ind.append(i);//normal NE indeces
    while(done==false)/*compute cel colors for NE*/
    {
        int cel=tempgame.Calculate_bincel(binplayer,stepsize);
         cel_col_alpha.append(0);
         for(int i=0;i<NE_for_celcol.count();i++)
         {
            qDebug()<<"NE"<<NE_for_celcol[i];
            double cel_color=1;
            for(int j=0;j<pl;j++)
            {
                qDebug()<<"player"<<binplayer[j]<<NE_for_celcol[i][NE_ind[j]][binplayer[j]];
                 cel_color=cel_color*NE_for_celcol[i][NE_ind[j]][binplayer[j]];
            }
             qDebug()<<"c_col"<<cel<< cel_color;

             if(qFabs(cel_col_alpha[cel] - 1.0) < std::numeric_limits<double>::epsilon());//let it stay at 1:pure NE
             else  if (qFabs(cel_color - 1.0) < std::numeric_limits<double>::epsilon())cel_col_alpha[cel]=(cel_color);//spot pure Ne after mixed
             else if(cel_col_alpha[cel]<std::numeric_limits<double>::epsilon())cel_col_alpha[cel]=(cel_color);
             else if(cel_color> 0) cel_col_alpha[cel]*=(cel_color);
             qDebug()<<"matrix"<<cel_col_alpha;

         }
         binplayer=tempgame.update_binpl(binplayer, &done);
   }
//   qDebug()<<"calculated cel_col_alpha"<<cel_col_alpha;
    //update the matrix with colors
   QStandardItemModel *sModel = qobject_cast<QStandardItemModel *>(ui->tab_matrix->model());
   QBrush my_brush;
   my_brush.setStyle(Qt::SolidPattern);
   for(int col_i=0;col_i<Players.count();col_i++)
   {
       for(int row_i=0;row_i<Players.count();row_i++)
       {
            my_brush.setColor(QColor(249,166,2,int(cel_col_alpha[col_i*Players.count()+row_i]*255)));
            sModel->item(row_i,col_i)->setBackground(my_brush);//NE color
       }
   }
}


//read the matrix table and convert the values to intergers in the format used by the pareto optimal function: 0,0 1,0 rows first
QVector<QVector <int>> Roundrobin_tab::conv_roundr_table()
{
    QVector<QVector <int>> matrix_i;
    QAbstractItemModel *matrix_tab_mod=ui->tab_matrix->model();
    for(int i=0;i<matrix_tab_mod->columnCount();i++)
    {
        for(int j=0;j<matrix_tab_mod->rowCount();j++)
        {
            QVector <int> payoff_cell;
            QString cel=matrix_tab_mod->index(j,i).data().toString();
            QStringList payoff=cel.split(',',QString::SkipEmptyParts);
            for(int k=0;k<payoff.count();k++)payoff_cell.append(qRound(payoff[k].toDouble()*100));//convert doubles to ints, using only the first two decimals as in the table
            matrix_i.append(payoff_cell);
        }
    }
    return matrix_i;
}

//read the matrix table and convert the values to double in the format used by the pareto optimal function: 0,0 1,0 rows first
QVector<QVector <double>> Roundrobin_tab::conv_roundr_table_d()
{
    QVector<QVector <double>> matrix_i;
    QAbstractItemModel *matrix_tab_mod=ui->tab_matrix->model();
    for(int i=0;i<matrix_tab_mod->columnCount();i++)
    {
        for(int j=0;j<matrix_tab_mod->rowCount();j++)
        {
            QVector <double> payoff_cell;
            QString cel=matrix_tab_mod->index(j,i).data().toString();
            QStringList payoff=cel.split(',',QString::SkipEmptyParts);
            for(int k=0;k<payoff.count();k++)payoff_cell.append(payoff[k].toDouble());
            matrix_i.append(payoff_cell);
        }
    }
    return matrix_i;
}

//compute and show the pareto optimal actions of this game
void Roundrobin_tab::on_comp_paropt_clicked()
{
    int action_c=Players.count();//its always a 2player, x action game!!!
    QVector<QVector <int>> matrix=conv_roundr_table();
    QVector<QVector <int>> pareto_opt;
    QVector <int> pareto_opt_ind;
    qDebug()<<"matr"<<matrix;
    widget->Find_pareto_opt_actions(matrix,2,pareto_opt,pareto_opt_ind);
    qDebug()<<"pareto_opt"<<pareto_opt;
    qDebug()<<"pareto_opt_ind"<<pareto_opt_ind;

    QFont textfont; //pareto optimal
    textfont.setBold(true);//cel based matrix
    QStandardItemModel *sModel = qobject_cast<QStandardItemModel *>(ui->tab_matrix->model());
    for(int i=0;i<pareto_opt_ind.count();i++)
    {
        QString result=("pl1:"+Players[pareto_opt_ind[i]%action_c]).leftJustified(15,' ',true);//leftJustified: add padding
        result+=" pl2:"+Players[pareto_opt_ind[i]/action_c];

        QListWidgetItem *parop = new QListWidgetItem;
        parop->setSizeHint(QSize (ui->listW_paropt->width()-15,15));//size line
        parop->setText(result);
        ui->listW_paropt->addItem(parop);

        QModelIndex nIndex = sModel->index(pareto_opt_ind[i]%action_c,pareto_opt_ind[i]/action_c);//row, column
        QStandardItem *item = sModel->itemFromIndex(nIndex);
        item->setFont(textfont);
        sModel->setItem(pareto_opt_ind[i]%action_c,pareto_opt_ind[i]/action_c,item);
    }
    ui->listW_paropt->setFont(create_spfont());
}

//create a font with fixed characted width for the lists
QFont Roundrobin_tab::create_spfont()
{
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(8);
    return font;
}

//send widget a signal with matrix table model & te players so it can create a reply dyn tab
void Roundrobin_tab::on_btn_repl_dyn_clicked()
{
    widget->add_tab_repl_dyn(conv_roundr_table_d(),Players);
    //not allowed to pass pointer to Qabstract table view around so we have to send the matrix instead.
}
