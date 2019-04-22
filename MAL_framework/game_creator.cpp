#include "game_creator.h"
#include "ui_game_creator.h"

Game_creator::Game_creator(Widget*widgetref,int next_game_number, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game_creator)
{
    ui->setupUi(this);
    widget=widgetref;

    qDebug()<<"dir:"<<widget->comp_data.Session+"/";
    QStringList Generators = {"ArmsRace","BattleOfTheSexes","BertrandOligopoly",
                              "BidirectionalLEG","Chicken","CollaborationGame",
                             "CoordinationGame","CournotDuopoly","CovariantGame","DispersionGame",
                             "GrabTheDollar","HawkAndDove","LocationGame",
                             "MajorityVoting","MatchingPennies","MinimumEffortGame","NPlayerChicken",
                             "NPlayerPrisonersDilemma","PolymatrixGame","PrisonersDilemma","RandomGame",
                             "RandomCompoundGame","RandomLEG","RandomGraphicalGame","RandomZeroSum","RockPaperScissors",
                              "ShapleysGame","TravelersDilemma","TwoByTwoGame","UniformLEG",
                             "WarOfAttrition"};
    ui->Generator_CMB->addItems(Generators);
    ui->Generator_CMB->setCurrentText("RandomGame");
    ui->Lbl_gnum->setText(QString::number(next_game_number));
    ui->lbl_NE->setStyleSheet("QLabel { background-color: rgba(249,166,2,255); }");
    ui->lbl_NE->setAlignment(Qt::AlignCenter);

    /*edit game: load params*/
    if(next_game_number<widget->G_data.count())//editing a game
    {
        Game curgame=widget->G_data[next_game_number];
        ui->Generator_CMB->setCurrentText(curgame.Generator);
        ui->LE_pl->setText(QString::number(curgame.Players));
        ui->LE_Act->setText(QString::number(curgame.Actions));
        ui->cb_norm->setChecked(curgame.Normalize);
        ui->Lbl_gnum->setText(QString::number(next_game_number));//curgame.Filename.midRef(4).toString()
        ui->LE_minpay->setText(QString::number(curgame.Min_payoff));
        ui->LE_maxpay->setText(QString::number(curgame.Max_payoff));
        //ui->cb_intval->setChecked(curgame.Int_payoffs);
        ui->LE_nr_g->setEnabled(false);//only make one game
        fill_gametable(&widget->G_data[next_game_number]);
    }
    else{fill_gametable(nullptr);}
}

//compute the NE colors, this is done for all cells
//if their are multiple NE or a mixed NE the color intensity varies based on how many NE contain the cel.
void Game_creator::compute_NE_col(Game *curgame)
{
    int pl=curgame->Players;
    QVector <double> cel_col_alpha;
    QVector<int> Binplayer=curgame->Binplayer;
    bool done=false;
    QVector<int>NE_ind;//={0,1};
    for(int i=0;i<pl;i++)NE_ind.append(i);//normal NE indeces
    for(int i=0;i<curgame->Matrix_i.count();i++)cel_col_alpha.append(0);

    while(done==false)/*compute cel colors for NE*/
    {
        int cel=curgame->Calculate_bincel(Binplayer,curgame->Stepsize);
        //qDebug()<<"cel"<<cel;

         for(int i=0;i<curgame->NE_d.count();i++)
         {
            double cel_color=1;
            for(int j=0;j<pl;j++)//for all players
            {
                 cel_color=cel_color*curgame->NE_d[i][NE_ind[j]][Binplayer[j]];
            }
            if(qFabs(cel_col_alpha[cel] - 1.0) < std::numeric_limits<double>::epsilon());//let it stay at 1:pure NE
            else  if (qFabs(cel_color - 1.0) < std::numeric_limits<double>::epsilon())cel_col_alpha[cel]=(cel_color);//spot pure Ne after mixed
            else if(cel_col_alpha[cel]<std::numeric_limits<double>::epsilon())cel_col_alpha[cel]=(cel_color);
            else if(cel_color> 0) cel_col_alpha[cel]*=(cel_color);
         }
         Binplayer=curgame->update_binpl(Binplayer, &done);
   }
//   qDebug()<<"calculated cel_col_alpha"<<cel_col_alpha;
   curgame->Cel_col_alpha=cel_col_alpha;
}


