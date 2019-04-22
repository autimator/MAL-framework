#include "replicator.h"
#include "ui_replicator.h"

//add printsupport to .pro file to make qcustumplot work.


Replicator::Replicator(Widget *widgetref,QVector<QVector <double>> round_rob_matr, QStringList play,QWidget *parent) ://constructor
    QWidget(parent),ui(new Ui::Replicator)
{
    ui->setupUi(this);
    widget=widgetref;
    Players=play;

    Birthrate=static_cast<double>(ui->slid_birthr->value())/100;
    ui->lbl_birthr->setText(QString::number(static_cast<double>(ui->slid_birthr->value())/100));
/*matrix table init*/
    Round_rob_matrix=round_rob_matr;//keep copy of the original results
    QStandardItemModel *matr_tab_mod=new QStandardItemModel(Players.count(),Players.count(),this);
    ui->tab_matrix->setModel(matr_tab_mod);
    QStringList players_short=Players;
    for(int i=0;i<Players.length();i++)players_short[i]=Players[i].left(4);//get only first 4 caracters
    matr_tab_mod->setHorizontalHeaderLabels(players_short);
    matr_tab_mod->setVerticalHeaderLabels(players_short);
    update_tab_matrix();
    ui->tab_matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tab_matrix->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

/*starting proportions table init*/
     ui->tab_props->setRowCount(Players.count());
     ui->tab_props->setColumnCount(3);
    for(int i=0;i<Players.count();i++)
    {
        ui->tab_props->setItem(i, 0, new QTableWidgetItem(Players[i]));
        ui->tab_props->setItem(i, 1, new QTableWidgetItem("0.0"));
        QSlider* prop = new QSlider(Qt::Horizontal);
        connect(prop,&QSlider::valueChanged,[this, i](){update_siderl(i);});
        prop->setMinimum(0);
        prop->setMaximum(100);
        //prop->setFixedWidth(80);
//       QModelIndex index= ui->tab_props->index(i,2,QModelIndex());
        ui->tab_props->setCellWidget(i,2, prop);
        ui->tab_props->horizontalHeader()->hide();
        ui->tab_props->verticalHeader()->hide();
        ui->tab_props->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tab_props->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

Replicator::~Replicator()//default destructor
{
    delete ui;
}

//update the matrix that contains the fitnesvalues
void Replicator::update_tab_matrix()
{
    if(Tick_counter>0){Repl_running=false;ui->pb_cont->setEnabled(false);} //if replicator is running, stop it! en diable continue button
    QAbstractItemModel *matrix_tab_mod=ui->tab_matrix->model();
    QVariant *v= new QVariant(QString("Some more text"));
    Scoretable= QVector<QVector<double>>{};//make double gamescore list for easy access
    QVector<double>score_row(Players.count(),0.0);
    double rew;
    for(int i=0;i<Players.count();i++)//lets loop through the table
    {        
        for(int j=0;j<Players.count();j++)
        {            
            if(ui->rb_actuator->isChecked()==true)rew=Round_rob_matrix[j*Players.count()+i][1];
            else //average
            {
                rew=(Round_rob_matrix[j*Players.count()+i][1]+Round_rob_matrix[i*Players.count()+j][0])/2;

            }
            v = new QVariant(QString::number(rew));
            score_row[j]=rew;
            QModelIndex mi = matrix_tab_mod->index(i,j);
            matrix_tab_mod->setData(mi,*v);
        }
        Scoretable.append(score_row);
    }
    qDebug()<<Scoretable;
}


//a proportions slider was moved, update the value
void Replicator::update_siderl(int row)//int row,
{
//    QAbstractItemModel *prop_tab_mod=ui->tab_matrix->model();
 //  ui->tab_props->it
    QSlider *slid = qobject_cast<QSlider*>((ui->tab_props->cellWidget(row, 2)));//get slider
  //  ui->tab_props->setIndexWidget()
    ui->tab_props->item(row,1)->setData(Qt::DisplayRole,QString::number(slid->value()));//show slider value
}


//initilase a replicator dynamic
void Replicator::on_pb_start_clicked()
{
    qDebug()<<"t"<<Scoretable;
    Repl_running=false;//stop replicator if it is running
    ui->pb_cont->setEnabled(false);//block runn while setting up next round.
    ui->pb_cont->setText("pause");
    if(ui->tab_props->rowCount()>10){qDebug()<<"not enough colors defined";return;}//one more than items, color 0 also counts...
    int total_prop=0;
    for(int i=0;i<ui->tab_props->rowCount();i++)total_prop+=ui->tab_props->item(i,1)->text().toInt();//get total proportion

    if(total_prop==0){qDebug()<<"no proportions selected";return;}
    double devider=static_cast<double>(total_prop)/100;
    Proportions=QVector<double> {};
    for(int i=0;i<ui->tab_props->rowCount();i++)//initialise the proportions
    {
        double propcel=qRound(ui->tab_props->item(i,1)->text().toDouble()/devider);
        Proportions.append(propcel/100);//proportions are between 0-1
        ui->tab_props->item(i,1)->setData(Qt::DisplayRole,QString::number(propcel));
        QSlider *slid = qobject_cast<QSlider*>((ui->tab_props->cellWidget(i, 2)));
        slid->setValue(static_cast<int>(propcel));
    }
    qDebug()<<"proportions"<<Proportions;

    //birthrate is already set.
    Tick_counter=0;
    ui->graph->clearGraphs();//remove all excisting graphs if there are anny
    ui->graph->xAxis->setLabel("Ticks");
    ui->graph->yAxis->setLabel("Proportions");
    ui->graph->yAxis->setRange(0, 1);
    ui->graph->setInteractions(QCP::iRangeDrag);
    ui->graph->axisRects().at(0)->setRangeDrag(Qt::Horizontal);
    ui->graph->setInteractions(QCP::iRangeZoom);

    ui->graph->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->graph->legend->setFont(legendFont);
    ui->graph->legend->setBrush(QBrush(QColor(255,255,255,230)));
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
//    ui->graph->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

/*
    //test values from elimgood_2_mal practical.
    Proportions={0.33,0.33,0.34};
    Players= QStringList{"one","two","three"};
    Scoretable={{297.5,295.7,117.82},{298.62,297.02,168.28},{415.56,268.31,174.54}};
    Birthrate=0.05;
*/

    for(int i=0;i<Proportions.count();i++)//for each player
    {
        ui->graph->addGraph();
        ui->graph->graph(i)->setName(Players[i]);
        if(i==0)ui->graph->graph(i)->setPen(QPen(Qt::blue));//QPen(QColor(40, 110, 255)));//blue
        else if(i==1)ui->graph->graph(i)->setPen(QPen(Qt::red));
        else if(i==2)ui->graph->graph(i)->setPen(QPen(Qt::black));
        else if(i==3)ui->graph->graph(i)->setPen(QPen(Qt::green));
        else if(i==4)ui->graph->graph(i)->setPen(QPen(Qt::magenta));
        else if(i==5)ui->graph->graph(i)->setPen(QPen(Qt::yellow));
        else if(i==6)ui->graph->graph(i)->setPen(QPen(Qt::darkRed));
        else if(i==7)ui->graph->graph(i)->setPen(QPen(Qt::cyan));
        else if(i==8)ui->graph->graph(i)->setPen(QPen(Qt::darkBlue));
        else if(i==9)ui->graph->graph(i)->setPen(QPen(Qt::gray));
    }
    Repl_running=true;
    ui->pb_cont->setEnabled(true);//enable pause button
    repl_step();
}


//step of the replicator dynamic
void Replicator::repl_step()
{
    int window_size=1000;
    int upd_step=0;
    while(Repl_running==true)
    {
        //qDebug()<<tick_counter<<","<<proportions;
        update_proportions();
        if(upd_step>=50)//update plot every 50 rounds, too reduce lagg
        {
            ui->graph->xAxis->setRange(Tick_counter, Tick_counter, Qt::AlignRight);//increate window in length.
            ui->graph->replot();
            upd_step=0;
        }
        upd_step++;
        Tick_counter++;
        QApplication::processEvents(QEventLoop::AllEvents);//too keep the UI responcive..
    }
    ui->graph->xAxis->setRange(Tick_counter, window_size, Qt::AlignRight);
    ui->graph_scrlbar->setRange(window_size/2,Tick_counter);//as 50 if graph size now. legend is shown on right, so little extra room
    ui->graph_scrlbar->setValue(Tick_counter-window_size/2);
}

//calculated one step of the replicator dynamic
void Replicator::update_proportions()
{
    QVector<double> fitnes(Proportions.count(),0);
    for (int i=0;i<Proportions.count();i++)//for all strategies against all strats calculate firness
    {
        for(int j=0;j<Proportions.count();j++){fitnes[i]+=Scoretable[i][j]*Proportions[j];}
        ui->graph->graph(i)->addData(Tick_counter,Proportions[i]);
    }
    if(Tick_counter<25)qDebug()<<"fitnes"<<fitnes;
    double sum_fitness=0;//calculate total fitness
    for (int i=0;i<Proportions.count();i++)sum_fitness+=Proportions[i]*fitnes[i];
    if(Tick_counter<25)qDebug()<<"sum_fitness"<<sum_fitness;

    QVector<double> new_props;//calculate the new proportions
    for (int i = 0; i < Proportions.count(); i++)//for all strategies
    {
        double new_prop=(Proportions[i] * (1 + Birthrate * fitnes[i])) / (1 + Birthrate * sum_fitness);
        new_props.append(new_prop);
    }
    if(Tick_counter<25)qDebug()<<"new_props"<<new_props;
    Proportions=new_props;//update the proportions
}

//slider for the birthrate
void Replicator::on_slid_birthr_valueChanged(int value)
{
    Birthrate=static_cast<double>(value)/100;
    ui->lbl_birthr->setText(QString::number(Birthrate));
}


//scrole trough graph with the scrolebar
void Replicator::on_graph_scrlbar_valueChanged(int value)
{
    if (qAbs(ui->graph->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
      {
        //qDebug()<<"scrole"<<value;
        ui->graph->xAxis->setRange(value, ui->graph->xAxis->range().size(), Qt::AlignCenter);
        ui->graph->replot();
      }
}

//continue replicator dyn
void Replicator::on_pb_cont_clicked()
{
    if(Repl_running==false){Repl_running=true;ui->pb_cont->setText("pause");repl_step();}
    else{Repl_running=false;ui->pb_cont->setText("continue");}

}

//use actuators rewards rewards
void Replicator::on_rb_actuator_toggled(bool checked)
{
    if(checked==true)
    {
        ui->rb_av_rew->setChecked(false);
        update_tab_matrix();
    }
}

//average rewards
void Replicator::on_rb_av_rew_toggled(bool checked)
{
    if(checked==true)
    {

        ui->rb_actuator->setChecked(false);
        update_tab_matrix();
    }

}