//fill the game_table with the rewards and colors
//only used for showing it on the screen.
void Game_creator::fill_gametable(Game *curgame)
{  
   //no games created so far: show empty 2x2 table
   if(curgame==nullptr)
   {
       //qDebug()<<"No game created so far";
       Tabmodel = new QStandardItemModel(2,2,this);//kolomen=act(only pl 2)
       ui->tableView->setModel(Tabmodel);
       for(int row = 0; row < 2; row++)/*create the game table*/
       {
           //ui->tableView->setRowHeight(row,(ui->tableView->height()/2)-12);
           for(int col = 0; col < 2; col++)/*create the game table*/
           {
               QStandardItem *item = new QStandardItem(QString("-,-"));
               Tabmodel->setItem(row, col, item);
               ui->tableView->setColumnWidth(col,(ui->tableView->width()/2)-10);
           }
       }
       ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
       ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
       return;
   }
   //show only limited sized games, if the game is bigger we abort.
   else if(curgame->Actions>4 ||curgame->Players>4){ui->lbl_stat->setText("Too big to show");return;}
   //compute the NE colors if its not yet done
   else if(curgame->Cel_col_alpha.count()==0){qDebug()<<"cel_col_alpha.count=0";compute_NE_col(curgame);}

    //create the game table
    int pl=curgame->Players;
    int act =curgame->Actions;
    int n_row=curgame->Stepsize[1];
    int n_col=act;

    Tabmodel = new QStandardItemModel(n_row,n_col,this);
    ui->tableView->setModel(Tabmodel);
    QBrush my_brush;
    my_brush.setStyle(Qt::SolidPattern);

    qDebug()<<"marker";
    qDebug()<<"game matrix"<<curgame->Matrix;
    qDebug()<<"game matrix"<<curgame->Matrix_i;
    qDebug()<<"NE's"<<curgame->NE;
    qDebug()<<"cel_col_alpha"<<curgame->Cel_col_alpha;

    for(int col = 0; col < n_col; col++)//create the game table
    {
        //ui->tableView->setColumnWidth(col,(ui->tableView->width()-20)/n_col);
        for(int row = 0; row < n_row; row++)
        {
            //ui->tableView->setRowHeight(row,(ui->tableView->height()-24)/n_row);
            my_brush.setColor(QColor(249,166,2,int(curgame->Cel_col_alpha[col*n_row+row]*255)));//cel based matrix
            QStandardItem *item = new QStandardItem(QString("-,-"));
            Tabmodel->setItem(row, col, item);
            QString tabtext=(curgame->Matrix[(col*n_row+row)*pl]);//payoff based matrix, cel based * pl.
            for(int i=1;i<pl;i++)   //first players reward: no ','
            {
                tabtext+=","+curgame->Matrix[(col*n_row+row)*pl+i];
            }
            Tabmodel->item(row,col)->setText(tabtext);
            Tabmodel->item(row,col)->setTextAlignment(Qt::AlignCenter);
            Tabmodel->item(row,col)->setBackground(my_brush);//NE color

            QFont textfont; //pareto optimal
            if(curgame->Pareto_opt_ind.contains(col*n_row+row))textfont.setBold(true);//cel based matrix
            else textfont.setBold(false);
            Tabmodel->item(row,col)->setFont(textfont);
        }
    }
//    ui->tableView->resizeRowsToContents();
//    ui->tableView->resizeColumnsToContents();
    if((curgame->Players>2 && curgame->Actions>2) || curgame->Actions>4)ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    else ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

}

//default destructor
Game_creator::~Game_creator()
{
    delete ui;
}

//we overload the default class destructor to send the list of created games back
//when the screen is closed.
void Game_creator::closeEvent(QCloseEvent *event)
{
      emit Game_createClosed();
//      event->accept();       
      QWidget::closeEvent(event);
}


//handler for the close button: closes the screen.
void Game_creator::on_Btn_close_clicked()
{
    this->close();
}

//create a new game button, pressed after all the settings are set.
//this loads the into an instance of the game class,
//call gamut, gambit and computer the pareto optimal cells
void Game_creator::on_Btn_create_clicked()
{
    /*check values enterd in the ui*/
    if(!(ui->LE_nr_g->text().toInt()>0))
    {
       ui->lbl_stat->setText("unvallid number of games");
       return;
    }
    else if (!(ui->LE_pl->text().toInt()>0) )
    {
        ui->lbl_stat->setText("unvallid number of players");
        return;
    }
    else if (!(ui->LE_Act->text().toInt()>0) )
    {
        ui->lbl_stat->setText("unvallid number of actions");
        return;
    }
    else if ((ui->LE_minpay->text().toInt()>ui->LE_maxpay->text().toInt()) )
    {
        ui->lbl_stat->setText("min payoff higher than max payoff");
        return;
    }
    int num_games=ui->LE_nr_g->text().toInt();
    int i=0,base_game_nmb=ui->Lbl_gnum->text().toInt();
    int num_play=ui->LE_pl->text().toInt(),num_act=ui->LE_Act->text().toInt();
    int min_pay=ui->LE_minpay->text().toInt(),max_pay=ui->LE_maxpay->text().toInt();

    //create games
    while(i<num_games)
    {
        Game  new_Game;
        Filename="Game"+QString::number(base_game_nmb+i);
        new_Game.Actions=num_act;
        new_Game.Players=num_play;
        new_Game.Normalize=ui->cb_norm->isChecked();
        new_Game.Min_payoff=min_pay;
        new_Game.Max_payoff=max_pay;
        //new_Game.Int_payoffs=ui->cb_intval->isChecked();
        new_Game.Generator=ui->Generator_CMB->currentText();
        new_Game.Init_binplayer();  //calculate some values.
        new_Game.Calculate_stepsize();

        Creating_Game= &new_Game;//for efficiency we use pointers where we can

        ui->lbl_stat->setText(QStringLiteral("Creating game %1 of %1").arg(i,num_games));

        Gamut();

        readTxt(widget->comp_data.Session+"/"+Filename);
        Creating_Game->NE=widget->CallGambit(widget->comp_data.Session+"/"+Filename,Creating_Game->Players,Creating_Game->Actions);
        Creating_Game->NE_d=widget->conv_NE(Creating_Game->NE,Creating_Game->Players,Creating_Game->Actions);
        //conv_NE(Creating_Game->NE);

        widget->Find_pareto_opt_actions(Creating_Game->Matrix_i,Creating_Game->Players,Creating_Game->Pareto_opt,Creating_Game->Pareto_opt_ind );


        //store game in widget->g_data
        if((base_game_nmb+i)<widget->G_data.count())//edited game
        {
            widget->G_data[base_game_nmb]=new_Game;
        }
        else //new game
        {
            widget->G_data.append(new_Game);
            ui->Lbl_gnum->setText(QString::number(base_game_nmb+num_games));
        }
        i++;
    }
    qDebug()<<"game_cr_done";
//UI input
    //LE_pl
    //LE_Act
    //LE_minpay  //min payoff
    //LE_maxpay  //max payoff
   // LE_nr_g //number of games

    //Generator_CMB
    //cb_all_gen //all Generators
    //cb_norm  //normalise
    //cb_intval //intvalues

    //lbl_stat  //status label

    qDebug()<<"datset"<<widget->G_data.count();
    qDebug()<<"index"<<base_game_nmb+(num_games-1);
    fill_gametable(&widget->G_data[base_game_nmb+(num_games-1)]);//show last created game
    qDebug()<<"fill_Gametab_done";
    //not using Creating_Game because that points to a different instance! now.
}

//call to gamut, to create a game
void Game_creator::Gamut()
{
//https://www.qtcentre.org/threads/57116-SOLVED-QProcess-Java-Application-Not-Working
//https://stackoverflow.com/questions/33865731/how-to-run-a-windows-cmd-command-using-qt

    Game cur_game;
    //if there already is a file with this name remove it
    if(fileExists(widget->comp_data.Session+"/"+Filename)==true)
    {
        QFile file (widget->comp_data.Session+"/"+Filename);
        file.remove();
    }

    QProcess *Gamut_pros= new QProcess(this);
    Gamut_pros->setWorkingDirectory(widget->Gamut_loc);
    Gamut_pros->setProgram("java");
    QStringList arg;
    arg << "-jar";
    arg << "gamut.jar";
    arg <<"-g"<<Creating_Game->Generator<<"-f"<<widget->comp_data.Session+"/"+Filename<<"-int_mult"<<"1"<<"-random_params"<<"-output"<<"GambitOutput";
    if(Creating_Game->Generator=="ArmsRace"||Creating_Game->Generator=="CournotDuopoly"||Creating_Game->Generator=="GrabTheDollar"||Creating_Game->Generator=="LocationGame"||Creating_Game->Generator=="RandomZeroSum"||Creating_Game->Generator=="WarOfAttrition")arg<<"-actions"<<QString::number(Creating_Game->Actions);
    else if(Creating_Game->Generator=="CollaborationGame"||Creating_Game->Generator=="CongestionGame"||Creating_Game->Generator=="CoordinationGame"||Creating_Game->Generator=="NPlayerChicken"||Creating_Game->Generator=="NPlayerPrisonersDilemma"||Creating_Game->Generator=="RandomCompoundGame")arg<<"-players"<<QString::number(Creating_Game->Players);
    else if(Creating_Game->Generator=="BertrandOligopoly"||Creating_Game->Generator=="BidirectionalLEG"||Creating_Game->Generator=="CovariantGame"||Creating_Game->Generator=="DispersionGame"||Creating_Game->Generator=="GuessTwoThirdsAve"||Creating_Game->Generator=="MajorityVoting"||Creating_Game->Generator=="MinimumEffortGame"||Creating_Game->Generator=="PolymatrixGame"||Creating_Game->Generator=="RandomGame"||Creating_Game->Generator=="RandomLEG"||Creating_Game->Generator=="RandomGraphicalGame"||Creating_Game->Generator=="TravelersDilemma"||Creating_Game->Generator=="UniformLEG")arg<<"-players"<<QString::number(Creating_Game->Players)<<"-actions"<<QString::number(Creating_Game->Actions);

//not all games accept all paramaters:
//    <<"-players"<<QString::number(Creating_Game->Players)<<"-actions"<<QString::number(Creating_Game->Actions);
//    "BattleOfTheSexes","Chicken","GreedyGame","HawkAndDove","MatchingPennies","PrisonersDilemma","TwoByTwoGame" 2x2: no actions nor payoff
//    "RockPaperScissors", "ShapleysGame"   only 2 player, 3 actions
//    "ArmsRace" "CournotDuopoly","GrabTheDollar","LocationGame","SimpleInspectionGame"             only 2 player

//GuessTwoThirdsAve  "CongestionGame" "GreedyGame" "SimpleInspectionGame"   //not playable.. as in require tweeking of more paramaters...

    if(Creating_Game->Normalize)arg <<"-normalize"<<"-min_payoff"<<QString::number(Creating_Game->Min_payoff)<<"-max_payoff"<<QString::number(Creating_Game->Max_payoff);
    //if(Creating_Game->Int_payoffs)
    arg <<"-int_payoffs";
    //Gamut_pros->setArguments(arg);
    //Gamut_pros->startDetached();//has to runn detached, so cannot see error signals or status, just wait for file to appear
    Gamut_pros->start(widget->Java_loc, arg);
    Gamut_pros->waitForFinished();
    Gamut_pros->close();

}

//Gamut helper, check if game file is created,
bool Game_creator::fileExists(QString path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

//Gamut helper,read game text file to extract the game matrix and integer variant of the matrix
void Game_creator::readTxt(QString path)
{
    QFile inputFile(path);
    inputFile.open(QIODevice::ReadOnly);
    if (!inputFile.isOpen()){qDebug() <<"Error: Gamut file open";}//return;

    QTextStream stream(&inputFile);
    QString line = stream.readLine();
    //qDebug() <<"Fline"<< line;
    while (!stream.atEnd())//!line.isNull()
    {
        line = stream.readLine();
        //qDebug() <<"line"<< line;
        if(line.size()==0)//line after the empty line contains the rewards
        {
            line = stream.readLine();//next line is payoffs
//            qDebug() <<"last line:"<< line;
            QStringList cells = line.split(' ',QString::SkipEmptyParts);

            //for >2 player games create matrix_i function
            if(Creating_Game->Players>2)convert_gamematrix(cells);
            else //for 2 player games create matrix_i here
            {
                Creating_Game->Matrix=cells;
                QVector<QVector <int>> matrix_i;
                QVector<int>pl_ind;
                for(int i=0;i<Creating_Game->Players;i++)pl_ind.append(i);
                //pl_ind.insert(1,pl_ind.takeFirst());//was used to switch player 2 become 1: col's rewards first

                for(int i=0;i<cells.count()/Creating_Game->Players;i++)
                {
                    QVector<int> row;
                    for (int j : pl_ind)row.append(cells[(i*Creating_Game->Players)+ j].toInt());
                    matrix_i.append(row);
                }
                Creating_Game->Matrix_i=matrix_i;
//                qDebug() <<"matrix_i"<<matrix_i;
            }
        }
    }
    inputFile.close();
}

//if number players >2 the gambit output is not ordered line 1,2,3,4
//but act profile: 0,0,0  1,0,0  2,0,0 etc. so we convert it back.
void Game_creator::convert_gamematrix(QStringList curmat)
{
    int pl=Creating_Game->Players;
    int act=Creating_Game->Actions;

    QStringList con_mat=curmat;//copy matrix
    QVector<QVector <int>> matrix_i;

    QVector<int>conv_steps={act};//conversion opperator
    for(int i=1;i<(pl-1);i++)conv_steps.append(conv_steps[i-1]*act);
    conv_steps.append(1);

    int matr_size=act;//create the new matrix
    for(int i=1;i<pl;i++)matr_size*=act;
    for(int i=0;i<matr_size;i++)matrix_i.append(conv_steps);

    QVector<int>pl_ind;
    for(int i=0;i<pl;i++)pl_ind.append(i);

    QVector<int> Binplayer=Creating_Game->Binplayer;
    bool done=false;
    int counter=0;//counter for the old matrix


    while(!done)//for all binary combinations
    {
        int cel=Creating_Game->Calculate_bincel(Binplayer,conv_steps);
        QVector<int> row;
        for (int i : pl_ind)//for all players
        {
            row.append(curmat[counter+i].toInt());
            con_mat[cel*pl+i]=curmat[counter+i];
        }
        //qDebug()<<"row"<<row;
        matrix_i[cel]=row;
        counter+=pl;
        Binplayer=Creating_Game->update_binpl(Binplayer,&done);
    }
    Creating_Game->Matrix=con_mat;
    Creating_Game->Matrix_i=matrix_i;
//    qDebug()<<"matrix_i"<<matrix_i;
}

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
/*Basic UI stuff*/

//Gamut only accepts min&max payoof when normalise is turned on
void Game_creator::on_cb_norm_stateChanged()
{
    if(ui->cb_norm->isChecked())//true
    {
        ui->LE_minpay->setEnabled(true);
        ui->LE_maxpay->setEnabled(true);
    }
    else
    {
        ui->LE_minpay->setEnabled(false);
        ui->LE_maxpay->setEnabled(false);
    }
}

//not all gamut Generator accept a player & action paramater
void Game_creator::on_Generator_CMB_currentTextChanged(const QString &arg1)
{
    //only 2 player
    if(arg1=="SimpleInspectionGame"||arg1=="ArmsRace"||arg1=="CournotDuopoly"||arg1=="GrabTheDollar"||arg1=="LocationGame"||arg1=="RandomZeroSum"||arg1=="WarOfAttrition")
    {
        ui->LE_pl->setText("2");ui->LE_pl->setEnabled(false);
        ui->LE_Act->setEnabled(true);
    }
    //2 player x 3 actions games
    else if (arg1=="RockPaperScissors"||arg1=="ShapleysGame")
    {
        ui->LE_pl->setText("2");ui->LE_pl->setEnabled(false);
        ui->LE_Act->setText("3");ui->LE_Act->setEnabled(false);
    }
    //2x2 games
    else if (arg1=="BattleOfTheSexes"||arg1=="Chicken"||arg1=="GreedyGame"||arg1=="HawkAndDove"||arg1=="MatchingPennies"||arg1=="PrisonersDilemma"||arg1=="TwoByTwoGame")
    {
        ui->LE_pl->setText("2");ui->LE_pl->setEnabled(false);
        ui->LE_Act->setText("2");ui->LE_Act->setEnabled(false);
    }
    //only players
    else if (arg1=="CollaborationGame"||arg1=="CongestionGame"||arg1=="CoordinationGame"||arg1=="NPlayerChicken"||arg1=="NPlayerPrisonersDilemma"||arg1=="RandomCompoundGame"||arg1=="TwoByTwoGame")
    {
        ui->LE_pl->setEnabled(true);
        ui->LE_Act->setText("2");ui->LE_Act->setEnabled(false);
    }
    else //both players and actions
    {
        ui->LE_pl->setEnabled(true);
        ui->LE_Act->setEnabled(true);
    }
}

